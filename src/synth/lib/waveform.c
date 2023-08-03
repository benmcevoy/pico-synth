#include "../include/waveform.h"

#include <math.h>

#define FIX16_NEGATIVE1 -65536

// TODO: should be defined as fix16 constants instead
static fix16 _waveTableLength = 0;
static fix16 _pi = 0;
static fix16 _twoPi = 0;
static uint16_t _seed = 17;
static int a = 1;
static int b = 5;
static int c = 2;

static fix16 read_from_wt(Voice_t* voice, fix16* waveTable) {
    int16_t indexBelow = fix2int16(voice->waveTableReadPointer);
    int16_t indexAbove = indexBelow + 1;

    if (indexAbove >= WAVE_TABLE_LENGTH) indexAbove -= WAVE_TABLE_LENGTH;

    fix16 fractionAbove = voice->waveTableReadPointer - int2fix16(indexBelow);
    fix16 fractionBelow = FIX16_UNIT - fractionAbove;

    fix16 value = multfix16(fractionBelow, waveTable[indexBelow]) +
                  multfix16(fractionAbove, waveTable[indexAbove]);

    voice->waveTableReadPointer +=
        multfix16(_waveTableLength, voice->wavetableStride);

    while (voice->waveTableReadPointer >= _waveTableLength)
        voice->waveTableReadPointer -= _waveTableLength;

    return value;
}

static fix16 square(Voice_t* voice) {
    fix16 value = (voice->waveTablePhase < _pi) ? FIX16_UNIT : FIX16_NEGATIVE1;

    voice->waveTablePhase += multfix16(_twoPi, voice->wavetableStride);

    if (voice->waveTablePhase > _twoPi) voice->waveTablePhase -= _twoPi;

    return value;
}

// https://en.wikipedia.org/wiki/Xorshift
static fix16 noise(Voice_t* voice) {
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
            return noise(voice);

        default:
            return 0;
    }
}

void synth_waveform_init() {
    _waveTableLength = int2fix16(WAVE_TABLE_LENGTH);
    _pi = float2fix16(M_PI);
    _twoPi = float2fix16(M_PI * 2.f);
}