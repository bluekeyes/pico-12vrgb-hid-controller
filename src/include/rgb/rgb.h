#ifndef RGB_RGB_H_
#define RGB_RGB_H_

#include <stdint.h>

#include "config.h"

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

extern const int32_t  rgb_lamp_positions[CFG_RGB_LAMP_COUNT][3];
extern const uint16_t rgb_lamp_purposes[CFG_RGB_LAMP_COUNT];
extern const uint8_t  rgb_lamp_gpios[CFG_RGB_LAMP_COUNT][3];

void rgb_init();
void rgb_set_lamp_color(rgb_lamp_id_t lamp_id, rgb_tuple_t const *tuple);

#endif // RGB_RGB_H_
