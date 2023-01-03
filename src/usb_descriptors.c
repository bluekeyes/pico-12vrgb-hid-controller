/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
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
 *
 */

#include <stdint.h>
#include <string.h>

#include "hardware/flash.h"
#include "tusb.h"

#include "config.h"
#include "usb_descriptors.h"

// TODO(bkeyes): set appropriate VID / PID

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

#define USB_VID   0xCafe
#define USB_BCD   0x0200

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
    .bcdDevice          = 0x0100, // TODO(bkeyes): device revision (in binary-coded decimal?)

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

// TODO(bkeyes): should logical min/max use the possible values from the field size or the actual hardware configuration?
// TODO(bkeyes): should fields be sized based on "natural" (i.e. byte) boundaries or by actual hardware configuration?
//               e.g. lamp counts / multi-update limited to actual channels

uint8_t const desc_hid_report[] = {
  HID_USAGE_PAGE    (HID_USAGE_PAGE_LIGHTING),
  HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ARRAY),
  HID_COLLECTION    (HID_COLLECTION_APPLICATION),

    // -------------------------
    // LampArrayAttributesReport
    // -------------------------
    HID_REPORT_ID   (HID_REPORT_ID_LAMP_ARRAY_ATTRIBUTES)
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_ARRAY_ATTRIBUTES_REPORT),
    HID_COLLECTION  (HID_COLLECTION_LOGICAL),
      // LampCount
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_COUNT),
      HID_LOGICAL_MIN   (0),
      HID_LOGICAL_MAX   (CFG_RGB_CHANNELS),
      HID_REPORT_SIZE   (8),
      HID_REPORT_COUNT  (1),
      HID_INPUT         (HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE),

      // BoundingBoxWidthInMicrometers, BoundingBoxHeightInMicrometers, BoundingBoxDepthInMicrometers
      // MinUpdateIntervalInMicroseconds
      HID_USAGE         (HID_USAGE_LIGHTING_BOUNDING_BOX_WIDTH_IN_MICROMETERS),
      HID_USAGE         (HID_USAGE_LIGHTING_BOUNDING_BOX_HEIGHT_IN_MICROMETERS),
      HID_USAGE         (HID_USAGE_LIGHTING_BOUNDING_BOX_DEPTH_IN_MICROMETERS),
      HID_USAGE         (HID_USAGE_LIGHTING_MIN_UPDATE_INTERVAL_IN_MICROSECONDS),
      HID_LOGICAL_MAX_N (INT32_MAX, 3),
      HID_REPORT_SIZE   (32),
      HID_REPORT_COUNT  (4),
      HID_INPUT         (HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE),

      // LampArrayKind
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ARRAY_KIND),
      HID_LOGICAL_MAX   (0xFF),
      HID_REPORT_SIZE   (8),
      HID_REPORT_COUNT  (1),
      HID_INPUT         (HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,

    // ---------------------------
    // LampAttributesRequestReport
    // ---------------------------
    HID_REPORT_ID   (HID_REPORT_ID_LAMP_ATTRIBUTES_REQUEST)
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_ATTRIBUTES_REQUEST_REPORT),
    HID_COLLECTION  (HID_COLLECTION_LOGICAL),
      // LampId
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ID),
      HID_LOGICAL_MAX   (CFG_RGB_CHANNELS - 1),
      HID_REPORT_SIZE   (8),
      HID_REPORT_COUNT  (1),
      HID_OUTPUT        (HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,

    // ----------------------------
    // LampAttributesResponseReport
    // ----------------------------
    HID_REPORT_ID   (HID_REPORT_ID_LAMP_ATTRIBUTES_RESPONSE)
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_ATTRIBUTES_RESPONSE_REPORT),
    HID_COLLECTION  (HID_COLLECTION_LOGICAL),
      // LampId
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ID),
      HID_LOGICAL_MAX   (CFG_RGB_CHANNELS - 1),
      HID_REPORT_SIZE   (8),
      HID_REPORT_COUNT  (1),
      HID_INPUT         (HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE),

      // PositionXInMicrometers, PositionYInMicrometers, PositionZInMicrometers
      HID_USAGE         (HID_USAGE_LIGHTING_POSITION_X_IN_MICROMETERS),
      HID_USAGE         (HID_USAGE_LIGHTING_POSITION_Y_IN_MICROMETERS),
      HID_USAGE         (HID_USAGE_LIGHTING_POSITION_Z_IN_MICROMETERS),
      HID_LOGICAL_MAX_N (INT32_MAX, 3),
      HID_REPORT_SIZE   (32),
      HID_REPORT_COUNT  (3),
      HID_INPUT         (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // LampPurposes
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_PURPOSES),
      HID_LOGICAL_MAX_N (0xFFFF, 2),
      HID_REPORT_SIZE   (16),
      HID_REPORT_COUNT  (1),
      HID_INPUT         (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // UpdateLatencyInMicroseconds
      HID_USAGE         (HID_USAGE_LIGHTING_UPDATE_LATENCY_IN_MICROSECONDS),
      HID_LOGICAL_MAX_N (INT32_MAX, 3),
      HID_REPORT_SIZE   (32),
      HID_REPORT_COUNT  (1),
      HID_INPUT         (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // RedLevelCount, GreenLevelCount, BlueLevelCount
      HID_USAGE         (HID_USAGE_LIGHTING_RED_LEVEL_COUNT),
      HID_USAGE         (HID_USAGE_LIGHTING_GREEN_LEVEL_COUNT),
      HID_USAGE         (HID_USAGE_LIGHTING_BLUE_LEVEL_COUNT),
      HID_LOGICAL_MAX   (CFG_RGB_LEVELS_PER_CHANNEL),
      HID_REPORT_SIZE   (CFG_RGB_BITS_PER_CHANNEL),
      HID_REPORT_COUNT  (3),
      HID_INPUT         (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // IntensityLevelCount
      HID_USAGE         (HID_USAGE_LIGHTING_INTENSITY_LEVEL_COUNT),
      HID_LOGICAL_MAX   (CFG_RGB_INTENSITY_LEVELS),
      HID_REPORT_SIZE   (8),
      HID_REPORT_COUNT  (1),
      HID_INPUT         (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // IsProgrammable
      HID_USAGE         (HID_USAGE_LIGHTING_IS_PROGRAMMABLE),
      HID_LOGICAL_MAX   (1),
      HID_REPORT_SIZE   (8),
      HID_REPORT_COUNT  (1),
      HID_INPUT         (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // InputBinding
      HID_USAGE         (HID_USAGE_LIGHTING_INPUT_BINDING),
      HID_LOGICAL_MAX_N (0xFFFF, 2),
      HID_REPORT_SIZE   (16),
      HID_REPORT_COUNT  (1),
      HID_INPUT         (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,

    // ---------------------
    // LampMultiUpdateReport
    // ---------------------
    HID_REPORT_ID   (HID_REPORT_ID_LAMP_MULTI_UPDATE)
    HID_USAGE       (HID_USAGE_LIGHTING_LAMP_MULTI_UPDATE_REPORT),
    HID_COLLECTION  (HID_COLLECTION_LOGICAL),
      // LampCount
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_COUNT),
      HID_LOGICAL_MAX   (4),
      HID_REPORT_SIZE   (8),
      HID_REPORT_COUNT  (1),
      HID_OUTPUT        (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // LampIds
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_ID),
      HID_LOGICAL_MAX   (CFG_RGB_CHANNELS - 1),
      HID_REPORT_SIZE   (8),
      HID_REPORT_COUNT  (4),
      HID_OUTPUT        (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // Lamp #1 (Red, Green, Blue, Intensity)
      HID_USAGE         (HID_USAGE_LIGHTING_RED_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_GREEN_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_BLUE_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_INTENSITY_UPDATE_CHANNEL),
      // Lamp #2 (Red, Green, Blue, Intensity)
      HID_USAGE         (HID_USAGE_LIGHTING_RED_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_GREEN_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_BLUE_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_INTENSITY_UPDATE_CHANNEL),
      // Lamp #3 (Red, Green, Blue, Intensity)
      HID_USAGE         (HID_USAGE_LIGHTING_RED_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_GREEN_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_BLUE_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_INTENSITY_UPDATE_CHANNEL),
      // Lamp #4 (Red, Green, Blue, Intensity)
      HID_USAGE         (HID_USAGE_LIGHTING_RED_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_GREEN_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_BLUE_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_INTENSITY_UPDATE_CHANNEL),
      HID_LOGICAL_MAX   (255),
      HID_REPORT_SIZE   (8),
      HID_REPORT_COUNT  (16),
      HID_OUTPUT        (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // LampUpdateFlags
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_UPDATE_FLAGS),
      HID_LOGICAL_MAX_N (0xFFFF, 2),
      HID_REPORT_SIZE   (16),
      HID_REPORT_COUNT  (1),
      HID_OUTPUT        (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
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
      HID_LOGICAL_MAX   (CFG_RGB_CHANNELS - 1),
      HID_REPORT_SIZE   (8),
      HID_REPORT_COUNT  (2),
      HID_OUTPUT        (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // Red, Green, Blue, Intensity
      HID_USAGE         (HID_USAGE_LIGHTING_RED_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_GREEN_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_BLUE_UPDATE_CHANNEL),
      HID_USAGE         (HID_USAGE_LIGHTING_INTENSITY_UPDATE_CHANNEL),
      HID_LOGICAL_MAX   (255),
      HID_REPORT_SIZE   (8),
      HID_REPORT_COUNT  (4),
      HID_OUTPUT        (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // LampUpdateFlags
      HID_USAGE         (HID_USAGE_LIGHTING_LAMP_UPDATE_FLAGS),
      HID_LOGICAL_MAX_N (0xFFFF, 2),
      HID_REPORT_SIZE   (16),
      HID_REPORT_COUNT  (1),
      HID_OUTPUT        (HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
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

      // Padding
      HID_REPORT_SIZE   (7),
      HID_OUTPUT        (HID_CONSTANT),
    HID_COLLECTION_END,

  HID_COLLECTION_END,
};

uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance)
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

uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

#define MAX_DESCRIPTOR_LENGTH   31
#define FLASH_ID_BYTES          8

// TODO(bkeyes): add init function, load serial once at startup

// static string descriptors
char const* string_desc_arr[] = {
  (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
  "BlueKeyes",                   // 1: Manufacturer
  "12VRGB HID Controller",       // 2: Product
  "",                            // 3: Serial Number (placeholder, dynamic)
};

// alphabet for converting chip ID to hex-encoded serial value
char const* hex_alphabet = "0123456789abcdef";

// static buffer for returning string descriptors
static uint16_t _desc_str[MAX_DESCRIPTOR_LENGTH + 1];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  uint8_t chr_count;
  const char *str;
  uint8_t id[FLASH_ID_BYTES];

  switch (index) {
    case 0:
      memcpy(&_desc_str[1], string_desc_arr[0], 2);
      chr_count = 1;
      break;

    case 1:
    case 2:
      str = string_desc_arr[index];
      chr_count = strlen(str);
      if (chr_count > MAX_DESCRIPTOR_LENGTH) {
        chr_count = MAX_DESCRIPTOR_LENGTH;
      }

      for (uint8_t i = 0; i < chr_count; i++) {
        _desc_str[1+i] = str[i];
      }
      break;

    case 3:
      flash_get_unique_id(id);

      for (uint8_t i = 0; i < FLASH_ID_BYTES; i++) {
        _desc_str[1+2*i] = hex_alphabet[id[i] & 0xF];
        _desc_str[1+2*i+1] = hex_alphabet[(id[i] >> 4) & 0xF];
      }
      chr_count = 2 * FLASH_ID_BYTES;
      break;

    default:
      return NULL;
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (TUSB_DESC_STRING << 8) | (2*chr_count + 2);

  return _desc_str;
}
