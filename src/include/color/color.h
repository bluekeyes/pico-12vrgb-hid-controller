#ifndef COLOR_COLOR_H_
#define COLOR_COLOR_H_

#include <stdint.h>

/**
 * @brief A linear RGB color with integer channel values in [0, 255].
 *
 * The struct is packed so that it can be directly mapped to array
 * representations of RGB data.
 */
struct __attribute__ ((packed)) RGBi {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

/**
 * @brief A linear RGB color with float channel values in [0.0, 1.0].
 */
struct RGBf {
    float r;
    float g;
    float b;
};

struct RGBi rgbf_to_i(struct RGBf rgb);
struct RGBf rgbi_to_f(struct RGBi rgb);

/**
 * @brief An Oklab (or L*a*b*) color with float channel values.
 */
struct Labf {
    float L;
    float a;
    float b;
};

struct Labf rgb_to_oklab(struct RGBf rgb);
struct RGBf oklab_to_rgb(struct Labf lab);

#endif /* COLOR_COLOR_H_ */
