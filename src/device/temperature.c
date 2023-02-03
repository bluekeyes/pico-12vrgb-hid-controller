#include <stdint.h>

#include "hardware/adc.h"

#include "device/specs.h"
#include "device/temperature.h"

#define TEMP_SENSOR_ADC_INPUT 4

void temperature_init()
{
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(TEMP_SENSOR_ADC_INPUT);
}

int16_t temperature_read()
{
    // Scale 12-bit ADC values to full voltage range
    const float volts_per_unit = CFG_RGB_TEMP_SENSOR_REF_VOLTAGE / (1 << 12);

    // Sample the internal temperature sensor
    uint32_t adc_value = 0;
    for (uint8_t i = 0; i < CFG_RGB_TEMP_SENSOR_SAMPLES; i++) {
        adc_value += adc_read();
    }

    // Average samples and convert to voltage
    float voltage = (float) (adc_value / CFG_RGB_TEMP_SENSOR_SAMPLES) * volts_per_unit;

    // Compute temperature using formula from RP2040 datasheet
    float temp = 27.0f - (voltage - 0.706f) / 0.001721f + CFG_RGB_TEMP_SENSOR_ADJUST;

    // Convert to integer centidegrees
    return (int16_t) (100.0f * temp);
}
