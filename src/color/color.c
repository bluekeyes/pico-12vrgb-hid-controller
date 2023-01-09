// RGB to Oklab conversion code from https://bottosson.github.io/posts/oklab/

#include <math.h>

#include "color/color.h"

struct RGBi rgbf_to_i(struct RGBf rgb)
{
    struct RGBi i = {
        (uint8_t) (255.f * rgb.r),
        (uint8_t) (255.f * rgb.g),
        (uint8_t) (255.f * rgb.b),
    };
    return i;
}

struct RGBf rgbi_to_f(struct RGBi rgb)
{
    struct RGBf f = {
        ((float) rgb.r) / 255.f,
        ((float) rgb.g) / 255.f,
        ((float) rgb.b) / 255.f,
    };
    return f;
}

struct Labf rgb_to_oklab(struct RGBf rgb)
{
    float l = 0.4122214708f * rgb.r + 0.5363325363f * rgb.g + 0.0514459929f * rgb.b;
	float m = 0.2119034982f * rgb.r + 0.6806995451f * rgb.g + 0.1073969566f * rgb.b;
	float s = 0.0883024619f * rgb.r + 0.2817188376f * rgb.g + 0.6299787005f * rgb.b;

    float l_ = cbrtf(l);
    float m_ = cbrtf(m);
    float s_ = cbrtf(s);

    struct Labf lab = {
        0.2104542553f*l_ + 0.7936177850f*m_ - 0.0040720468f*s_,
        1.9779984951f*l_ - 2.4285922050f*m_ + 0.4505937099f*s_,
        0.0259040371f*l_ + 0.7827717662f*m_ - 0.8086757660f*s_,
    };
    return lab;
}

struct RGBf oklab_to_rgb(struct Labf lab)
{
    float l_ = lab.L + 0.3963377774f * lab.a + 0.2158037573f * lab.b;
    float m_ = lab.L - 0.1055613458f * lab.a - 0.0638541728f * lab.b;
    float s_ = lab.L - 0.0894841775f * lab.a - 1.2914855480f * lab.b;

    float l = l_*l_*l_;
    float m = m_*m_*m_;
    float s = s_*s_*s_;

    struct RGBf rgb = {
		+4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
		-1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
		-0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
    };
    return rgb;
}
