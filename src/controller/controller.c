#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hardware/timer.h"

#include "controller/controller.h"
#include "device/lamp.h"
#include "device/specs.h"
#include "hid/data.h"
#include "hid/lights/report.h"

static inline struct AnimationState get_initial_animation_state(void *data) {
    struct AnimationState state = {
        .stage = 0,
        .frame = 0,
        .stage_frame = 0,
        .data = data,
    };
    return state;
}

void ctrl_init(controller_t *ctrl)
{
    ctrl->is_autonomous = true;
    ctrl->next_lamp_id = 0;

    ctrl->do_update = false;
    memset(ctrl->lamp_state, 0, sizeof(ctrl->lamp_state));

    for (uint8_t i = 0; i < LAMP_COUNT; i++) {
        ctrl->animation[i] = get_initial_animation_state(NULL);
        ctrl->frame_cb[i] = NULL;
    }
    ctrl->last_frame_time_us = 0;
}

/**
 * @brief Processes a single frame of animation for a lamp.
 */
static void ctrl_animation_frame(controller_t *ctrl, uint8_t lamp_id)
{
    struct AnimationState *state = &ctrl->animation[lamp_id];

    FrameCallback frame_cb = ctrl->frame_cb[lamp_id];
    if (frame_cb == NULL) {
        return;
    }

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

void ctrl_task(controller_t *ctrl)
{
    if (ctrl->is_autonomous) {
        uint32_t last = ctrl->last_frame_time_us;
        uint32_t now = time_us_32();

        uint32_t elapsed;
        if (now < last) { /* timer overflow */
            elapsed = now + (UINT32_MAX - last) + 1;
        } else {
            elapsed = now - last;
        }

        if (elapsed >= ANIM_FRAME_TIME_US) {
            for (uint8_t id = 0; id < LAMP_COUNT; id++) {
                ctrl_animation_frame(ctrl, id);
            }
            ctrl->last_frame_time_us = now;
        }
    }

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
    report->lamp_purpose = lamp_purposes[lamp_id];

    HID_SET_FLAG(report->is_programmable);

    report->position_x = lamp_positions[lamp_id][0];
    report->position_y = lamp_positions[lamp_id][1];
    report->position_z = lamp_positions[lamp_id][2];

    report->red_level_count = LAMP_COLOR_LEVELS;
    report->green_level_count = LAMP_COLOR_LEVELS;
    report->blue_level_count = LAMP_COLOR_LEVELS;
    report->intensity_level_count = LAMP_INTENSITY_LEVELS;

    report->update_latency = CFG_RGB_LAMP_UPDATE_LATENCY;
    report->input_binding = 0x0000;
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
