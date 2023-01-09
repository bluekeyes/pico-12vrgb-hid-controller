#include "pico/bootrom.h"
#include "tusb.h"

#include "config.h"
#include "controller/controller.h"
#include "hid/data.h"
#include "hid/descriptor.h"
#include "hid/lights/report.h"
#include "hid/lights/usage.h"
#include "hid/vendor/report.h"
#include "rgb/rgb.h"

extern controller_t ctrl;

static uint16_t get_report_lamp_array_attributes(uint8_t *buffer, uint16_t reqlen)
{
    if (reqlen < sizeof(lamp_array_attributes_report_t)) {
        return 0;
    }

    lamp_array_attributes_report_t *report = (lamp_array_attributes_report_t *) buffer;
    report->lamp_count = CFG_RGB_LAMP_COUNT;
    report->bounding_box_width = CFG_RGB_BOUNDING_BOX_WIDTH;
    report->bounding_box_height = CFG_RGB_BOUNDING_BOX_HEIGHT;
    report->bounding_box_depth = CFG_RGB_BOUNDING_BOX_DEPTH;
    report->min_update_interval = CFG_RGB_MINIMUM_UPDATE_INTERVAL;
    report->lamp_kind = LAMP_ARRAY_KIND_CHASSIS;

    return sizeof(report);
}

static uint16_t get_report_lamp_attributes_response(uint8_t *buffer, uint16_t reqlen)
{
    if (reqlen < sizeof(lamp_attributes_response_report_t)) {
        return 0;
    }
    ctrl_get_lamp_attributes(&ctrl, (lamp_attributes_response_report_t *) buffer);
    return sizeof(lamp_attributes_response_report_t);
}

static void set_report_lamp_attributes_request(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(lamp_attributes_request_report_t)) {
        return;
    }

    lamp_attributes_request_report_t *report = (lamp_attributes_request_report_t *) buffer;
    ctrl_set_next_lamp_attributes_id(&ctrl, report->lamp_id);
}

static void set_report_lamp_multi_update(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(lamp_multi_update_report_t)) {
        return;
    }
    if (ctrl_get_autonomous_mode(&ctrl)) {
        return;
    }

    lamp_multi_update_report_t *report = (lamp_multi_update_report_t *) buffer;

    if (report->lamp_count > LAMP_MULTI_UPDATE_BATCH_SIZE) {
        return;
    }

    for (uint8_t i = 0; i < report->lamp_count; i++) {
        uint8_t id = report->lamp_ids[i];
        if (id > CFG_RGB_LAMP_COUNT - 1) {
            return;
        }
        // TODO(bkeyes): per spec, need to check levels against allowed counts
        ctrl_update_lamp(&ctrl, id, (rgb_tuple_t *) &report->rgbi_tuples[i], false);
    }

    if ((report->update_flags & LAMP_UPDATE_COMPLETE) != 0) {
        ctrl_apply_lamp_updates(&ctrl);
    }
}

static void set_report_lamp_range_update(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(lamp_range_update_report_t)) {
        return;
    }
    if (ctrl_get_autonomous_mode(&ctrl)) {
        return;
    }

    lamp_range_update_report_t *report = (lamp_range_update_report_t *) buffer;

    if (report->lamp_id_start > CFG_RGB_LAMP_COUNT - 1 || report->lamp_id_end > CFG_RGB_LAMP_COUNT - 1) {
        return;
    }
    if (report->lamp_id_start > report->lamp_id_end) {
        return;
    }

    rgb_tuple_t *tuple = (rgb_tuple_t *) report->rgbi_tuple;
    for (uint8_t id = report->lamp_id_start; id <= report->lamp_id_end; id++) {
        // TODO(bkeyes): per spec, need to check levels against allowed counts
        ctrl_update_lamp(&ctrl, id, tuple, false);
    }

    if ((report->update_flags & LAMP_UPDATE_COMPLETE) != 0) {
        ctrl_apply_lamp_updates(&ctrl);
    }
}

static void set_report_lamp_array_control(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(lamp_array_control_report_t)) {
        return;
    }

    lamp_array_control_report_t *report = (lamp_array_control_report_t *) buffer;
    ctrl_set_autonomous_mode(&ctrl, HID_GET_FLAG(report->autonomous_mode) != 0);
}

static void set_report_vendor_12vrgb_bootsel(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(vendor_12vrgb_bootsel_report_t)) {
        return;
    }

    vendor_12vrgb_bootsel_report_t *report = (vendor_12vrgb_bootsel_report_t *) buffer;

    if (HID_GET_FLAG(report->bootsel_restart) > 0) {
        reset_usb_boot(0, 0);
    }
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
    switch (report_id) {
        case HID_REPORT_ID_LAMP_ATTRIBUTES_REQUEST:
            set_report_lamp_attributes_request(buffer, bufsize);
            return;
        case HID_REPORT_ID_LAMP_MULTI_UPDATE:
            set_report_lamp_multi_update(buffer, bufsize);
            return;
        case HID_REPORT_ID_LAMP_RANGE_UPDATE:
            set_report_lamp_range_update(buffer, bufsize);
            return;
        case HID_REPORT_ID_LAMP_ARRAY_CONTROL:
            set_report_lamp_array_control(buffer, bufsize);
            return;
        case HID_REPORT_ID_VENDOR_12VRGB_BOOTSEL:
            set_report_vendor_12vrgb_bootsel(buffer, bufsize);
            return;
    }
}
