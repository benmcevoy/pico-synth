#include "../include/waveform.h"

#include <math.h>

#include "../include/noise.h"

static float _sampleRate = 0.f;

static float read_from_wt(Voice_t* voice, float* waveTable) {
    uint indexBelow = floorf(voice->waveTableReadPointer);
    uint indexAbove = indexBelow + 1;

    if (indexAbove >= WAVE_TABLE_LENGTH) indexAbove -= WAVE_TABLE_LENGTH;

    float fractionAbove = voice->waveTableReadPointer - indexBelow;
    float fractionBelow = 1.f - fractionAbove;

    float value = fractionBelow * waveTable[indexBelow] +
                  fractionAbove * waveTable[indexAbove];

    voice->waveTableReadPointer += WAVE_TABLE_LENGTH * voice->wavetableStride;

    while (voice->waveTableReadPointer >= WAVE_TABLE_LENGTH)
        voice->waveTableReadPointer -= WAVE_TABLE_LENGTH;

    return value;
}

static float square(Voice_t* voice) {
    float value = (voice->waveTablePhase < M_PI) ? 1.f : -1.f;

    voice->waveTablePhase += TWO_PI * voice->wavetableStride;

    if (voice->waveTablePhase > TWO_PI) voice->waveTablePhase -= TWO_PI;

    return value;
}

static float noise(Voice_t* voice) { return synth_noise_next_prng(); }

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
