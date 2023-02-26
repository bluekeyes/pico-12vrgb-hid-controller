#ifndef HID_SENSOR_REPORT_H_
#define HID_SENSOR_REPORT_H_

#include <stdint.h>

#include "tusb.h"

#include "hid/sensor/usage.h"

#define HID_COLLECTION_SENSOR \
    HID_USAGE_PAGE  (HID_USAGE_PAGE_SENSOR), \
    HID_USAGE       (HID_USAGE_SENSOR_TYPE_COLLECTION), \
    HID_COLLECTION  (HID_COLLECTION_APPLICATION), \
        HID_LOGICAL_MIN     (0), \
        HID_UNIT_EXPONENT   (0)

// --------------------------
// Environmental: Temperature
// --------------------------

#define HID_REPORT_DESC_ENVIRONMENTAL_TEMPERATURE(REPORT_ID) \
    HID_REPORT_ID   (HID_REPORT_ID_TEMPERATURE) \
    HID_USAGE       (HID_USAGE_SENSOR_TYPE_ENVIRONMENTAL_TEMPERATURE), \
    HID_COLLECTION  (HID_COLLECTION_PHYSICAL), \
        /* -------------- */ \
        /* Feature Report */ \
        /* -------------- */ \
        /* Property: Connection Type */ \
        HID_USAGE_N         (HID_USAGE_SENSOR_PROPERTY_SENSOR_CONNECTION_TYPE, 2), \
        HID_LOGICAL_MAX     (2), \
        HID_REPORT_SIZE     (8), \
        HID_REPORT_COUNT    (1), \
        HID_COLLECTION      (HID_COLLECTION_LOGICAL), \
            HID_USAGE_MIN_N (HID_USAGE_SENSOR_PROPERTY_CONNECTION_TYPE_PC_INTEGRATED, 2), \
            HID_USAGE_MAX_N (HID_USAGE_SENSOR_PROPERTY_CONNECTION_TYPE_PC_EXTERNAL, 2), \
            HID_FEATURE     (HID_DATA | HID_ARRAY | HID_ABSOLUTE), \
        HID_COLLECTION_END, \
        /* Property: Reporting Type*/ \
        HID_USAGE_N         (HID_USAGE_SENSOR_PROPERTY_REPORTING_STATE, 2), \
        HID_LOGICAL_MAX     (5), \
        HID_REPORT_SIZE     (8), \
        HID_REPORT_COUNT    (1), \
        HID_COLLECTION      (HID_COLLECTION_LOGICAL), \
            HID_USAGE_MIN_N (HID_USAGE_SENSOR_PROPERTY_REPORTING_STATE_REPORT_NO_EVENTS, 2), \
            HID_USAGE_MAX_N (HID_USAGE_SENSOR_PROPERTY_REPORTING_STATE_REPORT_WAKE_ON_THRESHOLD_EVENTS, 2), \
            HID_FEATURE     (HID_DATA | HID_ARRAY | HID_ABSOLUTE), \
        HID_COLLECTION_END, \
        /* Property: Power State */ \
        HID_USAGE_N         (HID_USAGE_SENSOR_PROPERTY_POWER_STATE, 2), \
        HID_LOGICAL_MAX     (5), \
        HID_REPORT_SIZE     (8), \
        HID_REPORT_COUNT    (1), \
        HID_COLLECTION      (HID_COLLECTION_LOGICAL), \
            HID_USAGE_MIN_N (HID_USAGE_SENSOR_PROPERTY_POWER_STATE_UNDEFINED, 2), \
            HID_USAGE_MAX_N (HID_USAGE_SENSOR_PROPERTY_POWER_STATE_D4_POWER_OFF, 2), \
            HID_FEATURE     (HID_DATA | HID_ARRAY | HID_ABSOLUTE), \
        HID_COLLECTION_END, \
        /* Property: Sensor State */ \
        HID_USAGE_N         (HID_USAGE_SENSOR_STATE, 2), \
        HID_LOGICAL_MAX     (6), \
        HID_REPORT_SIZE     (8), \
        HID_REPORT_COUNT    (1), \
        HID_COLLECTION      (HID_COLLECTION_LOGICAL), \
            HID_USAGE_MIN_N (HID_USAGE_SENSOR_STATE_UNDEFINED, 2), \
            HID_USAGE_MAX_N (HID_USAGE_SENSOR_STATE_ERROR, 2), \
            HID_FEATURE     (HID_DATA | HID_ARRAY | HID_ABSOLUTE), \
        HID_COLLECTION_END, \
        /* Property: Report Interval */ \
        HID_USAGE_N         (HID_USAGE_SENSOR_PROPERTY_REPORT_INTERVAL, 2), \
        HID_LOGICAL_MAX_N   (UINT32_MAX, 3), \
        HID_REPORT_SIZE     (32), \
        HID_REPORT_COUNT    (1), \
        HID_FEATURE         (HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        /* ------------ */ \
        /* Input Report */ \
        /* ------------ */ \
        /* Event: Sensor State */ \
        HID_USAGE_N         (HID_USAGE_SENSOR_STATE, 2), \
        HID_LOGICAL_MAX     (6), \
        HID_REPORT_SIZE     (8), \
        HID_REPORT_COUNT    (1), \
        HID_COLLECTION      (HID_COLLECTION_LOGICAL), \
            HID_USAGE_MIN_N (HID_USAGE_SENSOR_STATE_UNDEFINED, 2), \
            HID_USAGE_MAX_N (HID_USAGE_SENSOR_STATE_ERROR, 2), \
            HID_INPUT       (HID_DATA | HID_ARRAY | HID_ABSOLUTE), \
        HID_COLLECTION_END, \
        /* Event: Sensor Event */ \
        HID_USAGE_N         (HID_USAGE_SENSOR_EVENT, 2), \
        HID_LOGICAL_MAX     (16), \
        HID_REPORT_SIZE     (8), \
        HID_REPORT_COUNT    (1), \
        HID_COLLECTION      (HID_COLLECTION_LOGICAL), \
            HID_USAGE_MIN_N (HID_USAGE_SENSOR_EVENT_UNKNOWN, 2), \
            HID_USAGE_MAX_N (HID_USAGE_SENSOR_EVENT_CHANGE_SENSITIVITY, 2), \
            HID_INPUT       (HID_DATA | HID_ARRAY | HID_ABSOLUTE), \
        HID_COLLECTION_END, \
        /* Data: Environmental Temperature */ \
        HID_USAGE_N         (HID_USAGE_SENSOR_DATA_ENVIRONMENTAL_TEMPERATURE, 2), \
        HID_LOGICAL_MIN_N   (INT16_MIN, 2), \
        HID_LOGICAL_MAX_N   (INT16_MAX, 2), \
        HID_REPORT_SIZE     (16), \
        HID_REPORT_COUNT    (1), \
        HID_UNIT_EXPONENT   (0x0E), \
        HID_INPUT           (HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
    HID_COLLECTION_END

struct __attribute__ ((packed)) EnvironmentalTemperatureFeatureReport {
    uint8_t sensor_connection_type;
    uint8_t reporting_state;
    uint8_t power_state;
    uint8_t sensor_state;
    uint32_t report_interval;
};

struct __attribute__ ((packed)) EnvironmentalTemperatureInputReport {
    uint8_t sensor_state;
    uint8_t sensor_event;
    int16_t temperature;
};

#endif /* HID_SENSOR_REPORT_H_ */
