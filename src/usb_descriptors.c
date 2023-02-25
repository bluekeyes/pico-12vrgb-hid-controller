#include <stdint.h>
#include <string.h>

#include "pico/unique_id.h"
#include "tusb.h"

#include "device/specs.h"
#include "hid/descriptor.h"
#include "hid/lights/report.h"
#include "hid/sensor/report.h"
#include "hid/vendor/report.h"

#define USB_BCD 0x0200 // USB 2.0

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = CFG_RGB_USB_VID,
    .idProduct          = CFG_RGB_USB_PID,
    .bcdDevice          = CFG_RGB_DEVICE_VERSION,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

uint8_t const desc_hid_report[] = {
    // ------------------------------------
    // Lighting and Illumination: LampArray
    // ------------------------------------
    HID_COLLECTION_LAMP_ARRAY,
        HID_REPORT_DESC_LAMP_ARRAY_ATTRIBUTES       (HID_REPORT_ID_LAMP_ARRAY_ATTRIBUTES),
        HID_REPORT_DESC_LAMP_ATTRIBUTES_REQUEST     (HID_REPORT_ID_LAMP_ATTRIBUTES_REQUEST),
        HID_REPORT_DESC_LAMP_ATTRIBUTES_RESPONSE    (HID_REPORT_ID_LAMP_ATTRIBUTES_RESPONSE),
        HID_REPORT_DESC_LAMP_MULTI_UPDATE_REPORT    (HID_REPORT_ID_LAMP_MULTI_UPDATE),
        HID_REPORT_DESC_LAMP_RANGE_UPDATE_REPORT    (HID_REPORT_ID_LAMP_RANGE_UPDATE),
        HID_REPORT_DESC_LAMP_ARRAY_CONTROL          (HID_REPORT_ID_LAMP_ARRAY_CONTROL),
    HID_COLLECTION_END,

    // --------------------
    // Sensors: Temperature
    // --------------------
    HID_COLLECTION_SENSOR,
        HID_REPORT_DESC_ENVIRONMENTAL_TEMPERATURE   (HID_REPORT_ID_TEMPERATURE),
    HID_COLLECTION_END,

    // -------------------------
    // Vendor 12VRGB: Controller
    // -------------------------
    HID_COLLECTION_VENDOR_12VRGB,
        HID_REPORT_DESC_VENDOR_12VRGB_RESET         (HID_REPORT_ID_VENDOR_12VRGB_RESET),
        HID_REPORT_DESC_VENDOR_12VRGB_ANIMATION     (HID_REPORT_ID_VENDOR_12VRGB_ANIMATION),
    HID_COLLECTION_END,
};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum {
    ITF_NUM_HID,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN)

#define HID_EP_NUM          0x01
#define HID_EP_DIR_OUT      0x00
#define HID_EP_DIR_IN       0x80

uint8_t const desc_configuration[] =
{
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0, DEVICE_USB_POWER),

    // Interface number, string index, protocol, report descriptor len, out addr, in addr, bufsize, poll interval
    TUD_HID_INOUT_DESCRIPTOR(
        ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report),
        HID_EP_DIR_OUT | HID_EP_NUM, HID_EP_DIR_IN | HID_EP_NUM,
        CFG_TUD_HID_EP_BUFSIZE, DEVICE_USB_POLL_FRAMES
    )
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

#define MAX_DESCRIPTOR_LENGTH 31

static char serial_num_str[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

static char const *string_desc_arr[] = {
    [1] = "BlueKeyes",                    // Manufacturer
    [2] = "12VRGB HID Controller",        // Product
    [3] = serial_num_str,                 // Serial Number
};

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    /*
     * This function is based on the version in pico-sdk (pico_stdio_usb),
     * which was originally based on a file originally part of the MicroPython
     * project, http://micropython.org/
     *
     * The MIT License (MIT)
     *
     * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
     * Copyright (c) 2019 Damien P. George
     *
     * Permission is hereby granted, free of charge, to any person obtaining a copy
     * of this software and associated documentation files (the "Software"), to deal
     * in the Software without restriction, including without limitation the rights
     * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
     * copies of the Software, and to permit persons to whom the Software is
     * furnished to do so, subject to the following conditions:
     *
     * The above copyright notice and this permission notice shall be included in
     * all copies or substantial portions of the Software.
     *
     * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
     * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
     * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
     * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
     * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
     * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
     * THE SOFTWARE.
     */

    // reusable buffer for returning strings
    static uint16_t desc_str[MAX_DESCRIPTOR_LENGTH + 1];

    if (!serial_num_str[0]) {
        // load serial number if we haven't yet
        pico_get_unique_board_id_string(serial_num_str, sizeof(serial_num_str));
    }
    if (index >= sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) {
        return NULL;
    }

    uint8_t len = 0;
    if (index == 0) {
        // first descriptor is a special language code (english)
        desc_str[1] = 0x0409;
        len = 1;
    } else {
        const char *str = string_desc_arr[index];
        for (len = 0; len < MAX_DESCRIPTOR_LENGTH && str[len]; len++) {
            desc_str[1+len] = str[len];
        }
    }

    // first byte is length (including header), second byte is string type
    desc_str[0] = (TUSB_DESC_STRING << 8) | (2*len + 2);

    return desc_str;
}
