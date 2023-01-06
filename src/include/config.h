#ifndef CONFIG_H_
#define CONFIG_H_

#include "hid/lights/usage.h"

// The device version as 0xJJMN, where JJ is the major version, M is the minor
// version and N is the patch version.
#define CFG_RGB_DEVICE_VERSION 0x0010

// The number of individually addressable lamps / RGB channels
//
// Range: [1, 256]
#define CFG_RGB_LAMP_COUNT 2

// The number of lamps that can be updated in a single LampMultiUpdateReport
//
// Range: [1, 8]
#define CFG_RGB_MULTI_UPDATE_SIZE 2

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
    {0, 0, 0}, \
    {0, 0, 0},

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
    LAMP_PURPOSE_ACCENT, \
    LAMP_PURPOSE_ACCENT,

// Map lamp RGB channels to GPIOs as (R, G, B) tuples. The number of entries
// must equal CFG_RGB_LAMP_COUNT. Each GPIO in the mapping must connect to a
// unique PWM channel. For example, you may not use both GPIO 0 and GPIO 16
// because they are both connected to PWM channel 0A.
//
// The PCB routes these GPIOs so they appear on headers in G-R-B order. You can
// change this in software by swapping the positions for a given lamp.  Other
// changes to this options will require hardware changes.
#define CFG_RGB_LAMP_GPIO_MAPPING \
    {3, 2, 4}, \
    {6, 5, 7},

// Set the divider for the PWM clock. The PWM counters wrap at the full 16-bit
// value, so with a 125 MHz system clock, the default value gives a PWM
// frequency of ~250 Hz.
#define CFG_RGB_PWM_CLOCK_DIVIDER 7.625f

// The minimum update interval.
//
// Range: [0, 2^31-1]
// Units: Microseconds
#define CFG_RGB_MINIMUM_UPDATE_INTERVAL 4000

// The latency between requesting a lamp update and the change being visible.
// Assumed to be the constant for all lamps. This is primarly determined by the
// PWM frequency: an update at the start of a cycle will not be visible until
// the start of the next cycle.
//
// Range: [0, 2^31-1]
// Units: Microseconds
#define CFG_RGB_LAMP_UPDATE_LATENCY 4000

#endif
