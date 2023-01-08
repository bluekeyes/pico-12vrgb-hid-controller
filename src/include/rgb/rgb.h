#ifndef RGB_RGB_H_
#define RGB_RGB_H_

#include <stdint.h>

#include "config.h"

// TODO(bkeyes): this header is a weird mix of functionality:
//
//  - functions for interacting with PWM output
//  - types for representing colors
//  - functions for manipulating/converting colors
//  - constant arrays of lamp position data
//
// Some of this should probably be in controller.h or a new lamp.h, so that all
// the color stuff can go in one place.

// rgb_lamp_id_t identifies an addressable lamp.
typedef uint8_t rgb_lamp_id_t;

// rgb_level_t contains a level value for the red, green, blue, or intensity channel.
typedef uint8_t rgb_level_t;

#define RGB_COLOR_LEVEL_COUNT       (1 << sizeof(rgb_level_t) - 1)
#define RGB_INTENSITY_LEVEL_COUNT   1

// rgb_tuple_t contains a (red, green, blue, intensity) tuple for a lamp.
typedef struct __attribute__ ((packed)) {
    rgb_level_t r;
    rgb_level_t g;
    rgb_level_t b;
    rgb_level_t i;
} rgb_tuple_t;

// rgb_oklab_t contains an Oklab tuple, used for color transformations.
typedef struct {
    float L;
    float a;
    float b;
} rgb_oklab_t;

extern const int32_t  rgb_lamp_positions[CFG_RGB_LAMP_COUNT][3];
extern const uint16_t rgb_lamp_purposes[CFG_RGB_LAMP_COUNT];
extern const uint8_t  rgb_lamp_gpios[CFG_RGB_LAMP_COUNT][3];

void rgb_init();
void rgb_set_lamp_color(rgb_lamp_id_t lamp_id, rgb_tuple_t const *tuple);

rgb_oklab_t rgb_to_oklab(rgb_tuple_t rgb);
rgb_tuple_t rgb_from_oklab(rgb_oklab_t lab);

#endif // RGB_RGB_H_
