#ifndef DEVICE_TEMPERATURE_H_
#define DEVICE_TEMPERATURE_H_

void temperature_init();

/**
 * @brief Returns the current internal temperature in centidegrees celsius.
 */
int16_t temperature_read();

#endif /* DEVICE_TEMPERATURE_H_ */
