#include "tusb.h"

#include "config.h"
#include "rgb.h"
#include "usb_descriptors.h"

static uint16_t get_lamp_array_attributes_report(uint8_t *buffer, uint16_t reqlen)
{
    // TODO(bkeyes): unclear if this is necessary... the buffer should always
    // be CFG_TUD_HID_EP_BUFSIZE, but reqlen is set to the smaller of the
    // length in the request and the buffer size
    if (reqlen != sizeof(lamp_array_attributes_report_t)) {
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

static rgb_lamp_id_t next_lamp_id = 0;

static uint16_t get_lamp_attributes_response_report(uint8_t *buffer, uint16_t reqlen)
{
    // TODO(bkeyes): unclear if this is necessary
    if (reqlen != sizeof(lamp_attributes_response_report_t)) {
        return 0;
    }

    rgb_lamp_id_t lamp_id = next_lamp_id;
    next_lamp_id = (next_lamp_id + 1) % CFG_RGB_LAMP_COUNT;

    lamp_attributes_response_report_t *report = (lamp_attributes_response_report_t *) buffer;
    report->lamp_id = lamp_id;

    size_t pos_idx = 3 * ((size_t) lamp_id);
    report->position_x = rgb_lamp_positions[pos_idx];
    report->position_y = rgb_lamp_positions[pos_idx + 1];
    report->position_z = rgb_lamp_positions[pos_idx + 2];

    report->lamp_purpose = rgb_lamp_purposes[lamp_id];

    report->update_latency = CFG_RGB_LAMP_UPDATE_LATENCY;

    report->red_level_count = RGB_COLOR_LEVEL_COUNT;
    report->green_level_count = RGB_COLOR_LEVEL_COUNT;
    report->blue_level_count = RGB_COLOR_LEVEL_COUNT;
    report->intensity_level_count = RGB_INTENSITY_LEVEL_COUNT;

    report->is_programmable = 0x01;

    report->input_binding = 0x0000;

    return sizeof(lamp_attributes_response_report_t);
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    switch (report_id) {
        case HID_REPORT_ID_LAMP_ARRAY_ATTRIBUTES:
            return get_lamp_array_attributes_report(buffer, reqlen);
        case HID_REPORT_ID_LAMP_ATTRIBUTES_RESPONSE:
            return get_lamp_attributes_response_report(buffer, reqlen);
    }
    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) bufsize;
}
