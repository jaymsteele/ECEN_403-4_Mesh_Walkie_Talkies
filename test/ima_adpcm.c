/* ima_adpcm.c - IMA ADPCM Encoder/Decoder Implementation */
#include "ima_adpcm.h"

/* Step size table */
static const int step_table[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31,
    34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143,
    157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544,
    598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707,
    1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871,
    5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

/* Index table for step size adjustments */
static const int index_table[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8
};

/* Initialize ADPCM state */
void adpcm_init(ADPCM_State *state) {
    state->prev_sample = 0;
    state->step_index = 0;
}

/* Encode a single PCM sample to ADPCM */
uint8_t adpcm_encode_sample(int16_t sample, ADPCM_State *state) {
    int step = step_table[state->step_index];
    int diff = sample - state->prev_sample;
    uint8_t code = 0;

    if (diff < 0) {
        code = 8;
        diff = -diff;
    }

    if (diff >= step) { code |= 4; diff -= step; }
    if (diff >= (step >> 1)) { code |= 2; diff -= (step >> 1); }
    if (diff >= (step >> 2)) { code |= 1; }

    int delta = step >> 3;
    if (code & 1) delta += step >> 2;
    if (code & 2) delta += step >> 1;
    if (code & 4) delta += step;
    if (code & 8) delta = -delta;

    state->prev_sample += delta;
    if (state->prev_sample > 32767) state->prev_sample = 32767;
    if (state->prev_sample < -32768) state->prev_sample = -32768;

    state->step_index += index_table[code];
    if (state->step_index < 0) state->step_index = 0;
    if (state->step_index > 88) state->step_index = 88;

    return code;
}

/* Decode a single ADPCM sample to PCM */
int16_t adpcm_decode_sample(uint8_t adpcm, ADPCM_State *state) {
    int step = step_table[state->step_index];
    int delta = step >> 3;

    if (adpcm & 1) delta += step >> 2;
    if (adpcm & 2) delta += step >> 1;
    if (adpcm & 4) delta += step;
    if (adpcm & 8) delta = -delta;

    state->prev_sample += delta;
    if (state->prev_sample > 32767) state->prev_sample = 32767;
    if (state->prev_sample < -32768) state->prev_sample = -32768;

    state->step_index += index_table[adpcm];
    if (state->step_index < 0) state->step_index = 0;
    if (state->step_index > 88) state->step_index = 88;

    return state->prev_sample;
}

/* Encode a buffer of PCM samples to ADPCM */
void adpcm_encode_buffer(int16_t *pcm, uint8_t *adpcm, int len, ADPCM_State *state) {
    for (int i = 0; i < len; i++) {
        adpcm[i] = adpcm_encode_sample(pcm[i], state);
    }
}

/* Decode a buffer of ADPCM samples to PCM */
void adpcm_decode_buffer(uint8_t *adpcm, int16_t *pcm, int len, ADPCM_State *state) {
    for (int i = 0; i < len; i++) {
        pcm[i] = adpcm_decode_sample(adpcm[i], state);
    }
}
