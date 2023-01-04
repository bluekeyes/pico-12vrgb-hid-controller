#ifndef RGB_H_
#define RGB_H_

#include <stdint.h>

#include "config.h"

// rgb_lamp_id_t identifies an addressable lamp.
typedef uint8_t rgb_lamp_id_t;

// rgb_level_t contains a level value for the red, green, blue, or intensity channel.
typedef uint8_t rgb_level_t;

#define RGB_COLOR_LEVEL_COUNT       (1 << sizeof(rgb_level_t) - 1)
#define RGB_INTENSITY_LEVEL_COUNT   1

extern const int32_t rgb_lamp_positions[];
extern const uint16_t rgb_lamp_purposes[];

#endif
