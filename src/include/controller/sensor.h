#ifndef CONTROLLER_SENSOR_H_
#define CONTROLLER_SENSOR_H_

#include <stdint.h>

#include "pico/time.h"

#include "hid/sensor/report.h"
#include "hid/sensor/usage.h"

struct SensorController {
    enum SensorReportingState temp_sensor_reporting_state;
    enum SensorPowerState temp_sensor_power_state; 
    uint32_t temp_sensor_report_interval;

    absolute_time_t temp_sensor_last_report;
};
typedef struct SensorController sensor_controller_t;

void ctrl_sensor_init(sensor_controller_t *ctrl);
void ctrl_sensor_task(sensor_controller_t *ctrl);

void ctrl_sensor_get_temp_features(sensor_controller_t *ctrl, struct EnvironmentalTemperatureFeatureReport *report);
void ctrl_sensor_get_temperature(sensor_controller_t *ctrl, struct EnvironmentalTemperatureInputReport *report);

void ctrl_sensor_set_temp_reporting_state(sensor_controller_t *ctrl, enum SensorReportingState state);
void ctrl_sensor_set_temp_power_state(sensor_controller_t *ctrl, enum SensorPowerState state);
void ctrl_sensor_set_temp_report_interval(sensor_controller_t *ctrl, uint32_t interval);

#endif /* CONTROLLER_SENSOR_H_ */
