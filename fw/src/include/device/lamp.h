#ifndef DEVICE_LAMP_H_
#define DEVICE_LAMP_H_

#include <stdint.h>

#include "color/color.h"
#include "device/specs.h"

#define MAX_LAMP_ID (LAMP_COUNT - 1)

/**
 * LampValue is a value that can be set on a lamp.
 */
struct LampValue {
    uint16_t r;
    uint16_t g;
    uint16_t b;
    uint8_t  i;
};

extern const int32_t  lamp_positions[LAMP_COUNT][3];
extern const uint16_t lamp_purposes[LAMP_COUNT];
extern const uint8_t  lamp_gpios[LAMP_COUNT][3];

void lamp_init();
void lamp_set_value(uint8_t lamp_id, struct LampValue value);

static inline struct LampValue lamp_value_from_rgbu16(struct RGBu16 rgb)
{
    struct LampValue value = {
        .r = rgb.r,
        .g = rgb.g,
        .b = rgb.b,
        .i = 0x01,
    };
    return value;
}

static inline struct LampValue lamp_value_from_u8_tuple(uint8_t const *rgbi)
{
    // squaring values is a simple approximation for gamma correction
    struct LampValue value = {
        .r = ((uint16_t) rgbi[0]) * ((uint16_t) rgbi[0]),
        .g = ((uint16_t) rgbi[1]) * ((uint16_t) rgbi[1]),
        .b = ((uint16_t) rgbi[2]) * ((uint16_t) rgbi[2]),
        .i = rgbi[3],
    };
    return value;
}

static inline struct LampValue lamp_value_off()
{
    struct LampValue value = {
        .r = 0,
        .g = 0,
        .b = 0,
        .i = 0,
    };
    return value;
}

#endif /* DEVICE_LAMP_H_ */
