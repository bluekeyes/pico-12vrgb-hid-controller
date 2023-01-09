#ifndef RGB_RGB_H_
#define RGB_RGB_H_

#include <stdint.h>

#include "config.h"

#define RGB_COLOR_LEVEL_COUNT       255
#define RGB_INTENSITY_LEVEL_COUNT   1

// TODO(bkeyes): refactor this
//
//   - rename this to lamp.h
//     - it should contain stuff about individual lamps
//     - rename types as needed

// rgb_tuple_t contains a (red, green, blue, intensity) tuple for a lamp.
typedef struct __attribute__ ((packed)) {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t i;
} rgb_tuple_t;

extern const int32_t  rgb_lamp_positions[CFG_RGB_LAMP_COUNT][3];
extern const uint16_t rgb_lamp_purposes[CFG_RGB_LAMP_COUNT];
extern const uint8_t  rgb_lamp_gpios[CFG_RGB_LAMP_COUNT][3];

void rgb_init();
void rgb_set_lamp_color(uint8_t lamp_id, rgb_tuple_t const *tuple);

#endif // RGB_RGB_H_
