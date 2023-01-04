#ifndef CONFIG_H_
#define CONFIG_H_

#include "hid_lighting.h"

// The device version as 0xJJMN, where JJ is the major version, M is the minor
// version and N is the patch version.
#define CFG_RGB_DEVICE_VERSION 0x0010

// The number of individually addressable lamps / RGB channels
//
// Range: [1, 256]
#define CFG_RGB_LAMP_COUNT 1

// The number of lamps that can be updated in a single LampMultiUpdateReport
//
// Range: [1, 8]
#define CFG_RGB_MULTI_UPDATE_SIZE 1

// The dimensions of the bounding box that contains all lights
//
// Range: [0, 2^31-1]
// Units: Micrometers
#define CFG_RGB_BOUNDING_BOX_WIDTH  0
#define CFG_RGB_BOUNDING_BOX_HEIGHT 0
#define CFG_RGB_BOUNDING_BOX_DEPTH  0

// Lamp positions as (X, Y, Z) tuples relative to the bounding box origin. The
// number of entries must equal CFG_RGB_LAMP_COUNT.
//
// Units: Micrometers
#define CFG_RGB_LAMP_POSITIONS \
    0, 0, 0,

// Lamp purpose flags. The number of entries must equal CFG_RGB_LAMP_COUNT. The
// following flags are allowed, see Section 25.3.1 in the HID Usage Table for
// details:
//
//     LAMP_PURPOSE_CONTROL        0x01
//     LAMP_PURPOSE_ACCENT         0x02
//     LAMP_PURPOSE_BRANDING       0x04
//     LAMP_PURPOSE_STATUS         0x08
//     LAMP_PURPOSE_ILLUMINATION   0x10
//     LAMP_PURPOSE_PRESENTATION   0x20
//
#define CFG_RGB_LAMP_PURPOSES \
    LAMP_PURPOSE_ACCENT

// The minimum update interval.
//
// Range: [0, 2^31-1]
// Units: Microseconds
#define CFG_RGB_MINIMUM_UPDATE_INTERVAL 0

// The latency between requesting a lamp update and the change being visible.
// Assumed to be the constant for all lamps.
//
// Range: [0, 2^31-1]
// Units: Microseconds
#define CFG_RGB_LAMP_UPDATE_LATENCY 0

#endif
