/**
 * This header includes defines for fixed and configurable properties of the
 * 12VRGB controller device. Values intended for configuration are defined in
 * config.h which is included here. All other files should only access these
 * properties by including this file, instead of using config.h directly.
 *
 * Values defined directly in this file should generally not be changed without
 * matching changes to the hardware or software.
 */

#ifndef DEVICE_SPECS_H_
#define DEVICE_SPECS_H_

/**
 * Expose all configurable properties to downstream files.
 */
#include "config.h"

/**
 * The number of independently addressable lamps (channels) in the system.
 */
#define LAMP_COUNT 4

/**
 * The number of levels the for red, green, and blue color channels of each lamp.
 */
#define LAMP_COLOR_LEVELS       255

/**
 * The number of intensity levels for each lamp (on or off).
 */
#define LAMP_INTENSITY_LEVELS   1

/**
 * The maximum current drawn by the device from USB, in mA.
 */
#define DEVICE_USB_POWER   120

/**
 * The polling frequency in frames (~1ms/frame) for USB endpoints.
 */
#define DEVICE_USB_POLL_FRAMES  5

#endif /* DEVICE_SPECS_H_ */
