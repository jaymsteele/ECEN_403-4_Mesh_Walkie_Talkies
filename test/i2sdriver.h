#ifndef I2SDRIVER_H
#define I2SDRIVER_H

#include <stdint.h>
#include <stddef.h>

// Initializes I2C and I2S peripherals and configures the TLV320AIC3254 codec
void i2s_init(void);

// Reads 'length' samples from the microphone into the PCM buffer
void i2s_receive(int16_t *buffer, size_t length);

// Writes 'length' samples from the PCM buffer to the speaker
void i2s_transmit(int16_t *buffer, size_t length);

#endif // I2SDRIVER_H
