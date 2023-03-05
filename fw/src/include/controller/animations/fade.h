#ifndef CONTROLLER_ANIMATIONS_FADE_H_
#define CONTROLLER_ANIMATIONS_FADE_H_

#include <stdint.h>

#include "color/color.h"
#include "controller/controller.h"
#include "hid/vendor/report.h"

#define MAX_FADE_TARGETS 8

struct AnimationFade {
    struct Lab current_color;

    uint8_t target_count;
    struct Lab targets[MAX_FADE_TARGETS];

    uint32_t fade_frames[MAX_FADE_TARGETS];
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

void anim_fade_set_targets(struct AnimationFade *fade, struct Lab *targets, uint8_t count);
void anim_fade_set_fade_time_us(struct AnimationFade *fade, uint8_t stage, uint32_t fade_time);
void anim_fade_set_hold_time_us(struct AnimationFade *fade, uint8_t stage, uint32_t hold_time);

uint8_t anim_fade(controller_t *ctrl, uint8_t lamp_id, struct AnimationState *state);

// --------------------------------------
// Constructors and data specific effects
// --------------------------------------

struct __attribute__ ((packed)) AnimationBreatheReportData {
    /**
     * The colors to cycle between. The off_color is usually (0, 0, 0).
     */
    struct RGBu8 on_color;
    struct RGBu8 off_color;

    /**
     * The timing of the animation in milliseconds. This means that each phase
     * can last up to a minute for a maximum 4 minute cycle.
     *
     *         | A | B | C | D |
     *         |   |___|   |   |
     *   light |  /|   |\  |   |
     *         | / |   | \ |   |
     *         |/  |   |  \|___|
     *               time
     *
     *   A: on_fade_time_ms
     *   B: on_time_ms
     *   C: off_fade_time_ms
     *   D: off_time_ms
     *
     * Setting `on_fade_time_ms` is required. If `off_fade_time_ms` is 0, use
     * the value of `on_fade_time_ms`.
     */
    uint16_t on_fade_time_ms;
    uint16_t on_time_ms;
    uint16_t off_fade_time_ms;
    uint16_t off_time_ms;
};

/**
 * @brief Allocates new state for a breathing animation.
 *
 * Callers must free the state when it is no longer used.
 */
struct AnimationFade *anim_fade_new_breathe(struct AnimationBreatheReportData *data);

struct __attribute__ ((packed)) AnimationFadeReportData {
    /**
     * The colors to fade between. The first `color_count` elements of `colors`
     * should be set.
     */
    uint8_t color_count;
    struct RGBu8 colors[MAX_FADE_TARGETS];

    /**
     * The timing of the animation in milliseconds. This means that each phase
     * can last up to a minute for a maximum 2 * `color_count` minute cycle.
     *
     * `fade_time_ms` is the time spent transitioning between colors.
     * `hold_time_ms` is the time spent on each color.
     */
    uint16_t fade_time_ms;
    uint16_t hold_time_ms;
};

/**
 * @brief Allocates new state for a color-fade animation.
 *
 * Callers must free the state when it is no longer used.
 */
struct AnimationFade *anim_fade_new_fade(struct AnimationFadeReportData *data);

#endif /* CONTROLLER_ANIMATIONS_FADE_H_ */
