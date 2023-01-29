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
        HID_LOGICAL_MIN     (0), \
        HID_UNIT_EXPONENT   (0)

// -----------
// ResetReport
// -----------

#define HID_REPORT_DESC_VENDOR_12VRGB_RESET(REPORT_ID) \
    HID_REPORT_ID   (REPORT_ID) \
    HID_USAGE       (HID_USAGE_VENDOR_12VRGB_RESET_REPORT), \
    HID_COLLECTION  (HID_COLLECTION_LOGICAL), \
        /* ResetFlags */ \
        HID_USAGE         (HID_USAGE_VENDOR_12VRGB_RESET_FLAGS), \
        HID_ITEM_UINT8    (FEATURE, 1, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
    HID_COLLECTION_END

struct __attribute__ ((packed)) Vendor12VRGBResetReport {
    uint8_t flags;
};

// ------------------------
// (Default)AnimationReport
// ------------------------

/**
 * The size of the opaque animation report data field. The total report must be
 * no more than 63 bytes so it fits in the 64-byte USB limit when combined with
 * the report ID byte.
 */
#define ANIMATION_REPORT_DATA_SIZE 60

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
        /* Data */ \
        HID_USAGE       (HID_USAGE_VENDOR_12VRGB_ANIMATION_DATA), \
        HID_ITEM_UINT8  (REPORT_TYPE, ANIMATION_REPORT_DATA_SIZE, HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
    HID_COLLECTION_END

#define HID_REPORT_DESC_VENDOR_12VRGB_ANIMATION(REPORT_ID) \
    HID_REPORT_DESC_VENDOR_12VRGB_ANIMATION_(REPORT_ID, OUTPUT, HID_USAGE_VENDOR_12VRGB_ANIMATION_REPORT)

#define HID_REPORT_DESC_VENDOR_12VRGB_DEFAULT_ANIMATION(REPORT_ID) \
    HID_REPORT_DESC_VENDOR_12VRGB_ANIMATION_(REPORT_ID, FEATURE, HID_USAGE_VENDOR_12VRGB_DEFAULT_ANIMATION_REPORT)

struct __attribute__ ((packed)) Vendor12VRGBAnimationReport {
    uint8_t lamp_id;
    uint8_t type;
    uint8_t data[ANIMATION_REPORT_DATA_SIZE];
};

#endif /* HID_VENDOR_REPORT_H_ */
