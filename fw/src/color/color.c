// RGB to Oklab conversion code from https://bottosson.github.io/posts/oklab/

#include <math.h>

#include "color/color.h"

static inline uint8_t channel_to_u8(float c)
{
    uint16_t i = (uint16_t) (255.f * c + 0.5f);
    return i > 255 ? 255 : (uint8_t) i;
}

static inline uint16_t channel_to_u16(float c)
{
    uint32_t i = (uint32_t) (65535.f * c + 0.5f);
    return i > 65535 ? 65535 : (uint16_t) i;
}

static inline float channel_to_linear(float c)
{
    if (c >= 0.04045) {
        return powf((c + 0.055f)/(1.055f), 2.4f);
    } else {
        return c / 12.92f;
    }
}

struct RGBu8 rgb_to_u8(struct RGB rgb)
{
    struct RGBu8 u8 = {
        channel_to_u8(rgb.r),
        channel_to_u8(rgb.g),
        channel_to_u8(rgb.b),
    };
    return u8;
}

struct RGBu16 rgb_to_u16(struct RGB rgb)
{
    struct RGBu16 u16 = {
        channel_to_u16(rgb.r),
        channel_to_u16(rgb.g),
        channel_to_u16(rgb.b),
    };
    return u16;
}

struct RGB rgb_from_u8(struct RGBu8 rgb)
{
    struct RGB f = {
        ((float) rgb.r) / 255.f,
        ((float) rgb.g) / 255.f,
        ((float) rgb.b) / 255.f,
    };
    return f;
}

struct RGB rgb_to_linear_rgb(struct RGB rgb)
{
    struct RGB lin = {
        channel_to_linear(rgb.r),
        channel_to_linear(rgb.g),
        channel_to_linear(rgb.b),
    };
    return lin;
}

struct Lab linear_rgb_to_oklab(struct RGB rgb)
{
    float l = 0.4122214708f * rgb.r + 0.5363325363f * rgb.g + 0.0514459929f * rgb.b;
	float m = 0.2119034982f * rgb.r + 0.6806995451f * rgb.g + 0.1073969566f * rgb.b;
	float s = 0.0883024619f * rgb.r + 0.2817188376f * rgb.g + 0.6299787005f * rgb.b;

    float l_ = cbrtf(l);
    float m_ = cbrtf(m);
    float s_ = cbrtf(s);

    struct Lab lab = {
        0.2104542553f*l_ + 0.7936177850f*m_ - 0.0040720468f*s_,
        1.9779984951f*l_ - 2.4285922050f*m_ + 0.4505937099f*s_,
        0.0259040371f*l_ + 0.7827717662f*m_ - 0.8086757660f*s_,
    };
    return lab;
}

struct RGB oklab_to_linear_rgb(struct Lab lab)
{
    float l_ = lab.L + 0.3963377774f * lab.a + 0.2158037573f * lab.b;
    float m_ = lab.L - 0.1055613458f * lab.a - 0.0638541728f * lab.b;
    float s_ = lab.L - 0.0894841775f * lab.a - 1.2914855480f * lab.b;

    float l = l_*l_*l_;
    float m = m_*m_*m_;
    float s = s_*s_*s_;

    struct RGB rgb = {
		+4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
		-1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
		-0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
    };
    return rgb;
}
