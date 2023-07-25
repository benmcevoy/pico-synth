#include "../include/waveform.h"

#include <math.h>

static float _sampleRate = 0.f;


static float read_from_wt(Voice_t* voice, float* waveTable) {
    uint indexBelow = floorf(voice->envelopeReadPointer);
    uint indexAbove = indexBelow + 1;

    if (indexAbove >= WAVE_TABLE_LENGTH) indexAbove -= WAVE_TABLE_LENGTH;

    float fractionAbove = voice->envelopeReadPointer - indexBelow;
    float fractionBelow = 1.f - fractionAbove;

    float value = fractionBelow * waveTable[indexBelow] +
                  fractionAbove * waveTable[indexAbove];

    voice->envelopeReadPointer +=
        WAVE_TABLE_LENGTH * (voice->frequency * voice->detune) / _sampleRate;

    while (voice->envelopeReadPointer >= WAVE_TABLE_LENGTH) voice->envelopeReadPointer -= WAVE_TABLE_LENGTH;

    return value;
}

static float square(Voice_t* voice) {
    float value = (voice->envelopePhase < M_PI) ? 1.f : -1.f;

    voice->envelopePhase += TWO_PI * (voice->frequency * voice->detune) / _sampleRate;

    if (voice->envelopePhase > TWO_PI) voice->envelopePhase -= TWO_PI;

    return value;
}

static float noise(Voice_t* voice) {
    // random between -1..1
    return (rand() / (float)(RAND_MAX)) * 2.f - 1.f;
}

float synth_waveform_sample(Voice_t* voice) {
    switch (voice->waveform) {
        case SINE:
            return read_from_wt(voice, SineWaveTable);
        case SQUARE:
            return square(voice);
        case SAW:
            return read_from_wt(voice, SawWaveTable);
        case TRIANGLE:
            return read_from_wt(voice, TriangleWaveTable);
        case NOISE:
            return noise(voice);

        default:
            return 0.f;
    }
}

void synth_waveform_init(float sampleRate) { _sampleRate = sampleRate; }