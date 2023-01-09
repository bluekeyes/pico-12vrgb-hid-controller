#ifndef CONTROLLER_ANIMATIONS_FADE_H_
#define CONTROLLER_ANIMATIONS_FADE_H_

#include <stdint.h>

#include "controller/controller.h"
#include "rgb/rgb.h"

#define FADE_TARGETS 2

struct AnimationFade {
    uint8_t lamp_id;

    rgb_oklab_t current_color;
    rgb_oklab_t targets[FADE_TARGETS];

    uint32_t fade_frames;
    uint32_t hold_frames[FADE_TARGETS];

    float Ldiff;
    float adiff;
    float bdiff;
};

struct AnimationFade anim_fade_get_defaults();
void anim_fade_set_targets(struct AnimationFade *fade, rgb_oklab_t *targets);
void anim_fade_set_fade_time(struct AnimationFade *fade, uint32_t fade_time_us);
void anim_fade_set_hold_time(struct AnimationFade *fade, uint8_t stage, uint32_t hold_time_us);
uint8_t anim_fade(struct Controller *ctrl, struct AnimationState *state);

/**
 * Shortcuts for specific types of fade effect
 */

struct AnimationFade anim_fade_breathe(rgb_tuple_t color, uint32_t fade_time_us);

#endif /* CONTROLLER_ANIMATIONS_FADE_H_ */
