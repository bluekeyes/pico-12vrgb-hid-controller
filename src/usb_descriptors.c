#include <stdint.h>
#include <string.h>

#include "hardware/flash.h"
#include "tusb.h"

#include "config.h"
#include "hid_lighting.h"
#include "usb_descriptors.h"

// TODO(bkeyes): set appropriate VID / PID
#define USB_VID   0xCafe
#define USB_PID   0x4100
#define USB_BCD   0x0200 // USB 2.0

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

    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
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
  HID_USAGE_PAGE    (HID_USAGE_PAGE_LIGHTING),
  HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ARRAY),
  HID_COLLECTION    (HID_COLLECTION_APPLICATION),

    // All fields in reports are non-negative
    HID_LOGICAL_MIN (0),

    // -------------------------
    // LampArrayAttributesReport
    // -------------------------
    HID_REPORT_ID   (HID_REPORT_ID_LAMP_ARRAY_ATTRIBUTES)
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_ARRAY_ATTRIBUTES_REPORT),
    HID_COLLECTION  (HID_COLLECTION_LOGICAL),
      // LampCount
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_COUNT),
      HID_ITEM_UINT16   (INPUT, 1, HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE),

      // BoundingBoxWidthInMicrometers, BoundingBoxHeightInMicrometers, BoundingBoxDepthInMicrometers
      // MinUpdateIntervalInMicroseconds
      HID_USAGE         (HID_USAGE_LIGHTING_BOUNDING_BOX_WIDTH_IN_MICROMETERS),
      HID_USAGE         (HID_USAGE_LIGHTING_BOUNDING_BOX_HEIGHT_IN_MICROMETERS),
      HID_USAGE         (HID_USAGE_LIGHTING_BOUNDING_BOX_DEPTH_IN_MICROMETERS),
      HID_USAGE         (HID_USAGE_LIGHTING_MIN_UPDATE_INTERVAL_IN_MICROSECONDS),
      HID_ITEM_INT32    (INPUT, 4, HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE),

      // LampArrayKind
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ARRAY_KIND),
      HID_ITEM_UINT8    (INPUT, 1, HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,

    // ---------------------------
    // LampAttributesRequestReport
    // ---------------------------
    HID_REPORT_ID   (HID_REPORT_ID_LAMP_ATTRIBUTES_REQUEST)
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_ATTRIBUTES_REQUEST_REPORT),
    HID_COLLECTION  (HID_COLLECTION_LOGICAL),
      // LampId
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ID),
      HID_ITEM_UINT8    (OUTPUT, 1, HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,

    // ----------------------------
    // LampAttributesResponseReport
    // ----------------------------
    HID_REPORT_ID   (HID_REPORT_ID_LAMP_ATTRIBUTES_RESPONSE)
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_ATTRIBUTES_RESPONSE_REPORT),
    HID_COLLECTION  (HID_COLLECTION_LOGICAL),
      // LampId
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ID),
      HID_ITEM_UINT8    (INPUT, 1, HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE),

      // PositionXInMicrometers, PositionYInMicrometers, PositionZInMicrometers
      HID_USAGE         (HID_USAGE_LIGHTING_POSITION_X_IN_MICROMETERS),
      HID_USAGE         (HID_USAGE_LIGHTING_POSITION_Y_IN_MICROMETERS),
      HID_USAGE         (HID_USAGE_LIGHTING_POSITION_Z_IN_MICROMETERS),
      HID_ITEM_INT32    (INPUT, 3, HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // LampPurposes
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_PURPOSES),
      HID_ITEM_UINT16   (INPUT, 1, HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // UpdateLatencyInMicroseconds
      HID_USAGE         (HID_USAGE_LIGHTING_UPDATE_LATENCY_IN_MICROSECONDS),
      HID_ITEM_INT32    (INPUT, 1, HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // RedLevelCount, GreenLevelCount, BlueLevelCount, IntensityLevelCount
      HID_USAGE         (HID_USAGE_LIGHTING_RED_LEVEL_COUNT),
      HID_USAGE         (HID_USAGE_LIGHTING_GREEN_LEVEL_COUNT),
      HID_USAGE         (HID_USAGE_LIGHTING_BLUE_LEVEL_COUNT),
      HID_USAGE         (HID_USAGE_LIGHTING_INTENSITY_LEVEL_COUNT),
      HID_ITEM_UINT8    (INPUT, 4, HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // IsProgrammable
      HID_USAGE         (HID_USAGE_LIGHTING_IS_PROGRAMMABLE),
      HID_LOGICAL_MAX   (1),
      HID_REPORT_SIZE   (1),
      HID_REPORT_COUNT  (1),
      HID_INPUT         (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // Padding
      HID_REPORT_SIZE   (7),
      HID_INPUT         (HID_CONSTANT),

      // InputBinding
      HID_USAGE         (HID_USAGE_LIGHTING_INPUT_BINDING),
      HID_ITEM_UINT16   (INPUT, 1, HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,

    // ---------------------
    // LampMultiUpdateReport
    // ---------------------
    HID_REPORT_ID   (HID_REPORT_ID_LAMP_MULTI_UPDATE)
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_MULTI_UPDATE_REPORT),
    HID_COLLECTION  (HID_COLLECTION_LOGICAL),
      // LampCount
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_COUNT),
      HID_LOGICAL_MAX   (CFG_RGB_MULTI_UPDATE_SIZE),
      HID_REPORT_SIZE   (8),
      HID_REPORT_COUNT  (1),
      HID_OUTPUT        (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // LampId Slots
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ID),
      HID_ITEM_UINT8    (OUTPUT, CFG_RGB_MULTI_UPDATE_SIZE, HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // (Red, Green, Blue, Intensity) Slots
      HID_REPEAT(CFG_RGB_MULTI_UPDATE_SIZE,
        HID_USAGE       (HID_USAGE_LIGHTING_RED_UPDATE_CHANNEL),
        HID_USAGE       (HID_USAGE_LIGHTING_GREEN_UPDATE_CHANNEL),
        HID_USAGE       (HID_USAGE_LIGHTING_BLUE_UPDATE_CHANNEL),
        HID_USAGE       (HID_USAGE_LIGHTING_INTENSITY_UPDATE_CHANNEL),
      )
      HID_ITEM_UINT8    (OUTPUT, 4*CFG_RGB_MULTI_UPDATE_SIZE, HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // LampUpdateFlags
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_UPDATE_FLAGS),
      HID_ITEM_UINT16   (OUTPUT, 1, HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,

    // ---------------------
    // LampRangeUpdateReport
    // ---------------------
    HID_REPORT_ID   (HID_REPORT_ID_LAMP_RANGE_UPDATE)
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_RANGE_UPDATE_REPORT),
    HID_COLLECTION  (HID_COLLECTION_LOGICAL),
      // LampIdStart, LampIdEnd
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ID_START),
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ID_END),
      HID_ITEM_UINT8    (OUTPUT, 2, HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // Red, Green, Blue, Intensity
      HID_USAGE         (HID_USAGE_LIGHTING_RED_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_GREEN_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_BLUE_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_INTENSITY_UPDATE_CHANNEL),
      HID_ITEM_UINT8    (OUTPUT, 4, HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // LampUpdateFlags
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_UPDATE_FLAGS),
      HID_ITEM_UINT16   (OUTPUT, 1, HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,

    // ----------------------
    // LampArrayControlReport
    // ----------------------
    HID_REPORT_ID   (HID_REPORT_ID_LAMP_ARRAY_CONTROL)
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_ARRAY_CONTROL_REPORT),
    HID_COLLECTION  (HID_COLLECTION_LOGICAL),
      // AutonomousMode
      HID_USAGE         (HID_USAGE_LIGHTING_AUTONOMOUS_MODE),
      HID_LOGICAL_MAX   (1),
      HID_REPORT_SIZE   (1),
      HID_REPORT_COUNT  (1),
      HID_OUTPUT        (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // Padding
      HID_REPORT_SIZE   (7),
      HID_OUTPUT        (HID_CONSTANT),
    HID_COLLECTION_END,

  HID_COLLECTION_END, // Lighting and Illumination: LampArray

  // -------------------------
  // Vendor 12VRGB: Controller
  // -------------------------
  HID_USAGE_PAGE_N  (HID_USAGE_PAGE_VENDOR_12VRGB, 2),
  HID_USAGE         (HID_USAGE_VENDOR_12VRGB_CONTROLLER),
  HID_COLLECTION    (HID_COLLECTION_APPLICATION),

    // -------------
    // BootSelReport
    // -------------
    HID_REPORT_ID   (HID_REPORT_ID_VENDOR_12VRGB_BOOTSEL)
    HID_USAGE       (HID_USAGE_VENDOR_12VRGB_BOOTSEL_REPORT),
    HID_COLLECTION  (HID_COLLECTION_LOGICAL),
      // BootSelRestart
      HID_USAGE         (HID_USAGE_VENDOR_12VRGB_BOOTSEL_RESTART),
      HID_LOGICAL_MAX   (1),
      HID_REPORT_SIZE   (1),
      HID_REPORT_COUNT  (1),
      HID_OUTPUT        (HID_DATA | HID_VARIABLE | HID_RELATIVE | HID_PREFERRED_STATE),

      // Padding
      HID_REPORT_SIZE   (7),
      HID_OUTPUT        (HID_CONSTANT),
    HID_COLLECTION_END,

  HID_COLLECTION_END, // Vendor 12VRGB: Controller
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

#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

#define EPNUM_HID   0x81

uint8_t const desc_configuration[] =
{
  // Config number, interface count, string index, total length, attribute, power in mA
  // TODO(bkeyes): check attributes and power
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

  // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
  // TODO(bkeyes): check if endpoint options (EPNUM_HID, bufsize, polling interval)
  TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5)
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
  return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

#define MAX_DESCRIPTOR_LENGTH   31
#define FLASH_ID_BYTES          8

// static string descriptors
static char const *string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 },  // 0: English (0x0409)
    "BlueKeyes",                    // 1: Manufacturer
    "12VRGB HID Controller",        // 2: Product
    "",                             // 3: Serial Number (placeholder)
};

static inline char hex_char(uint8_t v)
{
    return v < 0xA ? '0' + v : 'A' - 0xA + v;
}

static uint8_t get_serial_number_string(uint16_t *str)
{
    uint8_t id[FLASH_ID_BYTES];
    flash_get_unique_id(id);

    for (uint8_t i = 0; i < FLASH_ID_BYTES; i++) {
        str[2*i] = hex_char(id[i] & 0x0F);
        str[2*i+1] = hex_char((id[i] >> 4) & 0x0F);
    }

    return 2 * FLASH_ID_BYTES;
}

// buffer for returning string descriptors
static uint16_t _desc_str[MAX_DESCRIPTOR_LENGTH + 1];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    uint8_t chr_count;
    const char *str;

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
