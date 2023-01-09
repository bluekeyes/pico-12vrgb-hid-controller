#include <stdint.h>
#include <string.h>

#include "color/color.h"
#include "controller/animations/fade.h"
#include "controller/controller.h"

struct AnimationFade anim_fade_get_defaults()
{
    struct Labf targets[] = {
        {0.f, 0.f, 0.f},
        {1.f, 0.f, 0.f},
    };

    struct AnimationFade fade;
    fade.lamp_id = 0; // TODO(bkeyes): figure out how multi-channel animations work

    memset(fade.targets, 0, sizeof(fade.targets));
    anim_fade_set_targets(&fade, targets, 2);

    anim_fade_set_fade_time(&fade, 1000000);

    memset(fade.hold_frames, 0, sizeof(fade.hold_frames));
    anim_fade_set_hold_time(&fade, 0, 250000);
    anim_fade_set_hold_time(&fade, 1, 1000000);

    return fade;
}

struct AnimationFade anim_fade_breathe(struct RGBi color, uint32_t fade_time_us)
{
    struct Labf targets[2];
    targets[0] = rgb_to_oklab(rgbi_to_f(color));
    targets[1] = targets[0];
    targets[1].L = 0.f;

    struct AnimationFade fade = anim_fade_get_defaults();
    anim_fade_set_targets(&fade, targets, 2);
    anim_fade_set_fade_time(&fade, fade_time_us);
    anim_fade_set_hold_time(&fade, 0, fade_time_us/8);
    anim_fade_set_hold_time(&fade, 1, fade_time_us/2);
    return fade;
}

struct AnimationFade anim_fade_cross(struct RGBi color1, struct RGBi color2, uint32_t fade_time_us)
{
    struct Labf targets[] = {
        rgb_to_oklab(rgbi_to_f(color1)),
        rgb_to_oklab(rgbi_to_f(color2)),
    };

    struct AnimationFade fade = anim_fade_get_defaults();
    anim_fade_set_targets(&fade, targets, 2);
    anim_fade_set_fade_time(&fade, fade_time_us);
    anim_fade_set_hold_time(&fade, 0, fade_time_us/8);
    anim_fade_set_hold_time(&fade, 1, fade_time_us/8);
    return fade;
}

void anim_fade_set_targets(struct AnimationFade *fade, struct Labf *targets, uint8_t count)
{
    count = count > MAX_FADE_TARGETS ? MAX_FADE_TARGETS : count;

    memcpy(fade->targets, targets, count * sizeof(struct Labf));
    fade->target_count = count;
    fade->current_color = targets[count - 1];
}

void anim_fade_set_fade_time(struct AnimationFade *fade, uint32_t fade_time_us)
{
    fade->fade_frames = fade_time_us / ANIM_FRAME_TIME_US;
}

void anim_fade_set_hold_time(struct AnimationFade *fade, uint8_t stage, uint32_t hold_time_us)
{
    if (stage > MAX_FADE_TARGETS - 1) {
        return;
    }
    fade->hold_frames[stage] = hold_time_us / ANIM_FRAME_TIME_US;
}

static void anim_fade_set_diffs(struct AnimationFade *fade, uint8_t dest, uint8_t src)
{
    if (fade->fade_frames > 0) {
        fade->Ldiff = (fade->targets[dest].L - fade->targets[src].L) / ((float) fade->fade_frames);
        fade->adiff = (fade->targets[dest].a - fade->targets[src].a) / ((float) fade->fade_frames);
        fade->bdiff = (fade->targets[dest].b - fade->targets[src].b) / ((float) fade->fade_frames);
    }
}

uint8_t anim_fade(controller_t *ctrl, struct AnimationState *state)
{
    struct AnimationFade *fade = (struct AnimationFade *) state->data;

    bool is_dirty = false;
    bool is_hold = (state->stage % 2) == 1;

    uint8_t target = state->stage / 2;
    uint32_t stage_frames = 0;

    if (is_hold) {
        if (state->stage_frame == 0) {
            // first frame of a hold, make sure we show the exact color
            fade->current_color = fade->targets[target];
            is_dirty = true;
        }
        stage_frames = fade->hold_frames[target];
    } else {
        if (state->stage_frame == 0) {
            // starting a new fade stage, initialize fade diffs
            anim_fade_set_diffs(fade, target, target == 0 ? fade->target_count - 1 : (target - 1));
        }
        stage_frames = fade->fade_frames;

        fade->current_color.L += fade->Ldiff;
        fade->current_color.a += fade->adiff;
        fade->current_color.b += fade->bdiff;
        is_dirty = true;
    }

    if (is_dirty) {
        struct RGBi rgb = rgbf_to_i(oklab_to_rgb(fade->current_color));
        rgb_tuple_t t = {
            rgb.r,
            rgb.g,
            rgb.b,
            0x01,
        };
        ctrl_update_lamp(ctrl, fade->lamp_id, &t, true);
    }

    if (stage_frames == 0 || state->stage_frame == stage_frames - 1) {
        return (state->stage + 1) % (2 * fade->target_count);
    }
    return state->stage;
}
