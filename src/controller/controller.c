#include <stdint.h>
#include <string.h>

#include "hardware/timer.h"

#include "config.h"
#include "controller/controller.h"
#include "hid/data.h"
#include "hid/lights/report.h"
#include "rgb/rgb.h"

#define FRAME_TIME_US   8333 // ~120 fps
#define FADE_FRAMES     120
#define PAUSE_FRAMES    30

static rgb_oklab_t start_color;
static rgb_oklab_t target_color;
static float Ldiff;

void ctrl_init(controller_t *ctrl)
{
    ctrl->is_autonomous = true;
    ctrl->next_lamp_id = 0;

    ctrl->do_update = false;
    memset(ctrl->lamp_state, 0, sizeof(ctrl->lamp_state));

    rgb_tuple_t rgb_color = {0xa6, 0x24, 0xa6, 1};

    start_color = rgb_to_oklab(rgb_color);
    target_color = start_color;

    start_color.L = 0;
    target_color.L = 0.5;

    Ldiff = (target_color.L - start_color.L) / ((float) FADE_FRAMES);
}

static void animation_fade_frame(controller_t *ctrl)
{
    static uint8_t frame_count = 0;
    static uint8_t state = 0;
    static rgb_oklab_t color = {0, 0, 0};

    rgb_tuple_t rgb;
    switch (state) {
        case 0: // fade-in
            if (frame_count == 0) {
                color = start_color;
            }

            color.L += Ldiff;
            rgb = rgb_from_oklab(color);
            rgb_set_lamp_color(0, &rgb);

            frame_count++;
            if (frame_count == FADE_FRAMES) {
                frame_count = 0;
                state = 1;
            }
            break;

        case 1: // fade-out
            color.L -= Ldiff;
            rgb = rgb_from_oklab(color);
            rgb_set_lamp_color(0, &rgb);

            frame_count++;
            if (frame_count == FADE_FRAMES) {
                frame_count = 0;
                state = 2;
            }
            break;

        case 2: // pause
            frame_count++;
            if (frame_count == PAUSE_FRAMES) {
                frame_count = 0;
                state = 0;
            }
            break;
    }
}

void ctrl_task(controller_t *ctrl)
{
    static uint32_t last = 0;

    if (ctrl->do_update) {
        for (rgb_lamp_id_t id = 0; id < CFG_RGB_LAMP_COUNT; id++) {
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

    if (ctrl->is_autonomous) {
        uint32_t elapsed;
        uint32_t now = time_us_32();
        if (now < last) {
            // timer overflow
            elapsed = now + (UINT32_MAX - last) + 1;
        } else {
            elapsed = now - last;
        }

        if (elapsed >= FRAME_TIME_US) {
            last = now;
            animation_fade_frame(ctrl);
        }
    }
}

void ctrl_set_next_lamp_attributes_id(controller_t *ctrl, rgb_lamp_id_t lamp_id)
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
    rgb_lamp_id_t lamp_id = ctrl->next_lamp_id;
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

void ctrl_update_lamp(controller_t *ctrl, rgb_lamp_id_t lamp_id, rgb_tuple_t *tuple)
{
    if (ctrl->is_autonomous) {
        return;
    }
    lamp_state *state = &ctrl->lamp_state[lamp_id];
    state->next = *tuple;
    state->dirty = true;
}

void ctrl_apply_lamp_updates(controller_t *ctrl)
{
    if (ctrl->is_autonomous) {
        return;
    }
    ctrl->do_update = true;
}
