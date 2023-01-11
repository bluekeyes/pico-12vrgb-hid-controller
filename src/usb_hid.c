#include <stdlib.h>

#include "pico/bootrom.h"
#include "tusb.h"

#include "controller/animations/fade.h"
#include "controller/controller.h"
#include "device/lamp.h"
#include "device/specs.h"
#include "hid/data.h"
#include "hid/descriptor.h"
#include "hid/lights/report.h"
#include "hid/lights/usage.h"
#include "hid/vendor/report.h"
#include "hid/vendor/usage.h"

extern controller_t ctrl;

static uint16_t get_report_lamp_array_attributes(uint8_t *buffer, uint16_t reqlen)
{
    if (reqlen < sizeof(struct LampArrayAttributesReport)) {
        return 0;
    }

    struct LampArrayAttributesReport *report = (struct LampArrayAttributesReport *) buffer;
    report->lamp_count = LAMP_COUNT;
    report->bounding_box_width = CFG_RGB_BOUNDING_BOX_WIDTH;
    report->bounding_box_height = CFG_RGB_BOUNDING_BOX_HEIGHT;
    report->bounding_box_depth = CFG_RGB_BOUNDING_BOX_DEPTH;
    report->min_update_interval = CFG_RGB_MINIMUM_UPDATE_INTERVAL;
    report->lamp_kind = LAMP_ARRAY_KIND_CHASSIS;

    return sizeof(struct LampArrayAttributesReport);
}

static uint16_t get_report_lamp_attributes_response(uint8_t *buffer, uint16_t reqlen)
{
    if (reqlen < sizeof(struct LampAttributesResponseReport)) {
        return 0;
    }

    struct LampAttributesResponseReport *report = (struct LampAttributesResponseReport *) buffer;
    ctrl_get_lamp_attributes(&ctrl, report);

    return sizeof(struct LampAttributesResponseReport);
}

static void set_report_lamp_attributes_request(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(struct LampAttributesRequestReport)) {
        return;
    }

    struct LampAttributesRequestReport *report = (struct LampAttributesRequestReport *) buffer;
    ctrl_set_next_lamp_attributes_id(&ctrl, report->lamp_id);
}

static void set_report_lamp_multi_update(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(struct LampMultiUpdateReport)) {
        return;
    }
    if (ctrl_get_autonomous_mode(&ctrl)) {
        return;
    }

    struct LampMultiUpdateReport *report = (struct LampMultiUpdateReport *) buffer;

    if (report->lamp_count > LAMP_MULTI_UPDATE_BATCH_SIZE) {
        return;
    }

    for (uint8_t i = 0; i < report->lamp_count; i++) {
        uint8_t id = report->lamp_ids[i];
        if (id > MAX_LAMP_ID) {
            return;
        }
        // TODO(bkeyes): per spec, need to check levels against allowed counts
        ctrl_update_lamp(&ctrl, id, *((struct LampValue *) &report->rgbi_tuples[i]), false);
    }

    if ((report->update_flags & LAMP_UPDATE_COMPLETE) != 0) {
        ctrl_apply_lamp_updates(&ctrl);
    }
}

static void set_report_lamp_range_update(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(struct LampRangeUpdateReport)) {
        return;
    }
    if (ctrl_get_autonomous_mode(&ctrl)) {
        return;
    }

    struct LampRangeUpdateReport *report = (struct LampRangeUpdateReport *) buffer;

    if (report->lamp_id_start > MAX_LAMP_ID || report->lamp_id_end > MAX_LAMP_ID) {
        return;
    }
    if (report->lamp_id_start > report->lamp_id_end) {
        return;
    }

    struct LampValue value = *((struct LampValue *) report->rgbi_tuple);
    for (uint8_t id = report->lamp_id_start; id <= report->lamp_id_end; id++) {
        // TODO(bkeyes): per spec, need to check levels against allowed counts
        ctrl_update_lamp(&ctrl, id, value, false);
    }

    if ((report->update_flags & LAMP_UPDATE_COMPLETE) != 0) {
        ctrl_apply_lamp_updates(&ctrl);
    }
}

static void set_report_lamp_array_control(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(struct LampArrayControlReport)) {
        return;
    }

    struct LampArrayControlReport *report = (struct LampArrayControlReport *) buffer;
    ctrl_set_autonomous_mode(&ctrl, HID_GET_FLAG(report->autonomous_mode) != 0);
}

static void set_report_vendor_12vrgb_bootsel(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(struct Vendor12VRGBBootSelReport)) {
        return;
    }

    struct Vendor12VRGBBootSelReport *report = (struct Vendor12VRGBBootSelReport *) buffer;

    if (HID_GET_FLAG(report->bootsel_restart) > 0) {
        reset_usb_boot(0, 0);
    }
}

static void set_report_vendor_12vrgb_animation(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(struct Vendor12VRGBAnimationReport)) {
        return;
    }

    struct Vendor12VRGBAnimationReport *report = (struct Vendor12VRGBAnimationReport *) buffer;

    // TODO(bkeyes): document animation parameters somewhere
    switch (report->type) {
        case ANIMATION_TYPE_NONE: {
            ctrl_set_animation(&ctrl, NULL, NULL);
            break;
        }

        case ANIMATION_TYPE_BREATHE: {
            uint32_t fade_time = (uint32_t) report->parameters[0];
            struct RGBi color = *((struct RGBi *) report->colors[0]);

            ctrl_set_animation(&ctrl, anim_fade, anim_fade_new_breathe(color, fade_time));
            break;
        }

        case ANIMATION_TYPE_FADE: {
            uint32_t color_count = (uint32_t) report->parameters[0];
            if (color_count > ANIMATION_REPORT_MAX_COLORS) {
                color_count = ANIMATION_REPORT_MAX_COLORS;
            }
            if (color_count > MAX_FADE_TARGETS) {
                color_count = MAX_FADE_TARGETS;
            }

            uint32_t fade_time = (uint32_t) report->parameters[1];
            uint32_t hold_time = (uint32_t) report->parameters[2];

            struct Labf colors[MAX_FADE_TARGETS];
            for (uint8_t i = 0; i < color_count; i++) {
                struct RGBi color = *((struct RGBi *) report->colors[i]);
                colors[i] = rgb_to_oklab(rgbi_to_f(color));
            }

            struct AnimationFade *fade = anim_fade_new_empty();
            anim_fade_set_targets(fade, colors, color_count);
            anim_fade_set_fade_time(fade, fade_time);
            for (uint8_t i = 0; i < color_count; i++) {
                anim_fade_set_hold_time(fade, i, hold_time);
            }

            ctrl_set_animation(&ctrl, anim_fade, fade);
            break;
        }
    }
}

static void set_report_vendor_12vrgb_default_animation(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(struct Vendor12VRGBAnimationReport)) {
        return;
    }

    struct Vendor12VRGBAnimationReport *report = (struct Vendor12VRGBAnimationReport *) buffer;

    // TODO(bkeyes): implement this
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    switch (report_id) {
    case HID_REPORT_ID_LAMP_ARRAY_ATTRIBUTES:
        return get_report_lamp_array_attributes(buffer, reqlen);
    case HID_REPORT_ID_LAMP_ATTRIBUTES_RESPONSE:
        return get_report_lamp_attributes_response(buffer, reqlen);
    }
    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    switch (report_type) {
    case HID_REPORT_TYPE_OUTPUT:
        switch (report_id) {
        case HID_REPORT_ID_LAMP_ATTRIBUTES_REQUEST:
            set_report_lamp_attributes_request(buffer, bufsize);
            break;
        case HID_REPORT_ID_LAMP_MULTI_UPDATE:
            set_report_lamp_multi_update(buffer, bufsize);
            break;
        case HID_REPORT_ID_LAMP_RANGE_UPDATE:
            set_report_lamp_range_update(buffer, bufsize);
            break;
        case HID_REPORT_ID_LAMP_ARRAY_CONTROL:
            set_report_lamp_array_control(buffer, bufsize);
            break;
        case HID_REPORT_ID_VENDOR_12VRGB_BOOTSEL:
            set_report_vendor_12vrgb_bootsel(buffer, bufsize);
            break;
        case HID_REPORT_ID_VENDOR_12VRGB_ANIMATION:
            set_report_vendor_12vrgb_animation(buffer, bufsize);
            break;
        }
        return;

    case HID_REPORT_TYPE_FEATURE:
        switch (report_id) {
        case HID_REPORT_ID_VENDOR_12VRGB_DEFAULT_ANIMATION:
            set_report_vendor_12vrgb_default_animation(buffer, bufsize);
            break;
        }
        return;
    }
}
