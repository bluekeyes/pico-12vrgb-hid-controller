#include <stdint.h>
#include <string.h>

#include "controller/controller.h"
#include "controller/animations/fade.h"
#include "rgb/rgb.h"

struct AnimationFade anim_fade_get_defaults()
{
    rgb_oklab_t targets[] = {
        {0.f, 0.f, 0.f},
        {1.f, 0.f, 0.f},
    };

    struct AnimationFade fade;
    fade.lamp_id = 0; // TODO(bkeyes): figure out how multi-channel animations work
    anim_fade_set_targets(&fade, targets);
    anim_fade_set_fade_time(&fade, 1000000);
    anim_fade_set_hold_time(&fade, 0, 250000);
    anim_fade_set_hold_time(&fade, 1, 1000000);
    return fade;
}

struct AnimationFade anim_fade_breathe(rgb_tuple_t color, uint32_t fade_time_us)
{
    rgb_oklab_t targets[FADE_TARGETS];
    targets[0] = rgb_to_oklab(color);
    targets[1] = targets[0];
    targets[1].L = 0.f;

    struct AnimationFade fade = anim_fade_get_defaults();
    anim_fade_set_targets(&fade, targets);
    anim_fade_set_fade_time(&fade, fade_time_us);
    anim_fade_set_hold_time(&fade, 0, fade_time_us/8);
    anim_fade_set_hold_time(&fade, 1, fade_time_us/2);
    return fade;
}

struct AnimationFade anim_fade_cross(rgb_tuple_t color1, rgb_tuple_t color2, uint32_t fade_time_us)
{
    rgb_oklab_t targets[] = {
        rgb_to_oklab(color1),
        rgb_to_oklab(color2),
    };

    struct AnimationFade fade = anim_fade_get_defaults();
    anim_fade_set_targets(&fade, targets);
    anim_fade_set_fade_time(&fade, fade_time_us);
    anim_fade_set_hold_time(&fade, 0, fade_time_us/8);
    anim_fade_set_hold_time(&fade, 1, fade_time_us/8);
    return fade;
}

void anim_fade_set_targets(struct AnimationFade *fade, rgb_oklab_t *targets)
{
    memcpy(fade->targets, targets, sizeof(fade->targets));
    fade->current_color = targets[FADE_TARGETS - 1];
}

void anim_fade_set_fade_time(struct AnimationFade *fade, uint32_t fade_time_us)
{
    fade->fade_frames = fade_time_us / ANIM_FRAME_TIME_US;
}

void anim_fade_set_hold_time(struct AnimationFade *fade, uint8_t stage, uint32_t hold_time_us)
{
    if (stage > FADE_TARGETS - 1) {
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

uint8_t anim_fade(struct Controller *ctrl, struct AnimationState *state)
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
            anim_fade_set_diffs(fade, target, target == 0 ? FADE_TARGETS - 1 : (target - 1));
        }
        stage_frames = fade->fade_frames;

        fade->current_color.L += fade->Ldiff;
        fade->current_color.a += fade->adiff;
        fade->current_color.b += fade->bdiff;
        is_dirty = true;
    }

    if (is_dirty) {
        rgb_tuple_t rgb = rgb_from_oklab(fade->current_color);
        ctrl_update_lamp(ctrl, fade->lamp_id, &rgb, true);
    }

    if (stage_frames == 0 || state->stage_frame == stage_frames - 1) {
        return (state->stage + 1) % (2*FADE_TARGETS);
    }
    return state->stage;
}
