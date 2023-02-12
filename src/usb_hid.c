#include <stdbool.h>
#include <stdlib.h>

#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include "tusb.h"

#include "controller/animations/fade.h"
#include "controller/controller.h"
#include "controller/persist.h"
#include "controller/sensor.h"
#include "device/lamp.h"
#include "device/specs.h"
#include "hid/descriptor.h"
#include "hid/lights/report.h"
#include "hid/lights/usage.h"
#include "hid/sensor/report.h"
#include "hid/sensor/usage.h"
#include "hid/vendor/report.h"
#include "hid/vendor/usage.h"

#define RESET_STD_REBOOT_DELAY 100

extern controller_t ctrl;
extern sensor_controller_t sensectrl;

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

static uint16_t get_report_temperature(uint8_t *buffer, uint16_t reqlen)
{
    if (reqlen < sizeof(struct EnvironmentalTemperatureInputReport)) {
        return 0;
    }

    struct EnvironmentalTemperatureInputReport *report = (struct EnvironmentalTemperatureInputReport *) buffer;
    ctrl_sensor_get_temperature(&sensectrl, report);

    return sizeof(struct EnvironmentalTemperatureInputReport);
}

static uint16_t get_report_temperature_feature(uint8_t *buffer, uint16_t reqlen)
{
    if (reqlen < sizeof(struct EnvironmentalTemperatureFeatureReport)) {
        return 0;
    }

    struct EnvironmentalTemperatureFeatureReport *report = (struct EnvironmentalTemperatureFeatureReport *) buffer;
    ctrl_sensor_get_features(&sensectrl, report);

    return sizeof(struct EnvironmentalTemperatureFeatureReport);
}

static void set_report_lamp_attributes_request(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(struct LampAttributesRequestReport)) {
        return;
    }

    struct LampAttributesRequestReport *report = (struct LampAttributesRequestReport *) buffer;
    ctrl_set_next_lamp_attributes_id(&ctrl, report->lamp_id);
}

static inline bool is_valid_rgbi_tuple(uint8_t *rgbi)
{
    return rgbi[0] <= LAMP_COLOR_LEVELS
        && rgbi[1] <= LAMP_COLOR_LEVELS
        && rgbi[2] <= LAMP_COLOR_LEVELS
        && rgbi[3] <= LAMP_INTENSITY_LEVELS;
}

static void set_report_lamp_multi_update(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(struct LampMultiUpdateReport)) {
        return;
    }

    // Reject updates if device is running in autonomous mode
    if (ctrl_get_autonomous_mode(&ctrl)) {
        return;
    }

    struct LampMultiUpdateReport *report = (struct LampMultiUpdateReport *) buffer;

    // Validate input, reject report if any parameters are invalid
    if (report->lamp_count > LAMP_MULTI_UPDATE_BATCH_SIZE) {
        return;
    }
    for (uint8_t i = 0; i < report->lamp_count; i++) {
        if (report->lamp_ids[i] > MAX_LAMP_ID) {
            return;
        }
        if (!is_valid_rgbi_tuple(report->rgbi_tuples[i])) {
            return;
        }
    }

    for (uint8_t i = 0; i < report->lamp_count; i++) {
        ctrl_update_lamp(&ctrl, report->lamp_ids[i], *((struct LampValue *) report->rgbi_tuples[i]), false);
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

    // Reject updates if device is running in autonomous mode
    if (ctrl_get_autonomous_mode(&ctrl)) {
        return;
    }

    struct LampRangeUpdateReport *report = (struct LampRangeUpdateReport *) buffer;

    // Validate input, reject report if any parameters are invalid
    if (report->lamp_id_start > MAX_LAMP_ID || report->lamp_id_end > MAX_LAMP_ID) {
        return;
    }
    if (report->lamp_id_start > report->lamp_id_end) {
        return;
    }
    if (!is_valid_rgbi_tuple(report->rgbi_tuple)) {
        return;
    }

    struct LampValue value = *((struct LampValue *) report->rgbi_tuple);
    for (uint8_t id = report->lamp_id_start; id <= report->lamp_id_end; id++) {
        ctrl_update_lamp(&ctrl, id, value, false);
    }

    if (report->update_flags & LAMP_UPDATE_COMPLETE) {
        ctrl_apply_lamp_updates(&ctrl);
    }
}

static void set_report_lamp_array_control(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(struct LampArrayControlReport)) {
        return;
    }

    struct LampArrayControlReport *report = (struct LampArrayControlReport *) buffer;
    ctrl_set_autonomous_mode(&ctrl, report->autonomous_mode);
}

static void set_report_temperature_feature(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(struct EnvironmentalTemperatureFeatureReport)) {
        return;
    }

    struct EnvironmentalTemperatureFeatureReport *report = (struct EnvironmentalTemperatureFeatureReport *) buffer;

    ctrl_sensor_set_reporting_state(&sensectrl, report->reporting_state);
    ctrl_sensor_set_power_state(&sensectrl, report->power_state);
    ctrl_sensor_set_report_interval(&sensectrl, report->report_interval);
}

static void set_report_vendor_12vrgb_reset(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(struct Vendor12VRGBResetReport)) {
        return;
    }

    struct Vendor12VRGBResetReport *report = (struct Vendor12VRGBResetReport *) buffer;

    if (report->flags & VENDOR_RESET_FLAG_CLEAR_FLASH) {
        ctrl_persist_clear();
    }
    if (report->flags & VENDOR_RESET_FLAG_BOOTSEL) {
        reset_usb_boot(0, 0);
    } else {
        watchdog_reboot(0, 0, RESET_STD_REBOOT_DELAY);
    }
}

static void set_report_vendor_12vrgb_animation(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(struct Vendor12VRGBAnimationReport)) {
        return;
    }

    struct Vendor12VRGBAnimationReport *report = (struct Vendor12VRGBAnimationReport *) buffer;

    if (report->lamp_id > MAX_LAMP_ID) {
        return;
    }
    ctrl_set_animation_from_report(&ctrl, report);
}

static void set_report_vendor_12vrgb_default_animation(uint8_t const *buffer, uint16_t bufsize)
{
    if (bufsize < sizeof(struct Vendor12VRGBAnimationReport)) {
        return;
    }

    struct Vendor12VRGBAnimationReport *report = (struct Vendor12VRGBAnimationReport *) buffer;

    if (report->lamp_id > MAX_LAMP_ID) {
        return;
    }
    ctrl_persist_save_report(report);
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    switch (report_type) {
    case HID_REPORT_TYPE_INPUT:
        switch (report_id) {
        case HID_REPORT_ID_TEMPERATURE:
            return get_report_temperature(buffer, reqlen);
        }
        break;

    case HID_REPORT_TYPE_FEATURE:
        switch (report_id) {
        case HID_REPORT_ID_LAMP_ARRAY_ATTRIBUTES:
            return get_report_lamp_array_attributes(buffer, reqlen);
        case HID_REPORT_ID_LAMP_ATTRIBUTES_RESPONSE:
            return get_report_lamp_attributes_response(buffer, reqlen);
        case HID_REPORT_ID_TEMPERATURE:
            return get_report_temperature_feature(buffer, reqlen);
        }
        break;
    }

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    // Received data on OUT endpoint, convert to standard format for processing
    if (report_id == 0 && report_type == 0) {
        if (bufsize == 0) {
            return;
        }
        report_type = HID_REPORT_TYPE_OUTPUT;
        report_id = buffer[0];
        buffer++;
        bufsize--;
    }

    switch (report_type) {
    case HID_REPORT_TYPE_OUTPUT:
        switch (report_id) {
        case HID_REPORT_ID_LAMP_MULTI_UPDATE:
            set_report_lamp_multi_update(buffer, bufsize);
            break;
        case HID_REPORT_ID_LAMP_RANGE_UPDATE:
            set_report_lamp_range_update(buffer, bufsize);
            break;
        case HID_REPORT_ID_VENDOR_12VRGB_ANIMATION:
            set_report_vendor_12vrgb_animation(buffer, bufsize);
            break;
        }
        return;

    case HID_REPORT_TYPE_FEATURE:
        switch (report_id) {
        case HID_REPORT_ID_LAMP_ATTRIBUTES_REQUEST:
            set_report_lamp_attributes_request(buffer, bufsize);
            break;
        case HID_REPORT_ID_LAMP_ARRAY_CONTROL:
            set_report_lamp_array_control(buffer, bufsize);
            break;
        case HID_REPORT_ID_TEMPERATURE:
            set_report_temperature_feature(buffer, bufsize);
            break;
        case HID_REPORT_ID_VENDOR_12VRGB_RESET:
            set_report_vendor_12vrgb_reset(buffer, bufsize);
            break;
        case HID_REPORT_ID_VENDOR_12VRGB_DEFAULT_ANIMATION:
            set_report_vendor_12vrgb_default_animation(buffer, bufsize);
            break;
        }
        return;
    }
}
