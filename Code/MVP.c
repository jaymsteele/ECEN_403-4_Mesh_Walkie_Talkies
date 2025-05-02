#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>  // Include for usleep
#include <stdio.h>
/* TI Drivers */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/I2S.h>
#include <ti/drivers/Board.h>
#include "ti_drivers_config.h"
#include "lcd.h"  // Include the lcd.h header file
#include "adpcm.h"
#include "AudioCodec.h"
#include <semaphore.h>
#include <pthread.h>

/* Driverlib Header files */
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)
/* Board Header files */
#include "ti_drivers_config.h"
/* Application Header files */
#include "RFQueue.h"
#include <ti_radio_config.h>
/***** Defines *****/
#define PAYLOAD_LENGTH      255
#define PACKET_INTERVAL     (uint32_t)(40000*0.2f)  // 20ms
#define RX_TIMEOUT          (uint32_t)(400000*0.5f)  // 500ms
#define NUM_DATA_ENTRIES    2
#define NUM_APPENDED_BYTES  2
#define RECEIVING_HOLD_TIME (uint32_t)(400000*2.0f) // 2 seconds hold in receiving mode
I2C_Handle i2cHandle;  // Global I2C handle
#define WALKIE_ID 2  // Change this for each device (e.g., 1, 2, 3, ...)

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

#define PCM_SAMPLES_PER_PACKET      504   // 2 PCM samples per ADPCM byte
#define ADPCM_BYTES_PER_PACKET (PCM_SAMPLES_PER_PACKET / 2)

// ADPCM encoder/decoder state
static ADPCM_State adpcmEncState = {0};
static ADPCM_State adpcmDecState = {0};
#define THREADSTACKSIZE 2048
#define SAMPLE_RATE   8000
#define INPUT_OPTION  AudioCodec_MIC_MONO
#define OUTPUT_OPTION AudioCodec_SPEAKER_HP
#define NUMBUFS 10
#define BUFSIZE 256

// External I2S queues
static sem_t semDataReadyForTreatment;
static sem_t semErrorCallback;
List_List i2sReadList;
List_List treatmentList;
List_List i2sWriteList;

/*typedef struct {
    int16_t prevSample;
    int index;
} ADPCM_State;*/




static uint8_t buf1[BUFSIZE];
static uint8_t buf2[BUFSIZE];
static uint8_t buf3[BUFSIZE];
static uint8_t buf4[BUFSIZE];
static uint8_t buf5[BUFSIZE];
static uint8_t buf6[BUFSIZE];
static uint8_t buf7[BUFSIZE];
static uint8_t buf8[BUFSIZE];
static uint8_t buf9[BUFSIZE];
static uint8_t buf10[BUFSIZE];
static uint8_t *i2sBufList[NUMBUFS] = {buf1, buf2, buf3, buf4, buf5, buf6, buf7, buf8, buf9, buf10};

I2S_Transaction i2sTransaction1;
I2S_Transaction i2sTransaction2;
I2S_Transaction i2sTransaction3;
I2S_Transaction i2sTransaction4;
I2S_Transaction i2sTransaction5;
I2S_Transaction i2sTransaction6;
I2S_Transaction i2sTransaction7;
I2S_Transaction i2sTransaction8;
I2S_Transaction i2sTransaction9;
I2S_Transaction i2sTransaction10;
static I2S_Transaction *i2sTransactionList[NUMBUFS] = {&i2sTransaction1, &i2sTransaction2, &i2sTransaction3, &i2sTransaction4, &i2sTransaction5, &i2sTransaction6, &i2sTransaction7, &i2sTransaction8, &i2sTransaction9, &i2sTransaction10};
I2S_Handle i2sHandle;
I2C_Handle i2cHandle;
I2C_Params i2cParams1;
static bool i2sWriteRunning = false;
void resetI2S() {
    List_clearList(&i2sReadList);
    List_clearList(&treatmentList);
    List_clearList(&i2sWriteList);

    for (uint8_t k = 0; k < NUMBUFS / 2; k++) {
        I2S_Transaction_init(i2sTransactionList[k]);
        i2sTransactionList[k]->bufPtr = i2sBufList[k];
        i2sTransactionList[k]->bufSize = BUFSIZE;
        List_put(&i2sReadList, (List_Elem *)i2sTransactionList[k]);
    }

    for (uint8_t k = NUMBUFS / 2; k < NUMBUFS; k++) {
        I2S_Transaction_init(i2sTransactionList[k]);
        i2sTransactionList[k]->bufPtr = i2sBufList[k];
        i2sTransactionList[k]->bufSize = BUFSIZE;
        List_put(&i2sWriteList, (List_Elem *)i2sTransactionList[k]);
    }

    I2S_setReadQueueHead(i2sHandle, (I2S_Transaction *)List_head(&i2sReadList));
    I2S_setWriteQueueHead(i2sHandle, (I2S_Transaction *)List_head(&i2sWriteList));
}
static void writeCallbackFxn(I2S_Handle handle, int_fast16_t status, I2S_Transaction *transactionPtr) {
    I2S_Transaction *transactionFinished = (I2S_Transaction *)List_prev(&transactionPtr->queueElement);
    if (transactionFinished != NULL) {
        List_remove(&i2sWriteList, (List_Elem *)transactionFinished);
        List_put(&i2sReadList, (List_Elem *)transactionFinished);
    }
}

static void readCallbackFxn(I2S_Handle handle, int_fast16_t status, I2S_Transaction *transactionPtr) {
    I2S_Transaction *transactionFinished = (I2S_Transaction *)List_prev(&transactionPtr->queueElement);
    if (transactionFinished != NULL) {
        List_remove(&i2sReadList, (List_Elem *)transactionFinished);
        List_put(&treatmentList, (List_Elem *)transactionFinished);
        sem_post(&semDataReadyForTreatment);
    }
}
static void errCallbackFxn(I2S_Handle handle, int_fast16_t status, I2S_Transaction *transactionPtr) {
    sem_post(&semErrorCallback);
}
static void initAudioStreaming(void) {
    uint8_t status = AudioCodec_open();
    if (status != AudioCodec_STATUS_SUCCESS) {
        while (1);  // Error initializing codec
    }

    status = AudioCodec_config(AudioCodec_TI_3254, AudioCodec_16_BIT, SAMPLE_RATE, AudioCodec_STEREO, AudioCodec_SPEAKER_HP, AudioCodec_MIC_MONO);

    if (status != AudioCodec_STATUS_SUCCESS) {
        while (1);  // Error configuring codec
    }

    AudioCodec_micVolCtrl(AudioCodec_TI_3254, INPUT_OPTION, 100);
    AudioCodec_speakerVolCtrl(AudioCodec_TI_3254, OUTPUT_OPTION, 100);

    sem_init(&semDataReadyForTreatment, 0, 0);

       I2S_Params i2sParams;
       I2S_Params_init(&i2sParams);
       i2sParams.samplingFrequency = SAMPLE_RATE;
       i2sParams.fixedBufferLength = BUFSIZE;
       i2sParams.writeCallback = writeCallbackFxn;
       i2sParams.readCallback = readCallbackFxn;
       i2sParams.errorCallback = errCallbackFxn;
       i2sHandle = I2S_open(CONFIG_I2S_0, &i2sParams);
       if (i2sHandle == NULL) {
               while (1) {}
           }
       resetI2S();

}

typedef enum {
    LCD_DEFAULT,
    LCD_TRANSMITTING,
    LCD_RECEIVING
} LCD_State;

void updateLCD(LCD_State newState, uint8_t senderId);

LCD_State currentLCDState = LCD_RECEIVING;

void updateLCD(LCD_State newState, uint8_t senderId) {
    if (currentLCDState != newState) {
        currentLCDState = newState;  // Update state
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

static void initGPIO(void);
static void initI2C(void);
static void initRFQueue(void);
static void initRFCommands(void);
static void handleTransmitMode(uint32_t *curtime);
static bool handleReceiveMode(uint32_t *lastRxTime);
static void resetRFConnection(RF_Params *rfParams);
static void processReceivedPacket(void);
static void forwardReceivedPacket(void);
/***** Function definitions *****/
void *mainThread(void *arg0) {
    uint32_t curtime;
    RF_Params rfParams;
    RF_Params_init(&rfParams);

    // Initialize GPIO, I2C, RF Queue, and LCD
    I2S_init();
    initGPIO();
    initI2C();
    resetI2S();
    initRFQueue();
    initAudioStreaming();
    LCD_init(i2cHandle);

    // Initialize ADPCM states
    memset(&adpcmEncState, 0, sizeof(ADPCM_State));
    memset(&adpcmDecState, 0, sizeof(ADPCM_State));

    // Initialize RF commands
    initRFCommands();

    // Open RF driver
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);

    curtime = RF_getCurrentTime();
    updateLCD(LCD_DEFAULT, 0);
    uint32_t lastRxTime = 0;

    bool isTransmitting = false;
    bool audioStarted = false;

    while (1) {
        if (GPIO_read(CONFIG_GPIO_BUTTON_0) == 0) {
            // Push-to-talk button is pressed - transmission mode
            if (!isTransmitting) {
                isTransmitting = true;

                // Reset ADPCM encoder state for clean start
                memset(&adpcmEncState, 0, sizeof(ADPCM_State));

                // Start audio streaming if not already started
                if (!audioStarted) {
                    AudioCodec_micUnmute(AudioCodec_TI_3254, INPUT_OPTION);
                    I2S_startClocks(i2sHandle);
                    I2S_startRead(i2sHandle);
                    I2S_startWrite(i2sHandle);  // Needed for I2S operation
                    audioStarted = true;
                }
            }

            handleTransmitMode(&curtime);
        }
        else {
            // Button released - receiving mode
            if (isTransmitting) {
                isTransmitting = false;

                // Reset ADPCM decoder state for clean start of reception
                memset(&adpcmDecState, 0, sizeof(ADPCM_State));
            }

            if (!handleReceiveMode(&lastRxTime)) {
                // No packets received
                if (RF_getCurrentTime() - lastRxTime >= RECEIVING_HOLD_TIME) {
                    updateLCD(LCD_DEFAULT, 0);
                    GPIO_toggle(CONFIG_GPIO_GLED);
                    GPIO_toggle(CONFIG_GPIO_RLED);

                    // If we've been idle for a while, we can stop the audio streaming
                    // to save power
                    if (audioStarted) {
                        AudioCodec_micMute(AudioCodec_TI_3254, INPUT_OPTION);
                        I2S_stopRead(i2sHandle);
                        I2S_stopWrite(i2sHandle);
                        I2S_stopClocks(i2sHandle);
                        audioStarted = false;
                    }

                    resetRFConnection(&rfParams);
                }
            } else {
                // Packet received - ensure audio is running for playback
                if (!audioStarted) {
                    AudioCodec_speakerUnmute(AudioCodec_TI_3254, OUTPUT_OPTION);
                    AudioCodec_speakerVolCtrl(AudioCodec_TI_3254, OUTPUT_OPTION, 100);

                    I2S_startClocks(i2sHandle);
                    I2S_startWrite(i2sHandle);
                    audioStarted = true;
                }
                lastRxTime = RF_getCurrentTime();
            }
        }
    }
}

static void initGPIO(void) {
    GPIO_setConfig(CONFIG_GPIO_RLED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_GLED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_OFF);
    GPIO_write(CONFIG_GPIO_GLED, CONFIG_GPIO_LED_OFF);
}

static void initI2C(void) {
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
}

static void initRFQueue(void) {
    if (RFQueue_defineQueue(&dataQueue, rxDataEntryBuffer, sizeof(rxDataEntryBuffer),
                           NUM_DATA_ENTRIES, PAYLOAD_LENGTH + NUM_APPENDED_BYTES)) {
        while (1);
    }
}

static void initRFCommands(void) {
    // Configure TX command
    RF_cmdPropTx.pktLen = PAYLOAD_LENGTH;
    RF_cmdPropTx.pPkt = txPacket;
    RF_cmdPropTx.startTrigger.triggerType = TRIG_ABSTIME;
    RF_cmdPropTx.startTrigger.pastTrig = 1;
    RF_cmdPropTx.startTime = 0;
    RF_cmdPropTx.condition.rule = COND_STOP_ON_FALSE;

    // Configure RX command
    RF_cmdPropRx.pQueue = &dataQueue;
    RF_cmdPropRx.rxConf.bAutoFlushIgnored = 1;
    RF_cmdPropRx.rxConf.bAutoFlushCrcErr = 1;
    RF_cmdPropRx.maxPktLen = PAYLOAD_LENGTH;
    RF_cmdPropRx.pOutput = (uint8_t *)&rxStatistics;
    RF_cmdPropRx.endTrigger.triggerType = TRIG_REL_PREVEND;
    RF_cmdPropRx.endTime = RX_TIMEOUT;
}

static void handleTransmitMode(uint32_t *curtime) {
    // Update LCD and LEDs
    updateLCD(LCD_TRANSMITTING, 0);
    GPIO_toggle(CONFIG_GPIO_GLED);
    GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_OFF);

    // Prepare packet header
    txPacket[0] = WALKIE_ID;  // First byte is the walkie ID
    txPacket[1] = (uint8_t)(seqNumber >> 8);
    txPacket[2] = (uint8_t)(seqNumber++);

    // Wait for audio data to be available in the treatment list
    if (sem_trywait(&semDataReadyForTreatment) == 0) {
        // Get a transaction with audio data
        I2S_Transaction *transaction = (I2S_Transaction *)List_get(&treatmentList);
        if (transaction != NULL) {
            // Get pointer to the audio data
            int16_t *audioSamples = (int16_t *)transaction->bufPtr;
            int numSamples = transaction->bufSize / sizeof(int16_t);

            // Limit samples to what can fit in our packet (must be even for ADPCM)
            if (numSamples > PCM_SAMPLES_PER_PACKET) {
                numSamples = PCM_SAMPLES_PER_PACKET;
            }
            // Ensure we have an even number of samples
            numSamples = (numSamples / 2) * 2;

            // Compress audio using your ADPCM implementation
            uint8_t adpcmBuffer[ADPCM_BYTES_PER_PACKET];
            adpcm_encode_buffer(&adpcmEncState, audioSamples, adpcmBuffer, numSamples);

            // Copy ADPCM data to the packet starting after the header
            // Make sure we don't exceed PAYLOAD_LENGTH
            uint16_t adpcmDataSize = numSamples / 2; // 2 samples per byte in ADPCM
            if (adpcmDataSize > (PAYLOAD_LENGTH - 3)) {
                adpcmDataSize = PAYLOAD_LENGTH - 3;
            }

            memcpy(&txPacket[3], adpcmBuffer, adpcmDataSize);

            // Move the transaction to the read list for reuse
            List_put(&i2sReadList, (List_Elem *)transaction);
        } else {
            // If no audio data is available, fill with silence
            for (uint8_t i = 3; i < PAYLOAD_LENGTH; i++) {
                txPacket[i] = 0;
            }
        }
    } else {
        // No data ready, fill with silence
        for (uint8_t i = 3; i < PAYLOAD_LENGTH; i++) {
            txPacket[i] = 0;
        }
    }

    // Send packet
    *curtime += PACKET_INTERVAL;
    RF_cmdPropTx.startTime = *curtime;
    RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, RF_EventCmdDone);
}

static bool handleReceiveMode(uint32_t *lastRxTime) {
    // Post RX command
    RF_CmdHandle rxCmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx, RF_PriorityNormal, NULL, RF_EventRxEntryDone);
    RF_EventMask result = RF_pendCmd(rfHandle, rxCmdHandle, RF_EventRxEntryDone);

    if (result & RF_EventRxEntryDone) {
        // Update LEDs for receiving mode
        GPIO_write(CONFIG_GPIO_GLED, CONFIG_GPIO_LED_OFF);
        GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_ON);

        // Process the received packet
        processReceivedPacket();

        // Forward the packet
        forwardReceivedPacket();

        // Handle continuous receiving mode for hold time
        *lastRxTime = RF_getCurrentTime();
        while (RF_getCurrentTime() - *lastRxTime < RECEIVING_HOLD_TIME) {
            RF_EventMask result = RF_pendCmd(rfHandle, rxCmdHandle, RF_EventRxEntryDone | RF_EventLastCmdDone);
            if (result & RF_EventRxEntryDone) {
                *lastRxTime = RF_getCurrentTime();
                GPIO_write(CONFIG_GPIO_GLED, CONFIG_GPIO_LED_OFF);
                GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_ON);

                // Process the received packet during hold time
                currentDataEntry = RFQueue_getDataEntry();
                packetLength = *(uint8_t *)(&(currentDataEntry->data));

                if (packetLength > PAYLOAD_LENGTH) {
                    packetLength = PAYLOAD_LENGTH;  // Prevent buffer overflow
                }

                packetDataPointer = (uint8_t *)(&(currentDataEntry->data) + 1);
                memcpy(rxPacket, packetDataPointer, (packetLength + 1));
                RFQueue_nextEntry();

                // Forward during hold time
                memcpy(txPacket, rxPacket, packetLength);
                RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, RF_EventCmdDone);
            }
        }
        return true;
    }
    return false;
}

static void processReceivedPacket(void) {
    // Retrieve packet
    currentDataEntry = RFQueue_getDataEntry();
    packetLength = *(uint8_t *)(&(currentDataEntry->data));
    packetDataPointer = (uint8_t *)(&(currentDataEntry->data) + 1);
    memcpy(rxPacket, packetDataPointer, (packetLength + 1));
    RFQueue_nextEntry();

    // Extract sender ID
    uint8_t senderId = rxPacket[0];

    // Update LCD if needed

    // Extract ADPCM payload
    uint8_t *adpcmData = &rxPacket[3];
    uint16_t adpcmDataSize = packetLength - 3;

    // Get a buffer to write PCM samples into
    I2S_Transaction *transaction = (I2S_Transaction *)List_get(&i2sWriteList);
    if (transaction != NULL) {
        int numSamples = adpcmDataSize * 2;
        int16_t *audioBuffer = (int16_t *)transaction->bufPtr;

        // Ensure we don't overrun the buffer
        if (numSamples > (transaction->bufSize / sizeof(int16_t))) {
            numSamples = (transaction->bufSize / sizeof(int16_t));
            numSamples = (numSamples / 2) * 2;
        }

        // Decode to PCM
        adpcm_decode_buffer(&adpcmDecState, adpcmData, audioBuffer, numSamples);
        transaction->bufSize = numSamples * sizeof(int16_t);

        if (!i2sWriteRunning) {
            I2S_startWrite(i2sHandle);
            i2sWriteRunning = true;
        }
        // Add to write list for playback
        List_put(&i2sWriteList, (List_Elem *)transaction);
        if (currentLCDState != LCD_RECEIVING) {
            updateLCD(LCD_RECEIVING, senderId);
        }

        // Start I2S write if not already running

    }
}

static void forwardReceivedPacket(void) {
    memcpy(txPacket, rxPacket, packetLength);
    RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx, RF_PriorityNormal, NULL, RF_EventCmdDone);
}

static void resetRFConnection(RF_Params *rfParams) {
    RF_yield(rfHandle);   // Release RF resources
    RF_close(rfHandle);   // Close RF to reset internal state

    // Reinitialize RF
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, rfParams);
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
}
