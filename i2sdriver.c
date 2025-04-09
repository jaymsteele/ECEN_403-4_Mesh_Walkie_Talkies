#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "i2sdriver.h"
#include "i2cdriver.h"
#include "i2c.h"
#include "i2s.h"
#include "TI3254.h"
#include <ti/drivers/I2C.h>
#include "ti_drivers_config.h"

I2C_Handle i2cHandle = NULL;  // Global I2C handle

#define AUDIO_BUFFER_SIZE 256

static int16_t i2s_rx_buffer[AUDIO_BUFFER_SIZE];
static int16_t i2s_tx_buffer[AUDIO_BUFFER_SIZE];

void I2S_Init(void) {
    // Initialize I2C and configure codec
    I2C_Init();

    // Reset codec
    I2C_WriteRegister(0x18, 0x01, 0x01);  // Page select reset

    // Configure codec registers (simplified)
    I2C_WriteRegister(0x18, 0x00, 0x00);  // Page 0
    I2C_WriteRegister(0x18, 0x02, 0x00);  // Sample rate settings
    I2C_WriteRegister(0x18, 0x03, 0x91);  // I2S, 16-bit
    I2C_WriteRegister(0x18, 0x0A, 0x80);  // Enable ADC
    I2C_WriteRegister(0x18, 0x0B, 0x80);  // Enable DAC

    // Configure I2S peripheral using low-level driverlib functions
    I2SBufferConfig(I2S0_BASE, (uint32_t)i2s_rx_buffer, (uint32_t)i2s_tx_buffer, AUDIO_BUFFER_SIZE, AUDIO_BUFFER_SIZE);
    I2SAudioFormatConfigure(I2S0_BASE, I2S_MEM_LENGTH_16 | I2S_WORD_LENGTH_16 | I2S_DUAL_PHASE_FMT | I2S_POS_EDGE, 1);
    I2SChannelConfigure(I2S0_BASE, I2S_LINE_INPUT | I2S_MONO_MODE, I2S_LINE_OUTPUT | I2S_MONO_MODE);
    I2SClockConfigure(I2S0_BASE, I2S_INT_WCLK | I2S_NORMAL_WCLK);

    I2SPointerSet(I2S0_BASE, true, i2s_rx_buffer);
    I2SPointerSet(I2S0_BASE, false, i2s_tx_buffer);

    I2SEnable(I2S0_BASE);
}

void I2S_ReadMicrophone(int16_t *buffer, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        buffer[i] = i2s_rx_buffer[i];
    }
    I2SPointerUpdate(I2S0_BASE, true);
}

void I2S_PlaySpeaker(int16_t *buffer, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        i2s_tx_buffer[i] = buffer[i];
    }
    I2SPointerUpdate(I2S0_BASE, false);
}
