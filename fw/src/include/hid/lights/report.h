/**
 * This file defines structs for the reports in the Lighting and Illumination
 * HID usage table. It also defines macros for including these reports in a
 * report descriptor.
 *
 * Note that the report structs must exactly match the data layout given by the
 * report descriptors, including that all multi-byte values are little-endian.
 * This works out by default on the RP2040, but could be a problem elsewhere.
 */

#ifndef HID_LIGHTS_REPORT_H_
#define HID_LIGHTS_REPORT_H_

#include <stdint.h>

#include "tusb.h"

#include "device/specs.h"
#include "hid/descriptor.h"
#include "hid/lights/usage.h"

#if LAMP_COUNT <= 8
    #define LAMP_MULTI_UPDATE_BATCH_SIZE LAMP_COUNT
#else
    #define LAMP_MULTI_UPDATE_BATCH_SIZE 8
#endif

#define HID_COLLECTION_LAMP_ARRAY \
    HID_USAGE_PAGE    (HID_USAGE_PAGE_LIGHTING), \
    HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ARRAY), \
    HID_COLLECTION    (HID_COLLECTION_APPLICATION)

// -------------------------
// LampArrayAttributesReport
// -------------------------

#define HID_REPORT_DESC_LAMP_ARRAY_ATTRIBUTES(REPORT_ID) \
    HID_REPORT_ID   (REPORT_ID) \
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_ARRAY_ATTRIBUTES_REPORT), \
    HID_COLLECTION  (HID_COLLECTION_LOGICAL), \
      /* LampCount */ \
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_COUNT), \
      HID_ITEM_UINT16   (FEATURE, 1, HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE), \
      /* BoundingBoxWidthInMicrometers, BoundingBoxHeightInMicrometers, BoundingBoxDepthInMicrometers */ \
      /* LampArrayKind */ \
      /* MinUpdateIntervalInMicroseconds */ \
      HID_USAGE         (HID_USAGE_LIGHTING_BOUNDING_BOX_WIDTH_IN_MICROMETERS), \
      HID_USAGE         (HID_USAGE_LIGHTING_BOUNDING_BOX_HEIGHT_IN_MICROMETERS), \
      HID_USAGE         (HID_USAGE_LIGHTING_BOUNDING_BOX_DEPTH_IN_MICROMETERS), \
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ARRAY_KIND), \
      HID_USAGE         (HID_USAGE_LIGHTING_MIN_UPDATE_INTERVAL_IN_MICROSECONDS), \
      HID_ITEM_INT32    (FEATURE, 5, HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE), \
    HID_COLLECTION_END

struct __attribute__ ((packed)) LampArrayAttributesReport {
    uint16_t lamp_count;
    int32_t  bounding_box_width;
    int32_t  bounding_box_height;
    int32_t  bounding_box_depth;
    int32_t  lamp_kind;
    int32_t  min_update_interval;
};

// ---------------------------
// LampAttributesRequestReport
// ---------------------------

#define HID_REPORT_DESC_LAMP_ATTRIBUTES_REQUEST(REPORT_ID) \
    HID_REPORT_ID   (REPORT_ID) \
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_ATTRIBUTES_REQUEST_REPORT), \
    HID_COLLECTION  (HID_COLLECTION_LOGICAL), \
        /* LampId */ \
        HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ID), \
        HID_ITEM_UINT16   (FEATURE, 1, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
    HID_COLLECTION_END

struct __attribute__ ((packed)) LampAttributesRequestReport {
    uint16_t lamp_id;
};

// ----------------------------
// LampAttributesResponseReport
// ----------------------------
#define HID_REPORT_DESC_LAMP_ATTRIBUTES_RESPONSE(REPORT_ID) \
    HID_REPORT_ID   (REPORT_ID) \
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_ATTRIBUTES_RESPONSE_REPORT), \
    HID_COLLECTION  (HID_COLLECTION_LOGICAL), \
        /* LampId */ \
        HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ID), \
        HID_ITEM_UINT16   (FEATURE, 1, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        /* PositionXInMicrometers, PositionYInMicrometers, PositionZInMicrometers */ \
        /* UpdateLatencyInMicroseconds */ \
        /* LampPurposes */ \
        HID_USAGE         (HID_USAGE_LIGHTING_POSITION_X_IN_MICROMETERS), \
        HID_USAGE         (HID_USAGE_LIGHTING_POSITION_Y_IN_MICROMETERS), \
        HID_USAGE         (HID_USAGE_LIGHTING_POSITION_Z_IN_MICROMETERS), \
        HID_USAGE         (HID_USAGE_LIGHTING_UPDATE_LATENCY_IN_MICROSECONDS), \
        HID_USAGE         (HID_USAGE_LIGHTING_LAMP_PURPOSES), \
        HID_ITEM_INT32    (FEATURE, 5, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        /* RedLevelCount, GreenLevelCount, BlueLevelCount, IntensityLevelCount */ \
        /* IsProgrammable */ \
        /* InputBinding */ \
        HID_USAGE         (HID_USAGE_LIGHTING_RED_LEVEL_COUNT), \
        HID_USAGE         (HID_USAGE_LIGHTING_GREEN_LEVEL_COUNT), \
        HID_USAGE         (HID_USAGE_LIGHTING_BLUE_LEVEL_COUNT), \
        HID_USAGE         (HID_USAGE_LIGHTING_INTENSITY_LEVEL_COUNT), \
        HID_USAGE         (HID_USAGE_LIGHTING_IS_PROGRAMMABLE), \
        HID_USAGE         (HID_USAGE_LIGHTING_INPUT_BINDING), \
        HID_ITEM_UINT8    (FEATURE, 6, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
    HID_COLLECTION_END

struct __attribute__ ((packed)) LampAttributesResponseReport {
    uint16_t lamp_id;
    int32_t  position_x;
    int32_t  position_y;
    int32_t  position_z;
    int32_t  update_latency;
    int32_t  lamp_purpose;
    uint8_t  red_level_count;
    uint8_t  green_level_count;
    uint8_t  blue_level_count;
    uint8_t  intensity_level_count;
    uint8_t  is_programmable;
    uint8_t  input_binding;
};

// ---------------------
// LampMultiUpdateReport
// ---------------------

#define HID_REPORT_DESC_LAMP_MULTI_UPDATE_REPORT(REPORT_ID) \
    HID_REPORT_ID   (REPORT_ID) \
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_MULTI_UPDATE_REPORT), \
    HID_COLLECTION  (HID_COLLECTION_LOGICAL), \
        /* LampCount */ \
        HID_USAGE         (HID_USAGE_LIGHTING_LAMP_COUNT), \
        HID_LOGICAL_MIN   (0), \
        HID_LOGICAL_MAX   (LAMP_MULTI_UPDATE_BATCH_SIZE), \
        HID_REPORT_SIZE   (8), \
        HID_REPORT_COUNT  (1), \
        HID_FEATURE       (HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        /* LampUpdateFlags */ \
        HID_USAGE         (HID_USAGE_LIGHTING_LAMP_UPDATE_FLAGS), \
        HID_ITEM_UINT8    (FEATURE, 1, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        /* LampId Slots */ \
        HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ID), \
        HID_ITEM_UINT16   (FEATURE, LAMP_MULTI_UPDATE_BATCH_SIZE, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        /* (Red, Green, Blue, Intensity) Slots */ \
        HID_REPEAT(LAMP_MULTI_UPDATE_BATCH_SIZE, \
            HID_USAGE       (HID_USAGE_LIGHTING_RED_UPDATE_CHANNEL), \
            HID_USAGE       (HID_USAGE_LIGHTING_GREEN_UPDATE_CHANNEL), \
            HID_USAGE       (HID_USAGE_LIGHTING_BLUE_UPDATE_CHANNEL), \
            HID_USAGE       (HID_USAGE_LIGHTING_INTENSITY_UPDATE_CHANNEL), \
        ) \
        HID_ITEM_UINT8    (FEATURE, 4*LAMP_MULTI_UPDATE_BATCH_SIZE, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
    HID_COLLECTION_END

struct __attribute__ ((packed)) LampMultiUpdateReport {
    uint8_t  lamp_count;
    uint8_t  update_flags;
    uint16_t lamp_ids[LAMP_MULTI_UPDATE_BATCH_SIZE];
    uint8_t  rgbi_tuples[LAMP_MULTI_UPDATE_BATCH_SIZE][4];
};

// ---------------------
// LampRangeUpdateReport
// ---------------------
#define HID_REPORT_DESC_LAMP_RANGE_UPDATE_REPORT(REPORT_ID) \
    HID_REPORT_ID   (REPORT_ID) \
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_RANGE_UPDATE_REPORT), \
    HID_COLLECTION  (HID_COLLECTION_LOGICAL), \
        /* LampUpdateFlags */ \
        HID_USAGE         (HID_USAGE_LIGHTING_LAMP_UPDATE_FLAGS), \
        HID_ITEM_UINT8    (FEATURE, 1, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        /* LampIdStart, LampIdEnd */ \
        HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ID_START), \
        HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ID_END), \
        HID_ITEM_UINT16   (FEATURE, 2, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        /* Red, Green, Blue, Intensity */ \
        HID_USAGE         (HID_USAGE_LIGHTING_RED_UPDATE_CHANNEL), \
        HID_USAGE         (HID_USAGE_LIGHTING_GREEN_UPDATE_CHANNEL), \
        HID_USAGE         (HID_USAGE_LIGHTING_BLUE_UPDATE_CHANNEL), \
        HID_USAGE         (HID_USAGE_LIGHTING_INTENSITY_UPDATE_CHANNEL), \
        HID_ITEM_UINT8    (FEATURE, 4, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
    HID_COLLECTION_END

struct __attribute__ ((packed)) LampRangeUpdateReport {
    uint8_t  update_flags;
    uint16_t lamp_id_start;
    uint16_t lamp_id_end;
    uint8_t  rgbi_tuple[4];
};

// ----------------------
// LampArrayControlReport
// ----------------------

#define HID_REPORT_DESC_LAMP_ARRAY_CONTROL(REPORT_ID) \
    HID_REPORT_ID   (REPORT_ID) \
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_ARRAY_CONTROL_REPORT), \
    HID_COLLECTION  (HID_COLLECTION_LOGICAL), \
        /* AutonomousMode */ \
        HID_USAGE         (HID_USAGE_LIGHTING_AUTONOMOUS_MODE), \
        HID_LOGICAL_MIN   (0), \
        HID_LOGICAL_MAX   (1), \
        HID_REPORT_SIZE   (8), \
        HID_REPORT_COUNT  (1), \
        HID_FEATURE       (HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
    HID_COLLECTION_END

struct __attribute__ ((packed)) LampArrayControlReport {
    uint8_t autonomous_mode;
};

#endif /* HID_LIGHTS_REPORT_H_ */
