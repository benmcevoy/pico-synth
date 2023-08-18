#include "../include/waveform.h"

#include <math.h>

static uint16_t _seed = 17;
static int a = 1;
static int b = 5;
static int c = 2;

static fix16 read_from_wt(Voice_t* voice, fix16* waveTable) {
    int16_t indexBelow = fix2int16(voice->waveTableReadPointer);
    int16_t indexAbove = indexBelow + 1;

    if (indexAbove >= WAVE_TABLE_LENGTH) indexAbove -= WAVE_TABLE_LENGTH;

    fix16 fraction = voice->waveTableReadPointer - int2fix16(indexBelow);
    fix16 value =
        lerp(fraction, waveTable[indexBelow], waveTable[indexAbove]);

    voice->waveTableReadPointer +=
        multfix16(FIX16_WAVE_TABLE_LENGTH, voice->wavetableStride);

    while (voice->waveTableReadPointer >= FIX16_WAVE_TABLE_LENGTH)
        voice->waveTableReadPointer -= FIX16_WAVE_TABLE_LENGTH;

    return value;
}

static fix16 square(Voice_t* voice) {
    fix16 value =
        (voice->waveTablePhase < FIX16_PI) ? FIX16_ONE : FIX16_NEGATIVE_ONE;

    voice->waveTablePhase += multfix16(FIX16_TWOPI, voice->wavetableStride);

    if (voice->waveTablePhase > FIX16_TWOPI)
        voice->waveTablePhase -= FIX16_TWOPI;

    return value;
}

// https://en.wikipedia.org/wiki/Xorshift
fix16 synth_waveform_noise() {
    _seed ^= (_seed << a);
    _seed ^= (_seed >> b);
    _seed ^= (_seed << c);
    // just return it as it's random anyways...
    return _seed;
}

fix16 synth_waveform_sample(Voice_t* voice) {
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
            return synth_waveform_noise();

        default:
            return 0;
    }
}
