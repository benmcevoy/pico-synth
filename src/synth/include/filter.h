#ifndef SYNTH_FILTER_
#define SYNTH_FILTER_

#include <stdlib.h>

#include "fixedpoint.h"

static float b0;
static float b1;
static float b2;
static float s1;
static float s2;

#define NTICK 64

fix16 synth_filter(uint16_t* input, float cutoff, float resonance) {
//   for (uint8_t i = 0; i < NTICK; i++) {
//     uint16_t x = input[i];
//     input[i] = b0 * x + b1 * s1 + b2 * s2;
//     s2 = s1;
//     s1 = x;
//   }
}

#endif