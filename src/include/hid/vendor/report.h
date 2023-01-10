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

#endif /* HID_VENDOR_REPORT_H_ */
