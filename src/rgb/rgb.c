#include <math.h>

#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include "config.h"
#include "rgb/rgb.h"

const int32_t rgb_lamp_positions[CFG_RGB_LAMP_COUNT][3] = {
    CFG_RGB_LAMP_POSITIONS
};

const uint16_t rgb_lamp_purposes[CFG_RGB_LAMP_COUNT] = {
    CFG_RGB_LAMP_PURPOSES
};

const uint8_t rgb_lamp_gpios[CFG_RGB_LAMP_COUNT][3] = {
    CFG_RGB_LAMP_GPIO_MAPPING
};

static inline uint16_t rgb_get_pwm_level(uint8_t level)
{
    // squaring a value is a simple approximation for gamma corection
    return ((uint16_t) level) * ((uint16_t) level);
}

void rgb_init()
{
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, CFG_RGB_PWM_CLOCK_DIVIDER);

    uint8_t slice_mask = 0;
    for (uint8_t i = 0; i < CFG_RGB_LAMP_COUNT; i++) {
        for (uint8_t j = 0; j < 3; j++) {
            uint8_t pin = rgb_lamp_gpios[i][j];
            gpio_set_function(pin, GPIO_FUNC_PWM);

            uint8_t slice = pwm_gpio_to_slice_num(pin);
            if ((slice_mask & (1 << slice)) == 0) {
                pwm_init(slice, &config, false);
                slice_mask |= (1 << slice);
            }
            pwm_set_chan_level(slice, pwm_gpio_to_channel(pin), 0);
        }
    }

    pwm_set_mask_enabled(slice_mask);
}

void rgb_set_lamp_color(uint8_t lamp_id, rgb_tuple_t const *tuple)
{
    if (lamp_id > CFG_RGB_LAMP_COUNT - 1) {
        return;
    }

    uint8_t rp = rgb_lamp_gpios[lamp_id][0];
    uint8_t gp = rgb_lamp_gpios[lamp_id][1];
    uint8_t bp = rgb_lamp_gpios[lamp_id][2];

    if (tuple->i == 0) {
        pwm_set_gpio_level(rp, 0);
        pwm_set_gpio_level(gp, 0);
        pwm_set_gpio_level(bp, 0);
    } else {
        pwm_set_gpio_level(rp, rgb_get_pwm_level(tuple->r));
        pwm_set_gpio_level(gp, rgb_get_pwm_level(tuple->g));
        pwm_set_gpio_level(bp, rgb_get_pwm_level(tuple->b));
    }
}

// RGB to Oklab conversion code from https://bottosson.github.io/posts/oklab/
// (public domain) and modified for 8-bit RGB values.

rgb_oklab_t rgb_to_oklab(rgb_tuple_t rgb)
{
    float r = ((float) rgb.r) / 255.0f;
    float g = ((float) rgb.g) / 255.0f;
    float b = ((float) rgb.b) / 255.0f;

    float l = 0.4122214708f * r + 0.5363325363f * g + 0.0514459929f * b;
	float m = 0.2119034982f * r + 0.6806995451f * g + 0.1073969566f * b;
	float s = 0.0883024619f * r + 0.2817188376f * g + 0.6299787005f * b;

    float l_ = cbrtf(l);
    float m_ = cbrtf(m);
    float s_ = cbrtf(s);

    rgb_oklab_t result = {
        0.2104542553f*l_ + 0.7936177850f*m_ - 0.0040720468f*s_,
        1.9779984951f*l_ - 2.4285922050f*m_ + 0.4505937099f*s_,
        0.0259040371f*l_ + 0.7827717662f*m_ - 0.8086757660f*s_,
    };
    return result;
}

rgb_tuple_t rgb_from_oklab(rgb_oklab_t lab)
{
    float l_ = lab.L + 0.3963377774f * lab.a + 0.2158037573f * lab.b;
    float m_ = lab.L - 0.1055613458f * lab.a - 0.0638541728f * lab.b;
    float s_ = lab.L - 0.0894841775f * lab.a - 1.2914855480f * lab.b;

    float l = l_*l_*l_;
    float m = m_*m_*m_;
    float s = s_*s_*s_;

    rgb_tuple_t result = {
		(uint8_t) (255.0f * (+4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s)),
		(uint8_t) (255.0f * (-1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s)),
		(uint8_t) (255.0f * (-0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s)),
        1,
    };
    return result;
}
