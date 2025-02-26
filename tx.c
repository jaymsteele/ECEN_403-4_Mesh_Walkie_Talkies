#include <stdlib.h>
#include <unistd.h>  // Include for usleep

/* TI Drivers */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/Board.h>
#include "ti_drivers_config.h"
#include "lcd.h"  // Include the lcd.h header file
/* Driverlib Header files */
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)

/* Board Header files */
#include "ti_drivers_config.h"

/* Application Header files */
#include "RFQueue.h"
#include <ti_radio_config.h>

/***** Defines *****/
#define PAYLOAD_LENGTH      30
#define PACKET_INTERVAL     (uint32_t)(400000*0.2f)  // 1000ms
#define RX_TIMEOUT          (uint32_t)(400000*0.5f)  // 500ms
#define NUM_DATA_ENTRIES    2
#define NUM_APPENDED_BYTES  2
#define RECEIVING_HOLD_TIME (uint32_t)(400000*2.0f) // 2 seconds hold in receiving mode
I2C_Handle i2cHandle;  // Global I2C handle
/***** Variable declarations *****/
static RF_Object rfObject;
static RF_Handle rfHandle;
static uint8_t rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES, PAYLOAD_LENGTH, NUM_APPENDED_BYTES)] __attribute__((aligned(4)));
static rfc_propRxOutput_t rxStatistics;
static dataQueue_t dataQueue;
static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t packetLength;
static uint8_t* packetDataPointer;
static uint8_t txPacket[PAYLOAD_LENGTH];
static uint8_t rxPacket[PAYLOAD_LENGTH + NUM_APPENDED_BYTES - 1];
static uint16_t seqNumber;
static volatile bool bRxSuccess = false;
void displayDefaultMessage(void);
void displayTransmittingMessage(void);
void displayReceivingMessage(void);

void displayDefaultMessage(void) {
    LCD_clear(i2cHandle);
    LCD_print(i2cHandle, "Push To Talk: W1");
}

void displayTransmittingMessage(void) {
    LCD_clear(i2cHandle);
    LCD_print(i2cHandle, "Transmitting...");
}
void displayReceivingMessage(void) {
    LCD_clear(i2cHandle);
    LCD_print(i2cHandle, "Receiving...");
}


typedef enum {
    LCD_DEFAULT,
    LCD_TRANSMITTING,
    LCD_RECEIVING
} LCD_State;
void updateLCD(LCD_State newState);

LCD_State currentLCDState = LCD_RECEIVING;

void updateLCD(LCD_State newState) {
    if (currentLCDState != newState) {
        currentLCDState = newState;  // Update state

        LCD_clear(i2cHandle);
        if (newState == LCD_TRANSMITTING) {
            LCD_print(i2cHandle, "Transmitting...");
        } else if (newState == LCD_RECEIVING) {
            LCD_print(i2cHandle, "Receiving...");
        } else {
            LCD_print(i2cHandle, "Push To Talk: W1");
        }
    }
}
/***** Function definitions *****/
void *mainThread(void *arg0) {
    uint32_t curtime;
    RF_Params rfParams;
    RF_Params_init(&rfParams);

    GPIO_setConfig(CONFIG_GPIO_RLED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_GLED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_OFF);
    GPIO_write(CONFIG_GPIO_GLED, CONFIG_GPIO_LED_OFF);
    I2C_init();

       /* Initialize I2C parameters */
       I2C_Params i2cParams1;

       I2C_Params_init(&i2cParams1);
       i2cParams1.bitRate = I2C_100kHz;

       /* Open I2C instance */
       i2cHandle = I2C_open(CONFIG_I2C_1, &i2cParams1);
       if (i2cHandle == NULL) {
           while (1);
       }
    if (RFQueue_defineQueue(&dataQueue, rxDataEntryBuffer, sizeof(rxDataEntryBuffer), NUM_DATA_ENTRIES, PAYLOAD_LENGTH + NUM_APPENDED_BYTES)) {
        while (1);
    }
    LCD_init(i2cHandle);
    RF_cmdPropTx.pktLen = PAYLOAD_LENGTH;
    RF_cmdPropTx.pPkt = txPacket;
    RF_cmdPropTx.startTrigger.triggerType = TRIG_ABSTIME;
    RF_cmdPropTx.startTrigger.pastTrig = 1;
    RF_cmdPropTx.startTime = 0;
    RF_cmdPropTx.condition.rule = COND_STOP_ON_FALSE;

    RF_cmdPropRx.pQueue = &dataQueue;
    RF_cmdPropRx.rxConf.bAutoFlushIgnored = 1;
    RF_cmdPropRx.rxConf.bAutoFlushCrcErr = 1;
    RF_cmdPropRx.maxPktLen = PAYLOAD_LENGTH;
    RF_cmdPropRx.pOutput = (uint8_t *)&rxStatistics;
    RF_cmdPropRx.endTrigger.triggerType = TRIG_REL_PREVEND;
    RF_cmdPropRx.endTime = RX_TIMEOUT;

    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);

    curtime = RF_getCurrentTime();
    updateLCD(LCD_DEFAULT);
    uint32_t lastRxTime = 0;
    while (1) {
        if (GPIO_read(CONFIG_GPIO_BUTTON_0) == 0) {
            // Transmitting Mode
            updateLCD(LCD_TRANSMITTING);
            GPIO_toggle(CONFIG_GPIO_GLED);
            GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_OFF);

            txPacket[0] = (uint8_t)(seqNumber >> 8);
            txPacket[1] = (uint8_t)(seqNumber++);
            for (uint8_t i = 2; i < PAYLOAD_LENGTH; i++) {
                txPacket[i] = rand();
            }

            curtime += PACKET_INTERVAL;
            RF_cmdPropTx.startTime = curtime;
            RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, RF_EventCmdDone);
        }
        else {
            // Idle Mode: Waiting for packets
            RF_CmdHandle rxCmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx, RF_PriorityNormal, NULL, RF_EventRxEntryDone);
            RF_EventMask result = RF_pendCmd(rfHandle, rxCmdHandle, RF_EventRxEntryDone);

            if (result & RF_EventRxEntryDone) {
                // Receiving Mode
                if (currentLCDState != LCD_RECEIVING) {
                    updateLCD(LCD_RECEIVING);
                }
                GPIO_write(CONFIG_GPIO_GLED, CONFIG_GPIO_LED_OFF);
                GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_ON);

                // Retrieve packet
                currentDataEntry = RFQueue_getDataEntry();
                packetLength = *(uint8_t *)(&(currentDataEntry->data));
                packetDataPointer = (uint8_t *)(&(currentDataEntry->data) + 1);
                memcpy(rxPacket, packetDataPointer, (packetLength + 1));
                RFQueue_nextEntry();

                // Forward received packet
                memcpy(txPacket, rxPacket, packetLength);
                RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, RF_EventCmdDone);

                // Hold receiving mode for a while before switching back to idle
                uint32_t lastRxTime = RF_getCurrentTime();
                while (RF_getCurrentTime() - lastRxTime < RECEIVING_HOLD_TIME) {
                    RF_EventMask result = RF_pendCmd(rfHandle, rxCmdHandle, RF_EventRxEntryDone | RF_EventLastCmdDone);
                    if (result & RF_EventRxEntryDone) {
                        lastRxTime = RF_getCurrentTime();
                        GPIO_write(CONFIG_GPIO_GLED, CONFIG_GPIO_LED_OFF);
                        GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_ON);

                        currentDataEntry = RFQueue_getDataEntry();
                        packetLength = *(uint8_t *)(&(currentDataEntry->data));
                        packetDataPointer = (uint8_t *)(&(currentDataEntry->data) + 1);
                        memcpy(rxPacket, packetDataPointer, (packetLength + 1));
                        RFQueue_nextEntry();
                        memcpy(txPacket, rxPacket, packetLength);
                        RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, RF_EventCmdDone);
                    }
                }
            }
            else if (RF_getCurrentTime() - lastRxTime >= RECEIVING_HOLD_TIME) {
                // Only switch to default mode AFTER the receiving hold time has expired
                updateLCD(LCD_DEFAULT);
                GPIO_toggle(CONFIG_GPIO_GLED);
                GPIO_toggle(CONFIG_GPIO_RLED);

                // Restart RX command to keep listening for packets
                rxCmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx, RF_PriorityNormal, NULL, RF_EventRxEntryDone);
            }
        }
    }
}
