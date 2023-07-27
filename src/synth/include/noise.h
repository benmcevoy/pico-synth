#ifndef SYNTH_NOISE_
#define SYNTH_NOISE_

#include <stdlib.h>

#include "pico/stdlib.h"

static uint16_t _seed = 17;
static int a = 1;
static int b = 5;
static int c = 2;

// https://en.wikipedia.org/wiki/Xorshift
float synth_noise_next_prng() {
    _seed ^= (_seed << a);
    _seed ^= (_seed >> b);
    _seed ^= (_seed << c);
    // 16 bit PRNG scaled to -1..1
    return _seed / 32768.f - 1.f;
}

float synth_noise_next_random() {
    return (rand() / (float)(RAND_MAX)) * 2.f - 1.f;
}

/* Test a bit. Returns 1 if bit is set. */
static bool bit(long val, uint8_t bitnr) {
    return (val & (1 << bitnr)) ? 1 : 0;
}

// http://www.sidmusic.org/sid/sidtech5.html
static uint32_t reg = 0x7ffff8; /* Initial value of internal register*/
static uint8_t output;
static bool bit22; /* Temp. to keep bit 22 */
static bool bit17; /* Temp. to keep bit 17 */

/* Generate output from noise-waveform */
float synth_noise_next_c64() {
    /* Pick out bits to make output value */
    output = (bit(reg, 22) << 7) | (bit(reg, 20) << 6) | (bit(reg, 16) << 5) |
             (bit(reg, 13) << 4) | (bit(reg, 11) << 3) | (bit(reg, 7) << 2) |
             (bit(reg, 4) << 1) | (bit(reg, 2) << 0);

    /* Save bits used to feed bit 0 */
    bit22 = bit(reg, 22);
    bit17 = bit(reg, 17);

    /* Shift 1 bit left */
    reg = reg << 1;

    /* Feed bit 0 */
    reg = reg | (bit22 ^ bit17);

    // scale 8 bit number to -1..1
    return output / 128.f - 1.f;
}

#endif