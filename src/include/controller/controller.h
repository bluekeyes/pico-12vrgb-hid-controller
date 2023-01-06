#ifndef CONTROLLER_CONTROLLER_H_
#define CONTROLLER_CONTROLLER_H_

#include "hid/lights/report.h"
#include "rgb/rgb.h"

typedef struct {
    bool autonomous_mode;
    rgb_lamp_id_t next_lamp_id;
} controller_t;

void ctrl_init(controller_t *ctrl);
void ctrl_set_next_lamp_attributes_id(controller_t *ctrl, rgb_lamp_id_t lamp_id);
void ctrl_get_lamp_attributes(controller_t *ctrl, lamp_attributes_response_report_t *report);

#endif // CONTROLLER_CONTROLLER_H_
