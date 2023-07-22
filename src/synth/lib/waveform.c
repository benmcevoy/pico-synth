#include "../include/waveform.h"

#include <math.h>

float _readPointer = 0;
float _phase = 0;

float read_from_wt(AudioContext_t* context, float* waveTable) {
    uint indexBelow = floorf(_readPointer);
    uint indexAbove = indexBelow + 1;

    if (indexAbove >= WAVE_TABLE_LENGTH) indexAbove -= WAVE_TABLE_LENGTH;

    float fractionAbove = _readPointer - indexBelow;
    float fractionBelow = 1.f - fractionAbove;

    float value = fractionBelow * waveTable[indexBelow] +
                  fractionAbove * waveTable[indexAbove];

    _readPointer +=
        WAVE_TABLE_LENGTH * context->Voice.frequency / context->SampleRate;

    while (_readPointer >= WAVE_TABLE_LENGTH) _readPointer -= WAVE_TABLE_LENGTH;

    return value;
}

float square(AudioContext_t* context) {
    float value = (_phase < M_PI) ? 1.f : -1.f;

    _phase += TWO_PI * context->Voice.frequency / context->SampleRate;

    if (_phase > TWO_PI) _phase -= TWO_PI;

    return value;
}

float noise(AudioContext_t* context) {
    // random between -1..1
    return (rand() / (float)(RAND_MAX)) * 2.f - 1.f;
}

float synth_waveform_sample(AudioContext_t* context) {
    switch (context->Voice.waveform) {
        case SINE:
            return read_from_wt(context, SineWaveTable);
        case SQUARE:
            return square(context);
        case SAW:
            return read_from_wt(context, SawWaveTable);
        case TRIANGLE:
            return read_from_wt(context, TriangleWaveTable);
        case NOISE:
            return noise(context);

        default:
            return 0.f;
    }
}