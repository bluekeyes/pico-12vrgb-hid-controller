#ifndef CONTROLLER_CONTROLLER_H_
#define CONTROLLER_CONTROLLER_H_

#include "config.h"
#include "hid/lights/report.h"
#include "rgb/rgb.h"

typedef struct {
    rgb_tuple_t current;
    rgb_tuple_t next;
    bool dirty;
} lamp_state;

typedef struct {
    bool autonomous_mode;
    rgb_lamp_id_t next_lamp_id;

    bool do_update;
    lamp_state lamp_state[CFG_RGB_LAMP_COUNT];
} controller_t;

void ctrl_init(controller_t *ctrl);
void ctrl_task(controller_t *ctrl);

void ctrl_set_next_lamp_attributes_id(controller_t *ctrl, rgb_lamp_id_t lamp_id);
void ctrl_get_lamp_attributes(controller_t *ctrl, lamp_attributes_response_report_t *report);

void ctrl_update_lamp(controller_t *ctrl, rgb_lamp_id_t lamp_id, rgb_tuple_t *tuple);
void ctrl_apply_lamp_updates(controller_t *ctrl);

#endif // CONTROLLER_CONTROLLER_H_
