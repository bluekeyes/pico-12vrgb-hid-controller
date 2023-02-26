#include <stdint.h>

#include "pico/time.h"
#include "tusb.h"

#include "controller/sensor.h"
#include "device/temperature.h"
#include "hid/descriptor.h"
#include "hid/sensor/report.h"
#include "hid/sensor/usage.h"

#define INITIAL_REPORT_INTERVAL_MS 500

void ctrl_sensor_init(sensor_controller_t *ctrl)
{
    ctrl->reporting_state = SENSOR_REPORTING_STATE_REPORT_NO_EVENTS;
    ctrl->power_state = SENSOR_POWER_STATE_D4_POWER_OFF;
    ctrl->report_interval = INITIAL_REPORT_INTERVAL_MS;
}

void ctrl_sensor_task(sensor_controller_t *ctrl)
{
    if (ctrl->reporting_state == SENSOR_REPORTING_STATE_REPORT_NO_EVENTS ||
            ctrl->power_state == SENSOR_POWER_STATE_D4_POWER_OFF) {
        return;
    }

    absolute_time_t now = get_absolute_time();
    int64_t elapsed_us = absolute_time_diff_us(ctrl->last_report, now);

    if (tud_hid_ready() && elapsed_us >= 1000 * ctrl->report_interval) {
        struct EnvironmentalTemperatureInputReport report;
        ctrl_sensor_get_temperature(ctrl, &report);
        report.sensor_event = SENSOR_EVENT_DATA_UPDATED;

        tud_hid_report(HID_REPORT_ID_TEMPERATURE, &report, sizeof(report));
        ctrl->last_report = now;
    }
}

void ctrl_sensor_get_features(sensor_controller_t *ctrl, struct EnvironmentalTemperatureFeatureReport *report)
{
    report->sensor_connection_type = SENSOR_CONNECTION_TYPE_PC_INTEGRATED;
    report->sensor_state = SENSOR_STATE_READY;

    report->reporting_state = ctrl->reporting_state;
    report->power_state = ctrl->power_state;
    report->report_interval = ctrl->report_interval;
}

void ctrl_sensor_get_temperature(sensor_controller_t *ctrl, struct EnvironmentalTemperatureInputReport *report)
{
    report->sensor_state = SENSOR_STATE_READY;
    report->sensor_event = SENSOR_EVENT_POLL_RESPONSE;
    report->temperature = temperature_read();
}

void ctrl_sensor_set_reporting_state(sensor_controller_t *ctrl, enum SensorReportingState state)
{
    ctrl->reporting_state = state;
}

void ctrl_sensor_set_power_state(sensor_controller_t *ctrl, enum SensorPowerState state)
{
    ctrl->power_state = state;
}

void ctrl_sensor_set_report_interval(sensor_controller_t *ctrl, uint32_t interval)
{
    ctrl->report_interval = interval;
}
