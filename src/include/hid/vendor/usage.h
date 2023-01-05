/**
 * This file contains constants for the 12VRGB controller's HID usage table.
 */

#ifndef HID_VENDOR_USAGE_H_
#define HID_VENDOR_USAGE_H_

#define HID_USAGE_PAGE_VENDOR_12VRGB    0xFF00

enum {
    HID_USAGE_VENDOR_12VRGB_CONTROLLER          = 0x01,
    HID_USAGE_VENDOR_12VRGB_BOOTSEL_REPORT      = 0x02,
    HID_USAGE_VENDOR_12VRGB_BOOTSEL_RESTART     = 0x03,
};

#endif // HID_VENDOR_USAGE_H_
