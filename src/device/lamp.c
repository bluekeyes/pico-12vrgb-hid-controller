#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include "device/lamp.h"
#include "device/specs.h"

const int32_t lamp_positions[LAMP_COUNT][3] = {
    CFG_RGB_LAMP_POSITIONS
};

const uint16_t lamp_purposes[LAMP_COUNT] = {
    CFG_RGB_LAMP_PURPOSES
};

const uint8_t lamp_gpios[LAMP_COUNT][3] = {
    CFG_RGB_LAMP_GPIO_MAPPING
};

static inline uint16_t get_pwm_level(uint8_t value)
{
    // squaring a value is a simple approximation for gamma corection
    return ((uint16_t) value) * ((uint16_t) value);
}

void lamp_init()
{
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, CFG_RGB_PWM_CLOCK_DIVIDER);

    uint8_t slice_mask = 0;
    for (uint8_t i = 0; i < LAMP_COUNT; i++) {
        for (uint8_t j = 0; j < 3; j++) {
            uint8_t pin = lamp_gpios[i][j];
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

void lamp_set_value(uint8_t lamp_id, struct LampValue value)
{
    if (lamp_id > MAX_LAMP_ID) {
        return;
    }

    uint8_t rp = lamp_gpios[lamp_id][0];
    uint8_t gp = lamp_gpios[lamp_id][1];
    uint8_t bp = lamp_gpios[lamp_id][2];

    if (value.i > 0) {
        pwm_set_gpio_level(rp, get_pwm_level(value.r));
        pwm_set_gpio_level(gp, get_pwm_level(value.g));
        pwm_set_gpio_level(bp, get_pwm_level(value.b));
    } else {
        pwm_set_gpio_level(rp, 0);
        pwm_set_gpio_level(gp, 0);
        pwm_set_gpio_level(bp, 0);
    }
}
