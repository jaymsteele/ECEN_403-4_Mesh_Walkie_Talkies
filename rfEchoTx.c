/*
 * Copyright (c) 2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/Board.h>
#include <ti/drivers/I2S.h>
#include <ti_radio_config.h>
//#include <DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)>
#include "ti_drivers_config.h"
#include "RFQueue.h"
#include "lcd.h"
#include "i2sdriver.h"
#include "ima_adpcm.h"

#define PAYLOAD_LENGTH      30
#define PACKET_INTERVAL     (uint32_t)(40000 * 0.2f)
#define RX_TIMEOUT          (uint32_t)(400000 * 0.5f)
#define NUM_DATA_ENTRIES    2
#define NUM_APPENDED_BYTES  2
#define RECEIVING_HOLD_TIME (uint32_t)(400000 * 2.0f)
#define AUDIO_PACKET_SIZE   64
#define WALKIE_ID           2
#define AUDIO_FRAME_SIZE 64
#define COMPRESSED_FRAME_SIZE (AUDIO_FRAME_SIZE / 2)

static int16_t audio_samples[AUDIO_FRAME_SIZE];
static uint8_t compressed_audio[COMPRESSED_FRAME_SIZE];

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
static I2C_Handle i2cHandle;
static ADPCM_State adpcmState;

int16_t audioBuffer[AUDIO_PACKET_SIZE];
uint8_t compressedBuffer[PAYLOAD_LENGTH - 3];

typedef enum {
    LCD_DEFAULT,
    LCD_TRANSMITTING,
    LCD_RECEIVING
} LCD_State;

LCD_State currentLCDState = LCD_RECEIVING;

void updateLCD(LCD_State newState, uint8_t senderId) {
    if (currentLCDState != newState) {
        currentLCDState = newState;
        LCD_clear(i2cHandle);

        if (newState == LCD_TRANSMITTING) {
            LCD_print(i2cHandle, "Transmitting...");
        } else if (newState == LCD_RECEIVING) {
            char lcdMessage[16];
            snprintf(lcdMessage, sizeof(lcdMessage), "Receiving W%d", senderId);
            LCD_print(i2cHandle, lcdMessage);
        } else {
            char lcdMessage[20];
            snprintf(lcdMessage, sizeof(lcdMessage), "Push To Talk: W%d", WALKIE_ID);
            LCD_print(i2cHandle, lcdMessage);
        }
    }
}

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
    I2C_Params i2cParams;
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_100kHz;
    i2cHandle = I2C_open(CONFIG_I2C_1, &i2cParams);
    if (i2cHandle == NULL) while (1);

    I2S_Init();
    LCD_init(i2cHandle);

    if (RFQueue_defineQueue(&dataQueue, rxDataEntryBuffer, sizeof(rxDataEntryBuffer), NUM_DATA_ENTRIES, PAYLOAD_LENGTH + NUM_APPENDED_BYTES)) {
        while (1);
    }

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
    updateLCD(LCD_DEFAULT, 0);
    uint32_t lastRxTime = 0;

    while (1) {
        if (GPIO_read(CONFIG_GPIO_BUTTON_0) == 0) {
            updateLCD(LCD_TRANSMITTING, 0);
            GPIO_toggle(CONFIG_GPIO_GLED);
            GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_OFF);

            // Get audio samples and encode
            I2S_ReadMicrophone(audio_samples, AUDIO_FRAME_SIZE);
            //ima_adpcm_encode(audio_samples, compressed_audio, AUDIO_FRAME_SIZE, &adpcmState);

            // Prepare packet (1 byte ID + 2 byte seq + 30 bytes compressed audio)
            txPacket[0] = WALKIE_ID;
            txPacket[1] = (uint8_t)(seqNumber >> 8);
            txPacket[2] = (uint8_t)(seqNumber++);
            memcpy(&txPacket[3], compressed_audio, PAYLOAD_LENGTH - 3);

            // Transmit
            curtime += PACKET_INTERVAL;
            RF_cmdPropTx.startTime = curtime;
            RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, RF_EventCmdDone);
        } else {
            RF_CmdHandle rxCmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx, RF_PriorityNormal, NULL, RF_EventRxEntryDone);
            RF_EventMask result = RF_pendCmd(rfHandle, rxCmdHandle, RF_EventRxEntryDone);

            if (result & RF_EventRxEntryDone) {
                GPIO_write(CONFIG_GPIO_GLED, CONFIG_GPIO_LED_OFF);
                GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_ON);

                currentDataEntry = RFQueue_getDataEntry();
                packetLength = *(uint8_t *)(&(currentDataEntry->data));
                packetDataPointer = (uint8_t *)(&(currentDataEntry->data) + 1);
                memcpy(rxPacket, packetDataPointer, (packetLength + 1));
                RFQueue_nextEntry();

                uint8_t senderId = rxPacket[0];
                if (currentLCDState != LCD_RECEIVING) {
                    updateLCD(LCD_RECEIVING, senderId);
                }

                for (int i = 0; i < (PAYLOAD_LENGTH - 3); i++) {
                    //ima_adpcm_expand(&adpcmState, rxPacket[i + 3], &audioBuffer[i * 2]);
                    //don't use this function because expand doesn't work as expected
                }
                //i2s_transmit(audioBuffer, AUDIO_PACKET_SIZE);
                //removed function from i2sdriver since I couldn't get it to work

                memcpy(txPacket, rxPacket, packetLength);
                RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, RF_EventCmdDone);

                lastRxTime = RF_getCurrentTime();
                while (RF_getCurrentTime() - lastRxTime < RECEIVING_HOLD_TIME) {
                    result = RF_pendCmd(rfHandle, rxCmdHandle, RF_EventRxEntryDone | RF_EventLastCmdDone);
                    if (result & RF_EventRxEntryDone) {
                        lastRxTime = RF_getCurrentTime();
                        GPIO_write(CONFIG_GPIO_GLED, CONFIG_GPIO_LED_OFF);
                        GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_ON);

                        if (packetLength > PAYLOAD_LENGTH) {
                            packetLength = PAYLOAD_LENGTH;
                        }
                        packetDataPointer = (uint8_t *)(&(currentDataEntry->data) + 1);
                        memcpy(rxPacket, packetDataPointer, (packetLength + 1));
                        RFQueue_nextEntry();

                        memcpy(txPacket, rxPacket, packetLength);
                        RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, RF_EventCmdDone);
                    }
                }
            } else if (RF_getCurrentTime() - lastRxTime >= RECEIVING_HOLD_TIME) {
                updateLCD(LCD_DEFAULT, 0);
                GPIO_toggle(CONFIG_GPIO_GLED);
                GPIO_toggle(CONFIG_GPIO_RLED);

                RF_yield(rfHandle);
                RF_close(rfHandle);
                rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
                RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
            }
        }
    }
}
