#include "../include/filter.h"

#include <math.h>

static float _sampleRate = 0.f;
static float n1, n2, o1, o2, b0, b1, b2, a1, a2 = 0;

// Calculate the filter coefficients based on the given parameters
// Borrows code from the Bela Biquad library, itself based on code by
// Nigel Redmon
void coefficients(float f, float q) {
    float k = tanf(M_PI * f / _sampleRate);
    float norm = 1.f / (1.f + k / q + k * k);

    b0 = k * k * norm;
    b1 = 2.f * b0;
    b2 = b0;
    a1 = 2.f * (k * k - 1.f) * norm;
    a2 = (1.f - k / q + k * k) * norm;
}

float synth_filter_low_pass(float cutOffFrequency, float resonance,
                            float sample) {
    coefficients(cutOffFrequency, resonance);

    float n = 0.5f * sample;
    float o = b0 * n + b1 * n1 + b2 * n2 - a1 * o1 - a2 * o2;

    n2 = n1;
    n1 = n;
    o2 = o1;
    o1 = o;

    return o;
}

void synth_filter_init(float sampleRate) { _sampleRate = sampleRate; }