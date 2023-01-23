#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hardware/timer.h"
#include "hardware/sync.h"
#include "tusb.h"

#include "controller/animations/fade.h"
#include "controller/controller.h"
#include "device/lamp.h"
#include "device/specs.h"
#include "hid/data.h"
#include "hid/descriptor.h"
#include "hid/lights/report.h"

static struct AnimationState get_initial_animation_state(void *);
static void ctrl_animation_frame(controller_t *, uint8_t);
static uint32_t get_elapsed(uint32_t now, uint32_t last);

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
    ctrl->last_frame_time_us = 0;
    ctrl->last_keepalive_time_us = 0;
}

void ctrl_task(controller_t *ctrl)
{
    if (ctrl->is_suspended) {
        // If we're supposed to be suspended, wait for an interrupt. We should
        // get a USB interrupt on resume, which will clear the suspended flag.
        // If there's an interrupt for another reason, the next run of the task
        // will block again.
        __wfi();
        return;
    }

    uint32_t now = time_us_32();

    // Update animation state
    if (ctrl->is_autonomous) {
        uint32_t elapsed = get_elapsed(now, ctrl->last_frame_time_us);
        if (elapsed >= ANIM_FRAME_TIME_US) {
            for (uint8_t id = 0; id < LAMP_COUNT; id++) {
                ctrl_animation_frame(ctrl, id);
            }
            ctrl->last_frame_time_us = now;
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

    // Generate keepalive messages on the vendor interface
    //
    // Without regular traffic, the Windows 11 HID driver appears use USB
    // Selective Suspend to suspend idle devices to save power. There's no way
    // to distinguish between a selective suspend and a global suspend (i.e.
    // the system going to sleep), so we need to disable the selective events
    // in order to correctly detect global events and turn off the lamps.
    //
    // Theoretically, you can disable this feature in Windows, but I couldn't
    // get it to work in my initial tests and not having to configure stuff at
    // all sounds better.
    //
    // See also: https://learn.microsoft.com/en-us/windows-hardware/drivers/hid/selective-suspend-for-hid-over-usb-devices
    uint32_t elapsed = get_elapsed(now, ctrl->last_keepalive_time_us);
    if (elapsed >= CFG_RGB_KEEPALIVE_INTERVAL) {
        if (tud_hid_ready()) {
            uint8_t keepalive_data = 0x01;
            tud_hid_report(HID_REPORT_ID_VENDOR_12VRGB_KEEPALIVE, &keepalive_data, 1);
        }
        ctrl->last_keepalive_time_us = now;
    }
}

static inline uint32_t get_elapsed(uint32_t now, uint32_t last)
{
    if (now < last) { /* timer overflow */
        return now + (UINT32_MAX - last) + 1;
    }
    return now - last;
}

void ctrl_suspend(controller_t *ctrl)
{
    // turn off all lamps immediately, but mark them as dirty for the next task
    for (uint8_t id = 0; id < LAMP_COUNT; id++) {
        lamp_state *state = &ctrl->lamp_state[id];

        state->dirty = true;
        state->next = state->current;

        lamp_set_off(id);
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
    ctrl_set_animation(ctrl, report->lamp_id, NULL, NULL);
}

static void set_animation_breathe(controller_t *ctrl, struct Vendor12VRGBAnimationReport *report)
{
    struct ReportParametersBreathe *params = (struct ReportParametersBreathe *) report->parameters;
    struct AnimationFade *fade = anim_fade_new_breathe(params->color, params->fade_time);
    ctrl_set_animation(ctrl, report->lamp_id, anim_fade, fade);
}

static void set_animation_fade(controller_t *ctrl, struct Vendor12VRGBAnimationReport *report)
{
    struct ReportParametersFade *params = (struct ReportParametersFade *) report->parameters;

    uint32_t color_count = params->color_count;
    if (color_count > ANIMATION_REPORT_MAX_COLORS) {
        color_count = ANIMATION_REPORT_MAX_COLORS;
    }
    if (color_count > MAX_FADE_TARGETS) {
        color_count = MAX_FADE_TARGETS;
    }

    struct Labf colors[MAX_FADE_TARGETS];
    for (uint8_t i = 0; i < color_count; i++) {
        colors[i] = rgb_to_oklab(rgbi_to_f(params->colors[i]));
    }

    struct AnimationFade *fade = anim_fade_new_empty();
    anim_fade_set_targets(fade, colors, color_count);
    anim_fade_set_fade_time(fade, params->fade_time);
    for (uint8_t i = 0; i < color_count; i++) {
        anim_fade_set_hold_time(fade, i, params->hold_time);
    }

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
