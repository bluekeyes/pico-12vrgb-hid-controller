/**
 * This file defines structs for the 12VRGB controller's vendor HID reports. It
 * also defines macros for including these reports in a report descriptor.
 *
 * Note that the report structs must exactly match the data layout given by the
 * report descriptors, including that all multi-byte values are little-endian.
 * This works out by default on the RP2040, but could be a problem elsewhere.
 */

#ifndef HID_VENDOR_REPORT_H_
#define HID_VENDOR_REPORT_H_

#include <stdint.h>

#include "tusb.h"

#include "hid/descriptor.h"
#include "hid/vendor/usage.h"

#define HID_COLLECTION_VENDOR_12VRGB \
    HID_USAGE_PAGE_N  (HID_USAGE_PAGE_VENDOR_12VRGB, 2), \
    HID_USAGE         (HID_USAGE_VENDOR_12VRGB_CONTROLLER), \
    HID_COLLECTION    (HID_COLLECTION_APPLICATION), \
        /* All fields in reports are non-negative */ \
        HID_LOGICAL_MIN (0)

// -------------
// BootSelReport
// -------------
#define HID_REPORT_DESC_VENDOR_12VRGB_BOOTSEL(REPORT_ID) \
    HID_REPORT_ID   (REPORT_ID) \
    HID_USAGE       (HID_USAGE_VENDOR_12VRGB_BOOTSEL_REPORT), \
    HID_COLLECTION  (HID_COLLECTION_LOGICAL), \
        /* BootSelRestart */ \
        HID_USAGE         (HID_USAGE_VENDOR_12VRGB_BOOTSEL_RESTART), \
        HID_LOGICAL_MAX   (1), \
        HID_REPORT_SIZE   (1), \
        HID_REPORT_COUNT  (1), \
        HID_OUTPUT        (HID_DATA | HID_VARIABLE | HID_RELATIVE | HID_PREFERRED_STATE), \
        /* Padding */ \
        HID_REPORT_SIZE   (7), \
        HID_OUTPUT        (HID_CONSTANT), \
    HID_COLLECTION_END

struct __attribute__ ((packed)) Vendor12VRGBBootSelReport {
    uint8_t bootsel_restart;
};

// ------------------------
// (Default)AnimationReport
// ------------------------

#define ANIMATION_REPORT_MAX_PARAMS 4
#define ANIMATION_REPORT_MAX_COLORS 8

#define HID_REPORT_DESC_VENDOR_12VRGB_ANIMATION_(REPORT_ID, REPORT_TYPE, REPORT_USAGE) \
    HID_REPORT_ID   (REPORT_ID) \
    HID_USAGE       (REPORT_USAGE), \
    HID_COLLECTION  (HID_COLLECTION_LOGICAL), \
        /* Lamp ID */ \
        HID_USAGE       (HID_USAGE_VENDOR_12VRGB_LAMP_ID), \
        HID_ITEM_UINT8  (REPORT_TYPE, 1, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        /* Animation Type */ \
        HID_USAGE       (HID_USAGE_VENDOR_12VRGB_ANIMATION_TYPE), \
        HID_ITEM_UINT8  (REPORT_TYPE, 1, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        /* Parameters */ \
        HID_REPEAT(ANIMATION_REPORT_MAX_PARAMS, \
            HID_USAGE   (HID_USAGE_VENDOR_12VRGB_ANIMATION_PARAMETER), \
        ) \
        HID_ITEM_INT32  (REPORT_TYPE, ANIMATION_REPORT_MAX_PARAMS, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        /* Colors */ \
        HID_REPEAT(ANIMATION_REPORT_MAX_COLORS, \
            HID_USAGE   (HID_USAGE_VENDOR_12VRGB_ANIMATION_COLOR_RED), \
            HID_USAGE   (HID_USAGE_VENDOR_12VRGB_ANIMATION_COLOR_GREEN), \
            HID_USAGE   (HID_USAGE_VENDOR_12VRGB_ANIMATION_COLOR_BLUE), \
        ) \
        HID_ITEM_UINT8  (REPORT_TYPE, 3*ANIMATION_REPORT_MAX_COLORS, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
    HID_COLLECTION_END

#define HID_REPORT_DESC_VENDOR_12VRGB_ANIMATION(REPORT_ID) \
    HID_REPORT_DESC_VENDOR_12VRGB_ANIMATION_(REPORT_ID, OUTPUT, HID_USAGE_VENDOR_12VRGB_ANIMATION_REPORT)

#define HID_REPORT_DESC_VENDOR_12VRGB_DEFAULT_ANIMATION(REPORT_ID) \
    HID_REPORT_DESC_VENDOR_12VRGB_ANIMATION_(REPORT_ID, FEATURE, HID_USAGE_VENDOR_12VRGB_DEFAULT_ANIMATION_REPORT)

struct __attribute__ ((packed)) Vendor12VRGBAnimationReport {
    uint8_t lamp_id;
    uint8_t type;
    int32_t parameters[ANIMATION_REPORT_MAX_PARAMS];
    uint8_t colors[ANIMATION_REPORT_MAX_COLORS][3];
};

#endif /* HID_VENDOR_REPORT_H_ */
