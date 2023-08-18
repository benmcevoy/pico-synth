#ifndef SYNTH_AUDIOCONTEXT_
#define SYNTH_AUDIOCONTEXT_

#include <stdbool.h>
#include <stdlib.h>

#include "fixedpoint.h"

// if sample rate is greater than 32767 it will not fit in a signed fix16, so
// shift the decimal point to resolve this
// #define SAMPLE_RATE_SCALE_FACTOR 1
// FIX16_SCALED_SAMPLE_RATE is (SAMPLE_RATE >> SAMPLE_RATE_SCALE_FACTOR) as
// fix16 e.g << 16

// 32767 is a fine sample rate anyway.

#define SAMPLE_RATE 32767
#define FIX16_SAMPLE_RATE 2147418112
#define BUFFER_LENGTH 64
#define VOICES_LENGTH 2

// TODO: this is not a reasonable place to park this code
static fix16 lerp(fix16 fraction, fix16 start, fix16 end) {
    return fraction >= FIX16_ONE ? end
                                 : start + multfix16(fraction, (end - start));
}

typedef enum Waveform { SINE = 0, SQUARE, SAW, TRIANGLE, NOISE } Waveform_t;
typedef enum EnvelopeState {
    OFF = 0,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
} EnvelopeState_t;

typedef struct Envelope {
    bool triggerAttack;
    EnvelopeState_t state;
    fix16 elapsed;
    fix16 duration;
    fix16 envelope;

    fix16 attack;
    fix16 decay;
    fix16 sustain;
    fix16 release;
} Envelope_t;

typedef struct Voice {
    fix16 detune;
    fix16 frequency;
    Waveform_t waveform;
    fix16 wavetableStride;
    fix16 waveTableReadPointer;
    fix16 waveTablePhase;
    fix16 gain;
} Voice_t;

typedef struct Tempo {
    uint32_t durationInSamples;
    bool isBeat;
} Tempo_t;

typedef struct AudioContext {
    uint16_t* audioOut;
    size_t samplesElapsed;
    fix16 gain;
    Tempo_t tempo;
    uint16_t delay;
    fix16 delayGain;

    Voice_t voices[VOICES_LENGTH];

    Envelope_t envelope;
} AudioContext_t;

static void synth_audiocontext_set_wavetable_stride(Voice_t* voice) {
    voice->wavetableStride =
        divfix16(multfix16(voice->frequency, voice->detune), FIX16_SAMPLE_RATE);
}

static fix16 synth_audiocontext_to_duration(float value) {
    return multfix16(float2fix16(value), FIX16_SAMPLE_RATE);
}

#endif
