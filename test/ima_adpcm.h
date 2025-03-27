/* ima_adpcm.h - IMA ADPCM Encoder/Decoder Header */
#ifndef IMA_ADPCM_H
#define IMA_ADPCM_H

#include <stdint.h>

/* ADPCM state structure */
typedef struct {
    int16_t prev_sample;  // Previous output sample
    int step_index;       // Step table index
} ADPCM_State;

/* Function prototypes */
void adpcm_init(ADPCM_State *state);
uint8_t adpcm_encode_sample(int16_t sample, ADPCM_State *state);
int16_t adpcm_decode_sample(uint8_t adpcm, ADPCM_State *state);
void adpcm_encode_buffer(int16_t *pcm, uint8_t *adpcm, int len, ADPCM_State *state);
void adpcm_decode_buffer(uint8_t *adpcm, int16_t *pcm, int len, ADPCM_State *state);

#endif /* IMA_ADPCM_H */
