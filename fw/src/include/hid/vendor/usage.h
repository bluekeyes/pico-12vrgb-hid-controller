/**
 * This file contains constants for the 12VRGB controller's HID usage table.
 */

#ifndef HID_VENDOR_USAGE_H_
#define HID_VENDOR_USAGE_H_

#define HID_USAGE_PAGE_VENDOR_12VRGB    0xFF00

enum {
    HID_USAGE_VENDOR_12VRGB_CONTROLLER                  = 0x01,
    HID_USAGE_VENDOR_12VRGB_RESET_REPORT                = 0x02,
    HID_USAGE_VENDOR_12VRGB_RESET_FLAGS                 = 0x03,

    HID_USAGE_VENDOR_12VRGB_ANIMATION_REPORT            = 0x10,
    HID_USAGE_VENDOR_12VRGB_LAMP_ID                     = 0x11,
    HID_USAGE_VENDOR_12VRGB_ANIMATION_TYPE              = 0x12,
    HID_USAGE_VENDOR_12VRGB_ANIMATION_DATA              = 0x13,
};

enum {
    VENDOR_RESET_FLAG_BOOTSEL       = 0x01,
    VENDOR_RESET_FLAG_CLEAR_FLASH   = 0x02,
};

enum {
    ANIMATION_TYPE_NONE     = 0x00,
    ANIMATION_TYPE_BREATHE  = 0x01,
    ANIMATION_TYPE_FADE     = 0x02,
};

#endif // HID_VENDOR_USAGE_H_
