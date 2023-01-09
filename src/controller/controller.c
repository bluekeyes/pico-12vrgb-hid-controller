#include <stdint.h>
#include <string.h>

#include "hardware/timer.h"

#include "config.h"
#include "controller/controller.h"
#include "hid/data.h"
#include "hid/lights/report.h"
#include "rgb/rgb.h"

static inline struct AnimationState get_initial_animation_state(void *data) {
    struct AnimationState state = {
        .frame = 0,
        .stage = 0,
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

    ctrl->animation = get_initial_animation_state(NULL);
    ctrl->frame_cb = NULL;
    ctrl->last_frame_time_us = 0;
}

void ctrl_task(controller_t *ctrl)
{
    if (ctrl->is_autonomous && ctrl->frame_cb != NULL) {
        uint32_t last = ctrl->last_frame_time_us;
        uint32_t now = time_us_32();

        uint32_t elapsed;
        if (now < last) { /* timer overflow */
            elapsed = now + (UINT32_MAX - last) + 1;
        } else {
            elapsed = now - last;
        }

        if (elapsed >= ANIM_FRAME_TIME_US) {
            struct AnimationState *state = &ctrl->animation;
            uint8_t next_stage = ctrl->frame_cb(ctrl, state);

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
            ctrl->last_frame_time_us = now;
        }
    }

    if (ctrl->do_update) {
        for (uint8_t id = 0; id < CFG_RGB_LAMP_COUNT; id++) {
            lamp_state *state = &ctrl->lamp_state[id];
            if (state->dirty) {
                rgb_set_lamp_color(id, &state->next);

                state->current = state->next;
                memset(&state->next, 0, sizeof(rgb_tuple_t));
                state->dirty = false;
            }
        }
        ctrl->do_update = false;
    }

}

void ctrl_set_next_lamp_attributes_id(controller_t *ctrl, uint8_t lamp_id)
{
    if (lamp_id > CFG_RGB_LAMP_COUNT - 1) {
        ctrl->next_lamp_id = 0;
    } else {
        ctrl->next_lamp_id = lamp_id;
    }
}

void ctrl_get_lamp_attributes(controller_t *ctrl, lamp_attributes_response_report_t *report)
{
    // Advance the lamp ID to allow reading attributes for all lamps in
    // sequential reports without setting a new ID each time
    uint8_t lamp_id = ctrl->next_lamp_id;
    ctrl->next_lamp_id = (lamp_id + 1) % CFG_RGB_LAMP_COUNT;

    report->lamp_id = lamp_id;
    report->lamp_purpose = rgb_lamp_purposes[lamp_id];

    HID_SET_FLAG(report->is_programmable);

    report->position_x = rgb_lamp_positions[lamp_id][0];
    report->position_y = rgb_lamp_positions[lamp_id][1];
    report->position_z = rgb_lamp_positions[lamp_id][2];

    report->red_level_count = RGB_COLOR_LEVEL_COUNT;
    report->green_level_count = RGB_COLOR_LEVEL_COUNT;
    report->blue_level_count = RGB_COLOR_LEVEL_COUNT;
    report->intensity_level_count = RGB_INTENSITY_LEVEL_COUNT;

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

void ctrl_set_animation(controller_t *ctrl, FrameCallback frame_cb, void *data)
{
    ctrl->frame_cb = frame_cb;
    ctrl->animation = get_initial_animation_state(data);
}

void ctrl_update_lamp(controller_t *ctrl, uint8_t lamp_id, rgb_tuple_t *tuple, bool apply)
{
    lamp_state *state = &ctrl->lamp_state[lamp_id];
    state->next = *tuple;
    state->dirty = true;

    if (apply) {
        ctrl->do_update = true;
    }
}

void ctrl_apply_lamp_updates(controller_t *ctrl)
{
    ctrl->do_update = true;
}
