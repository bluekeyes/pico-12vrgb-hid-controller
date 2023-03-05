#ifndef COLOR_COLOR_H_
#define COLOR_COLOR_H_

#include <stdint.h>

/**
 * @brief An RGB color with integer channel values in [0, 255].
 *
 * The struct is packed so that it can be directly mapped to array
 * representations of RGB data.
 */
struct __attribute__ ((packed)) RGBu8 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

/**
 * @brief An RGB color with integer channel values in [0, 65535].
 */
struct RGBu16 {
    uint16_t r;
    uint16_t g;
    uint16_t b;
};

/**
 * @brief An RGB color with float channel values in [0.0, 1.0].
 */
struct RGB {
    float r;
    float g;
    float b;
};

struct RGB rgb_from_u8(struct RGBu8 rgb);
struct RGBu8 rgb_to_u8(struct RGB rgb);
struct RGBu16 rgb_to_u16(struct RGB rgb);
struct RGB rgb_to_linear_rgb(struct RGB rgb);

/**
 * @brief An Oklab (or L*a*b*) color with float channel values.
 */
struct Lab {
    float L;
    float a;
    float b;
};

struct Lab linear_rgb_to_oklab(struct RGB rgb);
struct RGB oklab_to_linear_rgb(struct Lab lab);

#endif /* COLOR_COLOR_H_ */
