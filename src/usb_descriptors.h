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

#define HID_USAGE_PAGE_VENDOR_12VRGB    0xFF00

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

// TODO(bkeyes): extract report definitions to macros, co-locate here or in a new header

typedef struct TU_ATTR_PACKED {
    uint16_t lamp_count;
    int32_t  bounding_box_width;
    int32_t  bounding_box_height;
    int32_t  bounding_box_depth;
    int32_t  min_update_interval;
    uint8_t  lamp_kind;
} lamp_array_attributes_report_t;

typedef struct TU_ATTR_PACKED {
    uint8_t lamp_id;
} lamp_attributes_request_report_t;

typedef struct TU_ATTR_PACKED {
    uint8_t  lamp_id;
    int32_t  position_x;
    int32_t  position_y;
    int32_t  position_z;
    uint16_t lamp_purpose;
    int32_t  update_latency;
    uint8_t  red_level_count;
    uint8_t  green_level_count;
    uint8_t  blue_level_count;
    uint8_t  intensity_level_count;
    uint8_t  is_programmable;
    uint16_t input_binding;
} lamp_attributes_response_report_t;

typedef struct TU_ATTR_PACKED {
    uint8_t  lamp_count;
    uint8_t  lamp_ids[CFG_RGB_MULTI_UPDATE_SIZE];
    uint8_t  rgbi_tuples[4 * CFG_RGB_MULTI_UPDATE_SIZE];
    uint16_t update_flags;
} lamp_multi_update_report_t;

typedef struct TU_ATTR_PACKED {
    uint8_t  lamp_id_start;
    uint8_t  lamp_id_end;
    uint8_t  rgbi_tuple[4];
    uint16_t update_flags;
} lamp_range_update_report_t;

typedef struct TU_ATTR_PACKED {
    uint8_t autonomous_mode;
} lamp_array_control_report_t;

typedef struct TU_ATTR_PACKED {
    uint8_t bootsel_restart;
} vendor_12vrgb_bootsel_report_t;

#endif /* USB_DESCRIPTORS_H_ */
