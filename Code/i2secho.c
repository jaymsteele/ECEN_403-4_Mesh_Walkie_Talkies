#include <stdint.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2S.h>
#include <ti/drivers/I2C.h>
#include "lcd.h"
#include "ti_drivers_config.h"
#include "AudioCodec.h"

#define THREADSTACKSIZE 2048
#define SAMPLE_RATE   44100
#define INPUT_OPTION  AudioCodec_MIC_MONO
#define OUTPUT_OPTION AudioCodec_SPEAKER_HP
#define NUMBUFS 10
#define BUFSIZE 256

static sem_t semDataReadyForTreatment;
static sem_t semErrorCallback;
List_List i2sReadList;
List_List treatmentList;
List_List i2sWriteList;

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

volatile bool buttonPressed = false;
volatile bool displayUpdated = false;
volatile bool transmitting = false;
volatile bool i2sStreamingActive = false; // Flag to indicate if I2S is streaming
I2C_Handle i2cHandle;
I2C_Params i2cParams1;

void buttonCallback(uint_least8_t index) {
    bool currentButtonState = (GPIO_read(CONFIG_GPIO_BUTTON_0) == 0);
    if (currentButtonState != buttonPressed) {
        buttonPressed = currentButtonState;
        transmitting = buttonPressed;

        if (transmitting && !i2sStreamingActive) {
            printf("Button pressed, starting transmission\n");
            AudioCodec_micUnmute(AudioCodec_TI_3254, AudioCodec_MIC_MONO);
            AudioCodec_speakerUnmute(AudioCodec_TI_3254, AudioCodec_SPEAKER_HP);
            I2S_startClocks(i2sHandle);
            I2S_startRead(i2sHandle);
            I2S_startWrite(i2sHandle);
            i2sStreamingActive = true;
        } else if (!transmitting && i2sStreamingActive) {
            printf("Button released, stopping transmission\n");
            AudioCodec_micMute(AudioCodec_TI_3254, AudioCodec_MIC_MONO);
            AudioCodec_speakerMute(AudioCodec_TI_3254, AudioCodec_SPEAKER_HP);
            I2S_stopClocks(i2sHandle);
            i2sStreamingActive = false;
        }
        displayUpdated = false;
    }
}

void displayDefaultMessage(void) {
    printf("Displaying default message\n");
    LCD_clear(i2cHandle);
    LCD_print(i2cHandle, "Push To Talk: W1");
    LCD_setCursor(i2cHandle, 0, 1);
    LCD_print(i2cHandle, "Battery: 100%");
}

void displayTransmittingMessage(void) {
    printf("Displaying transmitting message\n");
    LCD_clear(i2cHandle);
    LCD_print(i2cHandle, "Transmitting...");
}

static void errCallbackFxn(I2S_Handle handle, int_fast16_t status, I2S_Transaction *transactionPtr) {
    printf("Error callback triggered\n");
    sem_post(&semErrorCallback);
}

static void writeCallbackFxn(I2S_Handle handle, int_fast16_t status, I2S_Transaction *transactionPtr) {
    printf("Write callback triggered\n");
    I2S_Transaction *transactionFinished = (I2S_Transaction *)List_prev(&transactionPtr->queueElement);
    if (transactionFinished != NULL) {
        List_remove(&i2sWriteList, (List_Elem *)transactionFinished);
        List_put(&i2sReadList, (List_Elem *)transactionFinished);
    }
}

static void readCallbackFxn(I2S_Handle handle, int_fast16_t status, I2S_Transaction *transactionPtr) {
    printf("Read callback triggered\n");
    I2S_Transaction *transactionFinished = (I2S_Transaction *)List_prev(&transactionPtr->queueElement);
    if (transactionFinished != NULL) {
        List_remove(&i2sReadList, (List_Elem *)transactionFinished);
        List_put(&treatmentList, (List_Elem *)transactionFinished);
        sem_post(&semDataReadyForTreatment);
    }
}

void *echoThread(void *arg0) {
    printf("Initializing audio codec\n");
    uint8_t status = AudioCodec_open();
    if (AudioCodec_STATUS_SUCCESS != status) {
        printf("Error initializing codec\n");
        while (1) {}
    }

    status = AudioCodec_config(AudioCodec_TI_3254, AudioCodec_16_BIT, SAMPLE_RATE, AudioCodec_STEREO, AudioCodec_SPEAKER_HP, AudioCodec_MIC_MONO);
    if (AudioCodec_STATUS_SUCCESS != status) {
        printf("Error configuring codec\n");
        while (1) {}
    }

    AudioCodec_speakerVolCtrl(AudioCodec_TI_3254, AudioCodec_SPEAKER_HP, 100);
    AudioCodec_micVolCtrl(AudioCodec_TI_3254, AudioCodec_MIC_MONO, 75);

    int retc = sem_init(&semDataReadyForTreatment, 0, 0);
    if (retc == -1) {
        printf("Error initializing semaphore\n");
        while (1) {}
    }

    I2S_Params i2sParams;
    I2S_Params_init(&i2sParams);
    i2sParams.samplingFrequency = SAMPLE_RATE;
    i2sParams.fixedBufferLength = BUFSIZE;
    i2sParams.writeCallback = writeCallbackFxn;
    i2sParams.readCallback = readCallbackFxn;
    i2sParams.errorCallback = errCallbackFxn;
    i2sHandle = I2S_open(CONFIG_I2S_0, &i2sParams);
    if (i2sHandle == NULL) {
        printf("Error opening I2S driver\n");
        while (1) {}
    }

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

    printf("Starting audio streaming loop\n");
    while (1) {
        retc = sem_wait(&semDataReadyForTreatment);
        if (retc == -1) {
            printf("Error waiting for semaphore\n");
            while (1) {}
        }

        if (i2sStreamingActive) {
            I2S_Transaction *transactionToTreat = (I2S_Transaction *)List_head(&treatmentList);
            if (transactionToTreat != NULL) {
                printf("Processing audio buffer\n");
                int16_t *buf = transactionToTreat->bufPtr;
                uint16_t numOfSamples = transactionToTreat->bufSize / sizeof(int16_t);
                for (uint16_t n = 0; n < numOfSamples - 2; n += 2) {
                    buf[n] = (buf[n] + buf[n + 2]) / 2;
                }
                List_remove(&treatmentList, (List_Elem *)transactionToTreat);
                List_put(&i2sWriteList, (List_Elem *)transactionToTreat);
            }
        }
    }
}

void *mainThread(void *arg0) {
    printf("Initializing main thread\n");
    pthread_t thread0;
    pthread_attr_t attrs;
    struct sched_param priParam;
    int retc;
    int detachState;

    I2S_init();
    I2C_init();

    I2C_Params i2CParams1;
    I2C_Params_init(&i2cParams1);
    i2cParams1.bitRate = I2C_100kHz;
    i2cHandle = I2C_open(CONFIG_I2C_1, &i2cParams1);
    if (i2cHandle == NULL) {
        printf("I2C Open failed!\n");
        while (1);
    }

    GPIO_init();
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_BOTH_EDGES);
    GPIO_setCallback(CONFIG_GPIO_BUTTON_0, buttonCallback);
    GPIO_enableInt(CONFIG_GPIO_BUTTON_0);

    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    LCD_init(i2cHandle);
    displayDefaultMessage();
    displayUpdated = true;

    pthread_attr_init(&attrs);
    priParam.sched_priority = 1;
    detachState = PTHREAD_CREATE_DETACHED;
    retc = pthread_attr_setdetachstate(&attrs, detachState);
    if (retc != 0) {
        printf("Error setting detach state\n");
        while (1) {}
    }

    pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0) {
        printf("Error setting stack size\n");
        while (1) {}
    }

    retc = pthread_create(&thread0, &attrs, echoThread, NULL);
    if (retc != 0) {
        printf("Error creating thread\n");
        while (1) {}
    }

    retc = sem_init(&semErrorCallback, 0, 0);
    if (retc == -1) {
        printf("Error initializing error callback semaphore\n");
        while (1) {}
    }

    while (1) {
        if (!displayUpdated) {
            if (buttonPressed) {
                displayTransmittingMessage();
            } else {
                displayDefaultMessage();
            }
            displayUpdated = true;
        }
        usleep(50000);
    }

    sem_wait(&semErrorCallback);
    pthread_cancel(&thread0);
    I2S_stopClocks(i2sHandle);
    I2S_close(i2sHandle);
    return NULL;
}
