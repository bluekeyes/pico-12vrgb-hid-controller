#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include <stdint.h>

#include "tusb.h"

#include "config.h"

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
    HID_LOGICAL_MAX_N   (UINT8_MAX, 1),     \
    HID_REPORT_SIZE     (8),                \
    HID_REPORT_COUNT    (COUNT),            \
    HID_##TYPE          (FLAGS)

// Adds COUNT uint16 items of TYPE to the report with FLAGS
#define HID_ITEM_UINT16(TYPE, COUNT, FLAGS) \
    HID_LOGICAL_MAX_N   (UINT16_MAX, 2),    \
    HID_REPORT_SIZE     (16),               \
    HID_REPORT_COUNT    (COUNT),            \
    HID_##TYPE          (FLAGS)

// Adds COUNT int32 items of TYPE to the report with FLAGS
#define HID_ITEM_INT32(TYPE, COUNT, FLAGS)  \
    HID_LOGICAL_MAX_N   (INT32_MAX, 3),     \
    HID_REPORT_SIZE     (32),               \
    HID_REPORT_COUNT    (COUNT),            \
    HID_##TYPE          (FLAGS)

enum {
    HID_USAGE_PAGE_LIGHTING         = 0x59,
    HID_USAGE_PAGE_VENDOR_12VRGB    = 0xFF00,
};

enum {
    HID_USAGE_LIGHTING_LAMP_ARRAY                           = 0x01,
    HID_USAGE_LIGHTING_LAMP_ARRAY_ATTRIBUTES_REPORT         = 0x02,
    HID_USAGE_LIGHTING_LAMP_COUNT                           = 0x03,
    HID_USAGE_LIGHTING_BOUNDING_BOX_WIDTH_IN_MICROMETERS    = 0x04,
    HID_USAGE_LIGHTING_BOUNDING_BOX_HEIGHT_IN_MICROMETERS   = 0x05,
    HID_USAGE_LIGHTING_BOUNDING_BOX_DEPTH_IN_MICROMETERS    = 0x06,
    HID_USAGE_LIGHTING_LAMP_ARRAY_KIND                      = 0x07,
    HID_USAGE_LIGHTING_MIN_UPDATE_INTERVAL_IN_MICROSECONDS  = 0x08,
    HID_USAGE_LIGHTING_LAMP_ATTRIBUTES_REQUEST_REPORT       = 0x20,
    HID_USAGE_LIGHTING_LAMP_ID                              = 0x21,
    HID_USAGE_LIGHTING_LAMP_ATTRIBUTES_RESPONSE_REPORT      = 0x22,
    HID_USAGE_LIGHTING_POSITION_X_IN_MICROMETERS            = 0x23,
    HID_USAGE_LIGHTING_POSITION_Y_IN_MICROMETERS            = 0x24,
    HID_USAGE_LIGHTING_POSITION_Z_IN_MICROMETERS            = 0x25,
    HID_USAGE_LIGHTING_LAMP_PURPOSES                        = 0x26,
    HID_USAGE_LIGHTING_UPDATE_LATENCY_IN_MICROSECONDS       = 0x27,
    HID_USAGE_LIGHTING_RED_LEVEL_COUNT                      = 0x28,
    HID_USAGE_LIGHTING_GREEN_LEVEL_COUNT                    = 0x29,
    HID_USAGE_LIGHTING_BLUE_LEVEL_COUNT                     = 0x2A,
    HID_USAGE_LIGHTING_INTENSITY_LEVEL_COUNT                = 0x2B,
    HID_USAGE_LIGHTING_IS_PROGRAMMABLE                      = 0x2C,
    HID_USAGE_LIGHTING_INPUT_BINDING                        = 0x2D,
    HID_USAGE_LIGHTING_LAMP_MULTI_UPDATE_REPORT             = 0x50,
    HID_USAGE_LIGHTING_RED_UPDATE_CHANNEL                   = 0x51,
    HID_USAGE_LIGHTING_GREEN_UPDATE_CHANNEL                 = 0x52,
    HID_USAGE_LIGHTING_BLUE_UPDATE_CHANNEL                  = 0x53,
    HID_USAGE_LIGHTING_INTENSITY_UPDATE_CHANNEL             = 0x54,
    HID_USAGE_LIGHTING_LAMP_UPDATE_FLAGS                    = 0x55,
    HID_USAGE_LIGHTING_LAMP_RANGE_UPDATE_REPORT             = 0x60,
    HID_USAGE_LIGHTING_LAMP_ID_START                        = 0x61,
    HID_USAGE_LIGHTING_LAMP_ID_END                          = 0x62,
    HID_USAGE_LIGHTING_LAMP_ARRAY_CONTROL_REPORT            = 0x70,
    HID_USAGE_LIGHTING_AUTONOMOUS_MODE                      = 0x71,
};

enum {
    HID_USAGE_VENDOR_12VRGB_CONTROLLER          = 0x01,
    HID_USAGE_VENDOR_12VRGB_BOOTSEL_REPORT      = 0x02,
    HID_USAGE_VENDOR_12VRGB_BOOTSEL_RESTART     = 0x03,
};

enum {
    // Lighting and Illumination Reports
    HID_REPORT_ID_LAMP_ARRAY_ATTRIBUTES = 1,
    HID_REPORT_ID_LAMP_ATTRIBUTES_REQUEST,
    HID_REPORT_ID_LAMP_ATTRIBUTES_RESPONSE,
    HID_REPORT_ID_LAMP_MULTI_UPDATE,
    HID_REPORT_ID_LAMP_RANGE_UPDATE,
    HID_REPORT_ID_LAMP_ARRAY_CONTROL,

    // Vendor Reports
    HID_REPORT_ID_VENDOR_12VRGB_BOOTSEL,
};

// --------------
// Report Structs
// --------------
//
// NOTE: these must exactly match the data layout given by the report
// descriptors and all multi-byte values must be little-endian. This works out
// nicely on the RP2040, but could be a problem on other platforms.

typedef struct TU_ATTR_PACKED {
    uint16_t lamp_count;
    int32_t bounding_box_width;
    int32_t bounding_box_height;
    int32_t bounding_box_depth;
    int32_t min_update_interval;
    uint8_t lamp_kind;
} lamp_array_attributes_report_t;

typedef struct TU_ATTR_PACKED {
    uint8_t lamp_id;
} lamp_attributes_request_report_t;

typedef struct TU_ATTR_PACKED {
    uint8_t lamp_id;
    int32_t position_x;
    int32_t position_y;
    int32_t position_z;
    uint16_t lamp_purpose;
    int32_t update_latency;
    uint8_t red_level_count;
    uint8_t green_level_count;
    uint8_t blue_level_count;
    uint8_t intensity_level_count;
    uint8_t is_programmable;
    uint16_t input_binding;
} lamp_attributes_response_report_t;

// --------------------
// LampArrayKind Values
// --------------------

enum {
    LAMP_ARRAY_KIND_KEYBOARD         = 0x01,
    LAMP_ARRAY_KIND_MOUSE            = 0x02,
    LAMP_ARRAY_KIND_GAME_CONTROLLER  = 0x03,
    LAMP_ARRAY_KIND_PERIPHERAL       = 0x04,
    LAMP_ARRAY_KIND_SCENE            = 0x05,
    LAMP_ARRAY_KIND_NOTIFICATION     = 0x06,
    LAMP_ARRAY_KIND_CHASSIS          = 0x07,
    LAMP_ARRAY_KIND_WEARABLE         = 0x08,
    LAMP_ARRAY_KIND_FURNITURE        = 0x09,
    LAMP_ARRAY_KIND_ART              = 0x0A,
};

// ----------------
// LampUpdate Flags
// ----------------

#define LAMP_UPDATE_COMPLETE        0x01

#endif /* USB_DESCRIPTORS_H_ */
