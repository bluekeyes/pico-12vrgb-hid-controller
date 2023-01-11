#ifndef CONTROLLER_ANIMATIONS_FADE_H_
#define CONTROLLER_ANIMATIONS_FADE_H_

#include <stdint.h>

#include "controller/controller.h"
#include "color/color.h"

#define MAX_FADE_TARGETS 8

struct AnimationFade {
    struct Labf current_color;

    uint8_t target_count;
    struct Labf targets[MAX_FADE_TARGETS];

    uint32_t fade_frames;
    uint32_t hold_frames[MAX_FADE_TARGETS];

    float Ldiff;
    float adiff;
    float bdiff;
};

/**
 * @brief Allocates new, empty state for a fade animation.
 *
 * Callers must free the state when it is no longer used. The empty animation
 * has no visible output.
 */
struct AnimationFade *anim_fade_new_empty();

void anim_fade_set_targets(struct AnimationFade *fade, struct Labf *targets, uint8_t count);
void anim_fade_set_fade_time(struct AnimationFade *fade, uint32_t fade_time_us);
void anim_fade_set_hold_time(struct AnimationFade *fade, uint8_t stage, uint32_t hold_time_us);

uint8_t anim_fade(controller_t *ctrl, uint8_t lamp_id, struct AnimationState *state);

/**
 * Shortcuts for specific types of fade effect
 */

/**
 * @brief Allocates new state for a breathing animation.
 *
 * Callers must free the state when it is no longer used.
 */
struct AnimationFade *anim_fade_new_breathe(struct RGBi color, uint32_t fade_time_us);

/**
 * @brief Allocates new state for a cross-fade animation.
 *
 * Callers must free the state when it is no longer used.
 */
struct AnimationFade *anim_fade_new_cross(struct RGBi color1, struct RGBi color2, uint32_t fade_time_us);

#endif /* CONTROLLER_ANIMATIONS_FADE_H_ */
