#ifndef I2C_HELPER_H
#define I2C_HELPER_H

#include <stdbool.h>
#include <stdint.h>

void I2C_Init(void);
bool I2C_WriteRegister(uint8_t addr, uint8_t reg, uint8_t value);

#endif
