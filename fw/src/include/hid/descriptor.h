/**
 * Contains utilities and constants for defining HID report descriptors.
 */

#ifndef HID_DESCRIPTOR_H_
#define HID_DESCRIPTOR_H_

#include <stdint.h>
#include "tusb.h"

#define HID_REPEAT_1(...)   __VA_ARGS__
#define HID_REPEAT_2(...)   __VA_ARGS__ __VA_ARGS__
#define HID_REPEAT_3(...)   __VA_ARGS__ __VA_ARGS__ __VA_ARGS__
#define HID_REPEAT_4(...)   __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__
#define HID_REPEAT_5(...)   __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__
#define HID_REPEAT_6(...)   __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__
#define HID_REPEAT_7(...)   __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__
#define HID_REPEAT_8(...)   __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__ __VA_ARGS__

#define HID_REPEAT_(N, ...) HID_REPEAT_##N(__VA_ARGS__) // allows N to be another macro
#define HID_REPEAT(N, ...)  HID_REPEAT_(N, __VA_ARGS__)

// Adds COUNT uint8 items of TYPE to the report with FLAGS
#define HID_ITEM_UINT8(TYPE, COUNT, FLAGS)  \
    HID_LOGICAL_MIN     (0),                \
    HID_LOGICAL_MAX_N   (UINT8_MAX, 2),     \
    HID_REPORT_SIZE     (8),                \
    HID_REPORT_COUNT    (COUNT),            \
    HID_##TYPE          (FLAGS)

// Adds COUNT uint16 items of TYPE to the report with FLAGS
#define HID_ITEM_UINT16(TYPE, COUNT, FLAGS) \
    HID_LOGICAL_MIN     (0),                \
    HID_LOGICAL_MAX_N   (UINT16_MAX, 3),    \
    HID_REPORT_SIZE     (16),               \
    HID_REPORT_COUNT    (COUNT),            \
    HID_##TYPE          (FLAGS)

// Adds COUNT int32 items of TYPE to the report with FLAGS
#define HID_ITEM_INT32(TYPE, COUNT, FLAGS)  \
    HID_LOGICAL_MIN     (0),                \
    HID_LOGICAL_MAX_N   (INT32_MAX, 3),     \
    HID_REPORT_SIZE     (32),               \
    HID_REPORT_COUNT    (COUNT),            \
    HID_##TYPE          (FLAGS)

enum {
    // Lighting and Illumination Reports
    HID_REPORT_ID_LAMP_ARRAY_ATTRIBUTES     = 0x01,
    HID_REPORT_ID_LAMP_ATTRIBUTES_REQUEST   = 0x02,
    HID_REPORT_ID_LAMP_ATTRIBUTES_RESPONSE  = 0x03,
    HID_REPORT_ID_LAMP_MULTI_UPDATE         = 0x04,
    HID_REPORT_ID_LAMP_RANGE_UPDATE         = 0x05,
    HID_REPORT_ID_LAMP_ARRAY_CONTROL        = 0x06,

    // Sensor Reports
    HID_REPORT_ID_TEMPERATURE   = 0x20,

    // Vendor Reports
    HID_REPORT_ID_VENDOR_12VRGB_RESET       = 0x30,
    HID_REPORT_ID_VENDOR_12VRGB_ANIMATION   = 0x31,
};

#endif // HID_DESCRIPTOR_H_
