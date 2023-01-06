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

void rgb_set_lamp_color_tuple(rgb_lamp_id_t lamp_id, rgb_lamp_id_t const *tuple)
{
    rgb_set_lamp_color(lamp_id, tuple[0], tuple[1], tuple[2], tuple[3]);
}

void rgb_set_lamp_color(rgb_lamp_id_t lamp_id, rgb_level_t red, rgb_level_t green, rgb_level_t blue, rgb_level_t intensity)
{
    if (lamp_id > CFG_RGB_LAMP_COUNT - 1) {
        return;
    }

    uint8_t rp = rgb_lamp_gpios[lamp_id][0];
    uint8_t gp = rgb_lamp_gpios[lamp_id][1];
    uint8_t bp = rgb_lamp_gpios[lamp_id][2];

    if (intensity == 0) {
        pwm_set_gpio_level(rp, 0);
        pwm_set_gpio_level(gp, 0);
        pwm_set_gpio_level(bp, 0);
    } else {
        pwm_set_gpio_level(rp, rgb_get_pwm_level(red));
        pwm_set_gpio_level(gp, rgb_get_pwm_level(green));
        pwm_set_gpio_level(bp, rgb_get_pwm_level(blue));
    }
}

uint16_t rgb_get_pwm_level(rgb_level_t level)
{
    // squaring a value is a simple approximation for gamma corection
    return ((uint16_t) level) * ((uint16_t) level);
}
