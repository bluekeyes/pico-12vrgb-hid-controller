#ifndef CONFIG_H_
#define CONFIG_H_

#include "hid/lights/usage.h"

// The device version as 0xJJMN, where JJ is the major version, M is the minor
// version and N is the patch version.
#define CFG_RGB_DEVICE_VERSION 0x0010

// The USB vendor and product IDs for the device.
//
// TODO(bkeyes): these values are not "legal", but are the values used in
// tinyUSB examples for HID devices. These should be changed, possibly to one
// from pid.codes or to a Raspberry Pi one if they issue a generic HID code.
// For now, squatting on an ID used in a lot of example code seemed better than
// squatting on a random ID.
#define CFG_RGB_USB_VID   0xCafe
#define CFG_RGB_USB_PID   0x4100

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
// number of entries must equal LAMP_COUNT (device/specs.h).
//
// Units: Micrometers
#define CFG_RGB_LAMP_POSITIONS \
    {0, 0, 0}, \
    {0, 0, 0}, \
    {0, 0, 0}, \
    {0, 0, 0},

// Lamp purpose flags. The number of entries must equal LAMP_COUNT
// (device/specs.h). The following flags are allowed, see Section 25.3.1 in the
// HID Usage Table for details:
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
    LAMP_PURPOSE_ACCENT, \
    LAMP_PURPOSE_ACCENT, \
    LAMP_PURPOSE_ACCENT,

// Map lamp RGB channels to GPIOs as (R, G, B) tuples. The number of entries
// must equal LAMP_COUNT (device/specs.h). Each GPIO in the mapping must
// connect to a unique PWM channel. For example, you may not use both GPIO 0
// and GPIO 16 because they are both connected to PWM channel 0A.
//
// The PCB routes these GPIOs so they appear on headers in G-R-B order. You can
// change this in software by swapping the positions for a given lamp.  Other
// changes to this options will require hardware changes.
#define CFG_RGB_LAMP_GPIO_MAPPING \
    {11, 10, 12}, \
    {14, 15, 13}, \
    {20, 21, 19}, \
    {17, 16, 18},

// Set the divider for the PWM clock. The PWM counters wrap at the full 16-bit
// value, so with a 125 MHz system clock, the default value gives a PWM
// frequency of ~250 Hz.
#define CFG_RGB_PWM_CLOCK_DIVIDER 7.625f

// The minimum update interval. This is the minimum amount of time a host must
// wait between sending complete lamp update reports. Because this device does
// nothing else, this defaults to the maximum update latency as determined by
// the PWM period.
//
// Range: [0, 2^31-1]
// Units: Microseconds
#define CFG_RGB_MINIMUM_UPDATE_INTERVAL 4000

// The maximum latency between requesting a lamp update and the change being
// visible. Assumed to be the constant for all lamps. This is primarly
// determined by the PWM period: an update at the start of a cycle will not be
// visible until the start of the next cycle.
//
// Range: [0, 2^31-1]
// Units: Microseconds
#define CFG_RGB_LAMP_UPDATE_LATENCY 4000

// The built-in animation framerate.
//
// Range: [1, PWM frequency]
// Units: Hz
#define CFG_RGB_ANIMATION_FRAME_RATE 120

// The number of samples of the internal sensor to average for each
// temperature reading.
#define CFG_RGB_TEMP_SENSOR_SAMPLES 3

// The actual voltage of the ADC reference voltage used for the temperature
// sensor. This will be the value of the external reference or the measured
// value of the particular Pi Pico power supply.
//
// Range: [1, 3.3]
// Units: Volts
#define CFG_RGB_TEMP_SENSOR_REF_VOLTAGE 2.99f

// A fixed value to adjust measured temperatures by to account for differences
// between RP2040 diodes or other factors.
//
// Units: Degrees Celsius
#define CFG_RGB_TEMP_SENSOR_ADJUST 0.0f

#endif
