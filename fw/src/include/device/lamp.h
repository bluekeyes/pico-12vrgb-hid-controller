#ifndef DEVICE_LAMP_H_
#define DEVICE_LAMP_H_

#include <stdint.h>

#include "color/color.h"
#include "device/specs.h"

#define MAX_LAMP_ID (LAMP_COUNT - 1)

/**
 * LampValue is a value that can be set on a lamp. For convenience, it maps
 * directly to the format used in HID reports to send lamp values.
 */
struct __attribute__ ((packed)) LampValue {
    struct RGBi rgb;
    uint8_t i;
};

extern const int32_t  lamp_positions[LAMP_COUNT][3];
extern const uint16_t lamp_purposes[LAMP_COUNT];
extern const uint8_t  lamp_gpios[LAMP_COUNT][3];

void lamp_init();
void lamp_set_value(uint8_t lamp_id, struct LampValue value);
void lamp_set_off(uint8_t lamp_id);

static inline struct LampValue lamp_value_from_rgb(struct RGBi rgb)
{
    struct LampValue value = {
        .rgb = rgb,
        .i = 0x01,
    };
    return value;
}

#endif /* DEVICE_LAMP_H_ */
