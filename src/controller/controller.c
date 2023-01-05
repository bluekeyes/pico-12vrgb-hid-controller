#include "config.h"
#include "controller/controller.h"
#include "hid/data.h"
#include "hid/lights/report.h"

void ctrl_init(controller_t *ctrl)
{
    ctrl->autonomous_mode = true;
    ctrl->next_lamp_id = 0;
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

    size_t pos_idx = 3 * ((size_t) lamp_id);
    report->position_x = rgb_lamp_positions[pos_idx];
    report->position_y = rgb_lamp_positions[pos_idx + 1];
    report->position_z = rgb_lamp_positions[pos_idx + 2];

    report->red_level_count = RGB_COLOR_LEVEL_COUNT;
    report->green_level_count = RGB_COLOR_LEVEL_COUNT;
    report->blue_level_count = RGB_COLOR_LEVEL_COUNT;
    report->intensity_level_count = RGB_INTENSITY_LEVEL_COUNT;

    report->update_latency = CFG_RGB_LAMP_UPDATE_LATENCY;
    report->input_binding = 0x0000;
}
