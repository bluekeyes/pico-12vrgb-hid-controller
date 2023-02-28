#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "color/color.h"
#include "controller/animations/fade.h"
#include "controller/controller.h"
#include "device/lamp.h"
#include "hid/vendor/report.h"

static inline uint32_t ms_to_us(uint16_t ms)
{
    return 1000 * (uint32_t) ms;
}

struct AnimationFade *anim_fade_new_empty()
{
    struct AnimationFade *fade = calloc(1, sizeof(struct AnimationFade));
    if (fade == NULL) {
        return NULL;
    }

    fade->target_count = 2;
    anim_fade_set_fade_time_us(fade, 0, ms_to_us(1000));
    anim_fade_set_fade_time_us(fade, 1, ms_to_us(1000));

    return fade;
}

struct AnimationFade *anim_fade_new_breathe(struct AnimationBreatheReportData *data)
{
    struct AnimationFade *fade = anim_fade_new_empty();
    if (fade == NULL) {
        return NULL;
    }

    struct Labf targets[2];
    targets[0] = rgb_to_oklab(rgbi_to_f(data->on_color));
    if (data->off_color.r == 0 && data->off_color.g == 0 && data->off_color.b == 0) {
        // copy the on color if the off color is not set
        targets[1] = targets[0];
    } else {
        targets[1] = rgb_to_oklab(rgbi_to_f(data->off_color));
    }
    // lightness of off color is always 0
    targets[1].L = 0.f;
    anim_fade_set_targets(fade, targets, 2);

    anim_fade_set_fade_time_us(fade, 0, ms_to_us(data->on_fade_time_ms));
    anim_fade_set_hold_time_us(fade, 0, ms_to_us(data->on_time_ms));
    anim_fade_set_fade_time_us(fade, 1, ms_to_us(data->off_fade_time_ms > 0 ? data->off_fade_time_ms : data->on_fade_time_ms));
    anim_fade_set_hold_time_us(fade, 1, ms_to_us(data->off_time_ms));

    return fade;
}

struct AnimationFade *anim_fade_new_fade(struct AnimationFadeReportData *data)
{
    struct AnimationFade *fade = anim_fade_new_empty();
    if (fade == NULL) {
        return NULL;
    }

    uint8_t color_count = data->color_count;
    if (color_count > MAX_FADE_TARGETS) {
        color_count = MAX_FADE_TARGETS;
    }

    struct Labf targets[MAX_FADE_TARGETS];
    for (uint8_t i = 0; i < color_count; i++) {
        targets[i] = rgb_to_oklab(rgbu8tof(data->colors[i]));

        anim_fade_set_fade_time_us(fade, i, ms_to_us(data->fade_time_ms));
        anim_fade_set_hold_time_us(fade, i, ms_to_us(data->hold_time_ms));
    }
    anim_fade_set_targets(fade, targets, data->color_count);

    return fade;
}

void anim_fade_set_targets(struct AnimationFade *fade, struct Labf *targets, uint8_t count)
{
    count = count > MAX_FADE_TARGETS ? MAX_FADE_TARGETS : count;

    memcpy(fade->targets, targets, count * sizeof(struct Labf));
    fade->target_count = count;
    fade->current_color = targets[count - 1];
}

void anim_fade_set_fade_time_us(struct AnimationFade *fade, uint8_t stage, uint32_t fade_time)
{
    if (stage >= MAX_FADE_TARGETS) {
        return;
    }
    fade->fade_frames[stage] = fade_time / ANIM_FRAME_TIME_US;
}

void anim_fade_set_hold_time_us(struct AnimationFade *fade, uint8_t stage, uint32_t hold_time)
{
    if (stage >= MAX_FADE_TARGETS) {
        return;
    }
    fade->hold_frames[stage] = hold_time / ANIM_FRAME_TIME_US;
}

static void anim_fade_set_diffs(struct AnimationFade *fade, uint8_t dest, uint8_t src)
{
    uint32_t fade_frames = fade->fade_frames[dest];
    if (fade_frames > 0) {
        fade->Ldiff = (fade->targets[dest].L - fade->targets[src].L) / ((float) fade_frames);
        fade->adiff = (fade->targets[dest].a - fade->targets[src].a) / ((float) fade_frames);
        fade->bdiff = (fade->targets[dest].b - fade->targets[src].b) / ((float) fade_frames);
    } else {
        fade->Ldiff = 0.f;
        fade->adiff = 0.f;
        fade->bdiff = 0.f;
    }
}

void log_fade_stage(uint8_t stage, struct Labf current_color, struct Labf target_color)
{
    struct RGBu16 rgb;
    printf("animate/fade: start stage %d\n", stage);

    rgb = rgbftou16(oklab_to_rgb(current_color));
    printf(
        "    current color = oklab(% .8f, % .8f, % .8f) | rgb(%5d, %5d, %5d)\n",
        current_color.L, current_color.a, current_color.b,
        rgb.r, rgb.g, rgb.b
    );

    rgb = rgbftou16(oklab_to_rgb(target_color));
    printf(
        "     target color = oklab(% .8f, % .8f, % .8f) | rgb(%5d, %5d, %5d)\n",
        target_color.L, target_color.a, target_color.b,
        rgb.r, rgb.g, rgb.b
    );
}

uint8_t anim_fade(controller_t *ctrl, uint8_t lamp_id, struct AnimationState *state)
{
    struct AnimationFade *fade = (struct AnimationFade *) state->data;

    bool is_dirty = false;
    bool is_hold = (state->stage % 2) == 1;

    uint8_t target = state->stage / 2;
    uint32_t stage_frames = 0;

    if (is_hold) {
        if (state->stage_frame == 0) {
#ifdef DEBUG_ANIMATE
            log_fade_stage(state->stage, fade->current_color, fade->targets[target]);
#endif
            // first frame of a hold, make sure we show the exact color
            fade->current_color = fade->targets[target];
            is_dirty = true;
        }
        stage_frames = fade->hold_frames[target];
    } else {
        if (state->stage_frame == 0) {
#ifdef DEBUG_ANIMATE
            log_fade_stage(state->stage, fade->current_color, fade->targets[target]);
#endif
            // starting a new fade stage, initialize fade diffs
            anim_fade_set_diffs(fade, target, target == 0 ? fade->target_count - 1 : (target - 1));
        }
        stage_frames = fade->fade_frames[target];

        fade->current_color.L += fade->Ldiff;
        fade->current_color.a += fade->adiff;
        fade->current_color.b += fade->bdiff;
        is_dirty = true;
    }

    if (is_dirty) {
        struct RGBu16 rgb = rgbftou16(oklab_to_rgb(fade->current_color));
        ctrl_update_lamp(ctrl, lamp_id, lamp_value_from_rgbu16(rgb), true);
    }

    if (stage_frames == 0 || state->stage_frame == stage_frames - 1) {
        return (state->stage + 1) % (2 * fade->target_count);
    }
    return state->stage;
}

// ----------
// Assertions
// ----------

static_assert(
    sizeof(struct AnimationBreatheReportData) <= ANIMATION_REPORT_DATA_SIZE,
    "struct AnimationBreatheReportData is larger than the report data size"
);

static_assert(
    sizeof(struct AnimationFadeReportData) <= ANIMATION_REPORT_DATA_SIZE,
    "struct AnimationFadeReportData is larger than the report data size"
);
