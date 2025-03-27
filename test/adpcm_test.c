/*
 * adpcm_test.c
 *
 *  Created on: Feb 27, 2025
 *      Author: colec
 */

#include <stdio.h>
#include <stdlib.h>
#include "adpcm-lib.h"

int main() {
    // 1. Initialize ADPCM state
    adpcm_state_t state;
    adpcm_init(&state);

    // 2. Define a small test audio input buffer (a simple sine wave for ~0.1 seconds at 8kHz)
    #define NUM_SAMPLES 800  // 8kHz * 0.1s = 80 samples. Let's make it 800 for more data to see.
    short originalAudio[NUM_SAMPLES];
    for (int i = 0; i < NUM_SAMPLES; i++) {
        // Generate a simple sine wave (frequency ~ 500Hz)
        originalAudio[i] = (short)(16000 * sin(2.0 * M_PI * 500.0 * i / 8000.0)); // Amplitude scaled to ~ +/- 16000
    }

    // 3. Allocate buffers for encoded and decoded data
    // ADPCM 4:1 compression means encoded data will be ~1/4 the size of original sample data (in bytes)
    int encodedBufferSize = NUM_SAMPLES / 2; // Each encoded byte represents 2 samples (4 bits per sample)
    char encodedData[encodedBufferSize];
    short decodedAudio[NUM_SAMPLES];

    // 4. Encode the original audio
    adpcm_coder(&state, originalAudio, encodedData, NUM_SAMPLES / 2); // 'NUM_SAMPLES / 2' is the number of 4-byte blocks to process

    // 5. Decode the encoded data
    adpcm_decoder(&state, encodedData, decodedAudio, NUM_SAMPLES / 2);

    // 6. Print the first few original and decoded samples to the UART console for verification
    printf("ADPCM Encoding/Decoding Test:\n");
    printf("-----------------------------\n");
    printf("First 20 Original Samples:     First 20 Decoded Samples:\n");
    for (int i = 0; i < 20; i++) {
        printf("%6d ", originalAudio[i]);
        if (i == 19) printf(" | "); else printf(", ");
        printf("%6d", decodedAudio[i]);
        if (i == 19) printf(" |\n"); else printf(", ");
    }
    printf("\n...(and so on for %d samples)...\n", NUM_SAMPLES);
    printf("\nTest Completed. Check if decoded samples roughly resemble original.\n");

    return 0;
}


