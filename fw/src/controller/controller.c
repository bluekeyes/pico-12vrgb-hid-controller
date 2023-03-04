#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hardware/sync.h"
#include "pico/time.h"

#include "controller/animations/fade.h"
#include "controller/controller.h"
#include "device/lamp.h"
#include "device/specs.h"
#include "hid/lights/report.h"

static struct AnimationState get_initial_animation_state(void *);
static void ctrl_animation_frame(controller_t *, uint8_t);

void ctrl_init(controller_t *ctrl)
{
    ctrl->is_suspended = false;
    ctrl->is_autonomous = true;
    ctrl->next_lamp_id = 0;

    ctrl->do_update = false;
    memset(ctrl->lamp_state, 0, sizeof(ctrl->lamp_state));

    for (uint8_t i = 0; i < LAMP_COUNT; i++) {
        ctrl->animation[i] = get_initial_animation_state(NULL);
        ctrl->frame_cb[i] = NULL;
    }
    ctrl->last_frame = nil_time;
}

void ctrl_task(controller_t *ctrl)
{
    if (ctrl->is_suspended) {
        return;
    }

    // Update animation state
    if (ctrl->is_autonomous) {
        absolute_time_t now = get_absolute_time();
        int64_t elapsed_us = absolute_time_diff_us(ctrl->last_frame, now);
        if (elapsed_us >= ANIM_FRAME_TIME_US) {
            for (uint8_t id = 0; id < LAMP_COUNT; id++) {
                ctrl_animation_frame(ctrl, id);
            }
            ctrl->last_frame = now;
        }
    }

    // Apply pending changes to LEDs
    if (ctrl->do_update) {
        for (uint8_t id = 0; id < LAMP_COUNT; id++) {
            lamp_state *state = &ctrl->lamp_state[id];
            if (state->dirty) {
                lamp_set_value(id, state->next);

                state->current = state->next;
                memset(&state->next, 0, sizeof(struct LampValue));
                state->dirty = false;
            }
        }
        ctrl->do_update = false;
    }
}

void ctrl_suspend(controller_t *ctrl)
{
    // turn off all lamps immediately, but mark them as dirty for the next task
    for (uint8_t id = 0; id < LAMP_COUNT; id++) {
        lamp_state *state = &ctrl->lamp_state[id];

        state->dirty = true;
        state->next = state->current;

        lamp_set_value(id, lamp_value_off());
    }

    ctrl->is_suspended = true;
}

void ctrl_resume(controller_t *ctrl)
{
    // clear suspended flag, next task will do the rest
    ctrl->is_suspended = false;
}

static inline struct AnimationState get_initial_animation_state(void *data)
{
    struct AnimationState state = {
        .stage = 0,
        .frame = 0,
        .stage_frame = 0,
        .data = data,
    };
    return state;
}

/**
 * @brief Processes a single frame of animation for a lamp.
 */
static void ctrl_animation_frame(controller_t *ctrl, uint8_t lamp_id)
{
    FrameCallback frame_cb = ctrl->frame_cb[lamp_id];
    if (frame_cb == NULL) {
        return;
    }

    struct AnimationState *state = &ctrl->animation[lamp_id];
    uint8_t next_stage = frame_cb(ctrl, lamp_id, state);

    state->frame++;
    state->stage_frame++;

    if (state->stage != next_stage) {
        state->stage = next_stage;
        state->stage_frame = 0;

        // returning to stage 0 resets the full animation
        if (next_stage == 0) {
            state->frame = 0;
        }
    }
}

void ctrl_set_next_lamp_attributes_id(controller_t *ctrl, uint8_t lamp_id)
{
    if (lamp_id > MAX_LAMP_ID) {
        ctrl->next_lamp_id = 0;
    } else {
        ctrl->next_lamp_id = lamp_id;
    }
}

void ctrl_get_lamp_attributes(controller_t *ctrl, struct LampAttributesResponseReport *report)
{
    // Advance the lamp ID to allow reading attributes for all lamps in
    // sequential reports without setting a new ID each time
    uint8_t lamp_id = ctrl->next_lamp_id;
    ctrl->next_lamp_id = (lamp_id + 1) % LAMP_COUNT;

    report->lamp_id = lamp_id;
    report->position_x = lamp_positions[lamp_id][0];
    report->position_y = lamp_positions[lamp_id][1];
    report->position_z = lamp_positions[lamp_id][2];
    report->update_latency = CFG_RGB_LAMP_UPDATE_LATENCY;
    report->lamp_purpose = lamp_purposes[lamp_id];
    report->red_level_count = LAMP_COLOR_LEVELS;
    report->green_level_count = LAMP_COLOR_LEVELS;
    report->blue_level_count = LAMP_COLOR_LEVELS;
    report->intensity_level_count = LAMP_INTENSITY_LEVELS;
    report->is_programmable = 0x01;
    report->input_binding = 0x00;
}

void ctrl_update_lamp(controller_t *ctrl, uint8_t lamp_id, struct LampValue value, bool apply)
{
    lamp_state *state = &ctrl->lamp_state[lamp_id];
    state->next = value;
    state->dirty = true;

    if (apply) {
        ctrl->do_update = true;
    }
}

void ctrl_apply_lamp_updates(controller_t *ctrl)
{
    ctrl->do_update = true;
}

void ctrl_set_autonomous_mode(controller_t *ctrl, bool autonomous)
{
    ctrl->is_autonomous = autonomous;
}

bool ctrl_get_autonomous_mode(controller_t *ctrl)
{
    return ctrl->is_autonomous;
}

void ctrl_set_animation(controller_t *ctrl, uint8_t lamp_id, FrameCallback frame_cb, void *data)
{
    void *old_data = ctrl->animation[lamp_id].data;
    if (old_data != NULL) {
        free(old_data);
    }

    ctrl->frame_cb[lamp_id] = frame_cb;
    ctrl->animation[lamp_id] = get_initial_animation_state(data);
}

// --------------------------
// Report-specific animations
// --------------------------

static void set_animation_none(controller_t *ctrl, struct Vendor12VRGBAnimationReport *report)
{
    // Setting the "none" animation will also turn off the lamp
    ctrl_update_lamp(ctrl, report->lamp_id, lamp_value_off(), true);
    ctrl_set_animation(ctrl, report->lamp_id, NULL, NULL);
}

static void set_animation_breathe(controller_t *ctrl, struct Vendor12VRGBAnimationReport *report)
{
    struct AnimationBreatheReportData *data = (struct AnimationBreatheReportData *) report->data;
    struct AnimationFade *fade = anim_fade_new_breathe(data);
    ctrl_set_animation(ctrl, report->lamp_id, anim_fade, fade);
}

static void set_animation_fade(controller_t *ctrl, struct Vendor12VRGBAnimationReport *report)
{
    struct AnimationFadeReportData *data = (struct AnimationFadeReportData *) report->data;
    struct AnimationFade *fade = anim_fade_new_fade(data);
    ctrl_set_animation(ctrl, report->lamp_id, anim_fade, fade);
}

void ctrl_set_animation_from_report(controller_t *ctrl, struct Vendor12VRGBAnimationReport *report)
{
    switch (report->type) {
    case ANIMATION_TYPE_NONE:
        set_animation_none(ctrl, report);
        break;

    case ANIMATION_TYPE_BREATHE:
        set_animation_breathe(ctrl, report);
        break;

    case ANIMATION_TYPE_FADE:
        set_animation_fade(ctrl, report);
        break;
    }
}
