#ifndef CONTROLLER_ANIMATIONS_FADE_H_
#define CONTROLLER_ANIMATIONS_FADE_H_

#include <stdint.h>

#include "controller/controller.h"
#include "color/color.h"

#define MAX_FADE_TARGETS 8

struct AnimationFade {
    uint8_t lamp_id;

    struct Labf current_color;

    uint8_t target_count;
    struct Labf targets[MAX_FADE_TARGETS];

    uint32_t fade_frames;
    uint32_t hold_frames[MAX_FADE_TARGETS];

    float Ldiff;
    float adiff;
    float bdiff;
};

struct AnimationFade anim_fade_get_defaults();
void anim_fade_set_targets(struct AnimationFade *fade, struct Labf *targets, uint8_t count);
void anim_fade_set_fade_time(struct AnimationFade *fade, uint32_t fade_time_us);
void anim_fade_set_hold_time(struct AnimationFade *fade, uint8_t stage, uint32_t hold_time_us);
uint8_t anim_fade(controller_t *ctrl, struct AnimationState *state);

/**
 * Shortcuts for specific types of fade effect
 */

struct AnimationFade anim_fade_breathe(struct RGBi color, uint32_t fade_time_us);
struct AnimationFade anim_fade_cross(struct RGBi color1, struct RGBi color2, uint32_t fade_time_us);

#endif /* CONTROLLER_ANIMATIONS_FADE_H_ */
