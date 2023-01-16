/**
 * This file contains constants for the 12VRGB controller's HID usage table.
 */

#ifndef HID_VENDOR_USAGE_H_
#define HID_VENDOR_USAGE_H_

#define HID_USAGE_PAGE_VENDOR_12VRGB    0xFF00

enum {
    // TODO(bkeyes): change bootsel to generic reset report with flags
    //   bootsel, clear flash
    HID_USAGE_VENDOR_12VRGB_CONTROLLER                  = 0x01,
    HID_USAGE_VENDOR_12VRGB_BOOTSEL_REPORT              = 0x02,
    HID_USAGE_VENDOR_12VRGB_BOOTSEL_RESTART             = 0x03,
    HID_USAGE_VENDOR_12VRGB_ANIMATION_REPORT            = 0x10,
    HID_USAGE_VENDOR_12VRGB_LAMP_ID                     = 0x11,
    HID_USAGE_VENDOR_12VRGB_ANIMATION_TYPE              = 0x12,
    HID_USAGE_VENDOR_12VRGB_ANIMATION_PARAMETER         = 0x13,
    HID_USAGE_VENDOR_12VRGB_ANIMATION_COLOR_RED         = 0x14,
    HID_USAGE_VENDOR_12VRGB_ANIMATION_COLOR_GREEN       = 0x15,
    HID_USAGE_VENDOR_12VRGB_ANIMATION_COLOR_BLUE        = 0x16,
    HID_USAGE_VENDOR_12VRGB_DEFAULT_ANIMATION_REPORT    = 0x20,
};

enum {
    ANIMATION_TYPE_NONE     = 0x00,
    ANIMATION_TYPE_BREATHE  = 0x01,
    ANIMATION_TYPE_FADE     = 0x02,
};

#endif // HID_VENDOR_USAGE_H_
