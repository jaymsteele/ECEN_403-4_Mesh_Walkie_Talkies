#include <stdint.h>
#include <stdio.h>

#define ADPCM_STEP_SIZE_TABLE_SIZE 89
#define ADPCM_INDEX_TABLE_SIZE 16

// Step size table for IMA ADPCM
static const int16_t stepSizeTable[ADPCM_STEP_SIZE_TABLE_SIZE] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 30,
    33, 35, 38, 41, 45, 49, 53, 57, 60, 65, 70, 74, 79, 84, 90, 95,
    102, 108, 115, 122, 130, 137, 145, 153, 162, 171, 180, 189, 199, 208,
    218, 227, 237, 247, 257, 267, 277, 288, 298, 309, 320, 331, 342, 353,
    364, 375, 386, 398, 409, 421, 432, 444, 456, 467, 479, 491, 503, 515,
    527, 539, 551, 563, 575, 587, 599, 611, 623, 635, 647, 659, 671, 683,
    695, 707, 719, 731, 743, 755, 767, 779, 791, 803, 815, 827, 839, 851,
    863, 875, 887, 899, 911, 923, 935, 947, 959, 971, 983, 995, 1007, 1019
};

// Index table for IMA ADPCM
static const int8_t indexTable[ADPCM_INDEX_TABLE_SIZE] = {
    -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8
};

// ADPCM state structure
typedef struct {
    int16_t previousSample;
    int8_t index;
} ADPCMState;

// Initialize ADPCM state
void initADPCMState(ADPCMState *state) {
    state->previousSample = 0;
    state->index = 0;
}

// ADPCM encoding: takes 16-bit PCM sample and outputs 4-bit ADPCM code
int8_t encodeIMA_ADPCM(ADPCMState *state, int16_t pcmSample) {
    int step = stepSizeTable[state->index];
    int diff = pcmSample - state->previousSample;
    int stepChange = step >> 3;
    int code = 0;

    if (diff < 0) {
        code = 8;
        diff = -diff;
    }

    if (diff >= step) {
        code |= 4;
        diff -= step;
    }
    if (diff >= (step >> 1)) {
        code |= 2;
        diff -= (step >> 1);
    }
    if (diff >= (step >> 2)) {
        code |= 1;
    }

    // Update the previous sample and index
    state->previousSample += (code & 8 ? -1 : 1) * stepChange;
    if (state->previousSample < -32768) state->previousSample = -32768;
    if (state->previousSample > 32767) state->previousSample = 32767;

    state->index += indexTable[code];
    if (state->index < 0) state->index = 0;
    if (state->index > ADPCM_INDEX_TABLE_SIZE - 1) state->index = ADPCM_INDEX_TABLE_SIZE - 1;

    return (int8_t) code;
}

// ADPCM decoding: takes 4-bit ADPCM code and outputs 16-bit PCM sample
int16_t decodeIMA_ADPCM(ADPCMState *state, int8_t code) {
    int step = stepSizeTable[state->index];
    int diff = step >> 3;

    if (code & 4) diff += step;
    if (code & 2) diff += step >> 1;
    if (code & 1) diff += step >> 2;

    if (code & 8) diff = -diff;

    // Update the previous sample and index
    state->previousSample += diff;
    if (state->previousSample < -32768) state->previousSample = -32768;
    if (state->previousSample > 32767) state->previousSample = 32767;

    state->index += indexTable[code];
    if (state->index < 0) state->index = 0;
    if (state->index > ADPCM_INDEX_TABLE_SIZE - 1) state->index = ADPCM_INDEX_TABLE_SIZE - 1;

    return state->previousSample;
}

int mainTest() {
    System_printf("Start of imaTest function.\n");
    System_flush();

    // Example PCM data
    int16_t pcmData[8] = { 1000, 2000, -3000, 4000, -5000, 6000, -7000, 8000 };

    // Initialize ADPCM state
    ADPCMState adpcmState;
    initADPCMState(&adpcmState);

    // Encode PCM to ADPCM
    printf("Encoding PCM to ADPCM:\n");
    for (int i = 0; i < 8; i++) {
        int8_t adpcmCode = encodeIMA_ADPCM(&adpcmState, pcmData[i]);
        printf("PCM: %d => ADPCM: 0x%X\n", pcmData[i], adpcmCode);
    }

    // Decode ADPCM back to PCM
    printf("\nDecoding ADPCM back to PCM:\n");
    for (int i = 0; i < 8; i++) {
        int16_t pcmSample = decodeIMA_ADPCM(&adpcmState, (int8_t)i);
        printf("ADPCM: 0x%X => PCM: %d\n", i, pcmSample);
    }

    return 0;
}
