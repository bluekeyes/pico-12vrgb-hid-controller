#include <stdint.h>
#include <string.h>

#include "pico/unique_id.h"
#include "tusb.h"

#include "device/specs.h"
#include "hid/descriptor.h"
#include "hid/lights/report.h"
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

    // -------------------------
    // Vendor 12VRGB: Controller
    // -------------------------
    HID_COLLECTION_VENDOR_12VRGB,
        HID_REPORT_DESC_VENDOR_12VRGB_RESET             (HID_REPORT_ID_VENDOR_12VRGB_RESET),
        HID_REPORT_DESC_VENDOR_12VRGB_ANIMATION         (HID_REPORT_ID_VENDOR_12VRGB_ANIMATION), 
        HID_REPORT_DESC_VENDOR_12VRGB_DEFAULT_ANIMATION (HID_REPORT_ID_VENDOR_12VRGB_DEFAULT_ANIMATION), 
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

// static string descriptors
static char const *string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 },  // 0: English (0x0409)
    "BlueKeyes",                    // 1: Manufacturer
    "12VRGB HID Controller",        // 2: Product
    "",                             // 3: Serial Number (placeholder)
};

static inline char hex_char(uint8_t v)
{
    return v < 0xA ? '0' + v : 'A' + v - 0xA;
}

static uint8_t get_serial_number_string(uint16_t *str)
{
    pico_unique_board_id_t id;
    pico_get_unique_board_id(&id);

    for (uint8_t i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; i++) {
        str[2*i] = hex_char(id.id[i] & 0x0F);
        str[2*i+1] = hex_char((id.id[i] >> 4) & 0x0F);
    }

    return 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES;
}

// buffer for returning string descriptors
static uint16_t _desc_str[MAX_DESCRIPTOR_LENGTH + 1];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    uint8_t chr_count = 0;
    const char *str = NULL;

    if (index >= sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) {
        return NULL;
    }
    switch (index) {
        case 0:
            memcpy(&_desc_str[1], string_desc_arr[0], 2);
            chr_count = 1;
            break;

        case 3:
            chr_count = get_serial_number_string(&_desc_str[1]);
            break;

        default:
            str = string_desc_arr[index];

            chr_count = strlen(str);
            if (chr_count > MAX_DESCRIPTOR_LENGTH) {
                chr_count = MAX_DESCRIPTOR_LENGTH;
            }

            for (uint8_t i = 0; i < chr_count; i++) {
                _desc_str[1+i] = str[i];
            }
            break;
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2*chr_count + 2);

    return _desc_str;
}
