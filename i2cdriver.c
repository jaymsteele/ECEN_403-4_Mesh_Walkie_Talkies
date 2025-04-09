#include <ti/drivers/I2C.h>
#include "i2cdriver.h"
#include "ti_drivers_config.h"

extern I2C_Handle i2cHandle;  // Declared elsewhere in your code

void I2C_Init(void) {
    I2C_Params params;
    I2C_Params_init(&params);
    params.bitRate = I2C_100kHz;

    i2cHandle = I2C_open(CONFIG_I2C_1, &params);  // Use correct instance
    if (i2cHandle == NULL) {
        while (1);  // Failed to initialize I2C
    }
}

bool I2C_WriteRegister(uint8_t deviceAddress, uint8_t regAddr, uint8_t value) {
    uint8_t txBuf[2] = { regAddr, value };

    I2C_Transaction transaction;
    //I2C_Transaction_init(&transaction);
    transaction.writeBuf = txBuf;
    transaction.writeCount = sizeof(txBuf);
    transaction.readBuf = NULL;
    transaction.readCount = 0;
    transaction.targetAddress = deviceAddress;  //Correct field name

    return I2C_transfer(i2cHandle, &transaction);
}

