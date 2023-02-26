#ifndef HID_LIGHTS_USAGE_H_
#define HID_LIGHTS_USAGE_H_

/**
 * This file contains constants from the HID "Lighting and Illumination" usage
 * table. See Section 25 of the HID Usage Tables 1.3 document for details.
 */

#define HID_USAGE_PAGE_LIGHTING 0x59

enum {
    HID_USAGE_LIGHTING_LAMP_ARRAY                           = 0x01,
    HID_USAGE_LIGHTING_LAMP_ARRAY_ATTRIBUTES_REPORT         = 0x02,
    HID_USAGE_LIGHTING_LAMP_COUNT                           = 0x03,
    HID_USAGE_LIGHTING_BOUNDING_BOX_WIDTH_IN_MICROMETERS    = 0x04,
    HID_USAGE_LIGHTING_BOUNDING_BOX_HEIGHT_IN_MICROMETERS   = 0x05,
    HID_USAGE_LIGHTING_BOUNDING_BOX_DEPTH_IN_MICROMETERS    = 0x06,
    HID_USAGE_LIGHTING_LAMP_ARRAY_KIND                      = 0x07,
    HID_USAGE_LIGHTING_MIN_UPDATE_INTERVAL_IN_MICROSECONDS  = 0x08,
    HID_USAGE_LIGHTING_LAMP_ATTRIBUTES_REQUEST_REPORT       = 0x20,
    HID_USAGE_LIGHTING_LAMP_ID                              = 0x21,
    HID_USAGE_LIGHTING_LAMP_ATTRIBUTES_RESPONSE_REPORT      = 0x22,
    HID_USAGE_LIGHTING_POSITION_X_IN_MICROMETERS            = 0x23,
    HID_USAGE_LIGHTING_POSITION_Y_IN_MICROMETERS            = 0x24,
    HID_USAGE_LIGHTING_POSITION_Z_IN_MICROMETERS            = 0x25,
    HID_USAGE_LIGHTING_LAMP_PURPOSES                        = 0x26,
    HID_USAGE_LIGHTING_UPDATE_LATENCY_IN_MICROSECONDS       = 0x27,
    HID_USAGE_LIGHTING_RED_LEVEL_COUNT                      = 0x28,
    HID_USAGE_LIGHTING_GREEN_LEVEL_COUNT                    = 0x29,
    HID_USAGE_LIGHTING_BLUE_LEVEL_COUNT                     = 0x2A,
    HID_USAGE_LIGHTING_INTENSITY_LEVEL_COUNT                = 0x2B,
    HID_USAGE_LIGHTING_IS_PROGRAMMABLE                      = 0x2C,
    HID_USAGE_LIGHTING_INPUT_BINDING                        = 0x2D,
    HID_USAGE_LIGHTING_LAMP_MULTI_UPDATE_REPORT             = 0x50,
    HID_USAGE_LIGHTING_RED_UPDATE_CHANNEL                   = 0x51,
    HID_USAGE_LIGHTING_GREEN_UPDATE_CHANNEL                 = 0x52,
    HID_USAGE_LIGHTING_BLUE_UPDATE_CHANNEL                  = 0x53,
    HID_USAGE_LIGHTING_INTENSITY_UPDATE_CHANNEL             = 0x54,
    HID_USAGE_LIGHTING_LAMP_UPDATE_FLAGS                    = 0x55,
    HID_USAGE_LIGHTING_LAMP_RANGE_UPDATE_REPORT             = 0x60,
    HID_USAGE_LIGHTING_LAMP_ID_START                        = 0x61,
    HID_USAGE_LIGHTING_LAMP_ID_END                          = 0x62,
    HID_USAGE_LIGHTING_LAMP_ARRAY_CONTROL_REPORT            = 0x70,
    HID_USAGE_LIGHTING_AUTONOMOUS_MODE                      = 0x71,
};

// -----------------
// LampPurpose Flags
// -----------------

#define LAMP_PURPOSE_CONTROL        0x01
#define LAMP_PURPOSE_ACCENT         0x02
#define LAMP_PURPOSE_BRANDING       0x04
#define LAMP_PURPOSE_STATUS         0x08
#define LAMP_PURPOSE_ILLUMINATION   0x10
#define LAMP_PURPOSE_PRESENTATION   0x20

// --------------------
// LampArrayKind Values
// --------------------

enum {
    LAMP_ARRAY_KIND_KEYBOARD         = 0x01,
    LAMP_ARRAY_KIND_MOUSE            = 0x02,
    LAMP_ARRAY_KIND_GAME_CONTROLLER  = 0x03,
    LAMP_ARRAY_KIND_PERIPHERAL       = 0x04,
    LAMP_ARRAY_KIND_SCENE            = 0x05,
    LAMP_ARRAY_KIND_NOTIFICATION     = 0x06,
    LAMP_ARRAY_KIND_CHASSIS          = 0x07,
    LAMP_ARRAY_KIND_WEARABLE         = 0x08,
    LAMP_ARRAY_KIND_FURNITURE        = 0x09,
    LAMP_ARRAY_KIND_ART              = 0x0A,
};

// ----------------
// LampUpdate Flags
// ----------------

#define LAMP_UPDATE_COMPLETE        0x01

#endif /* HID_LIGHTS_USAGE_H_ */
