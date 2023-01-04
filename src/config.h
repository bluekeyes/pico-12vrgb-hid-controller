#ifndef CONFIG_H_
#define CONFIG_H_

// The device version as 0xJJMN, where JJ is the major version, M is the minor
// version and N is the patch version.
#define CFG_RGB_DEVICE_VERSION 0x0010

// The number of individually addressable lamps / RGB channels
// Range: [1, 256]
#define CFG_RGB_LAMP_COUNT 1

// The number of lamps that can be updated in a single LampMultiUpdateReport
// Range: [1, 8]
#define CFG_RGB_MULTIUPDATE_SIZE 1

// The dimensions of the bounding box (in micrometers) that contains all lights
// Range: [0, 2^31-1]
#define CFG_RGB_BOUNDING_BOX_WIDTH  0
#define CFG_RGB_BOUNDING_BOX_HEIGHT 0
#define CFG_RGB_BOUNDING_BOX_DEPTH  0

#endif
