#include "../include/filter.h"

#include <math.h>

static uint32_t _sampleRate = 0;
static float n1, n2, o1, o2, b0, b1, b2, a1, a2 = 0;

// Calculate the filter coefficients based on the given parameters
// Borrows code from the Bela Biquad library, itself based on code by
// Nigel Redmon
void synth_filter_calculate_coefficients(float cutOffFrequency,
                                         float resonance) {
    float k = tanf(M_PI * cutOffFrequency / _sampleRate);
    float norm = 1.f / (1.f + k / resonance + k * k);

    b0 = k * k * norm;
    b1 = 2.f * b0;
    b2 = b0;
    a1 = 2.f * (k * k - 1.f) * norm;
    a2 = (1.f - k / resonance + k * k) * norm;
}

float synth_filter_low_pass(float sample) {
    float n = 0.5f * sample;
    float o = b0 * n + b1 * n1 + b2 * n2 - a1 * o1 - a2 * o2;

    n2 = n1;
    n1 = n;
    o2 = o1;
    o1 = o;

    return o;
}

void synth_filter_init(uint32_t sampleRate, float cutOffFrequency,
                       float resonance) {
    _sampleRate = sampleRate;
    synth_filter_calculate_coefficients(cutOffFrequency, resonance);
}