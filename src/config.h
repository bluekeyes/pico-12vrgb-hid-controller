#ifndef CONFIG_H_
#define CONFIG_H_

// TODO(bkeyes): a lot of this isn't really configuration, i.e. it can't change without other code changes

// The number of RGB channels
#define CFG_RGB_CHANNELS 1

// The number of bits used to represent each RGB channel
#define CFG_RGB_BITS_PER_CHANNEL    8
#define CFG_RGB_LEVELS_PER_CHANNEL  (1 << CFG_RGB_BITS_PER_CHANNEL - 1)
#define CFG_RGB_INTENSITY_LEVELS    1

// The dimensions of the bounding box (in micrometers) that contains all lights
#define CFG_RGB_BOUNDING_BOX_WIDTH  0
#define CFG_RGB_BOUNDING_BOX_HEIGHT 0
#define CFG_RGB_BOUNDING_BOX_DEPTH  0

#endif
