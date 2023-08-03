#ifndef SYNTH_AUDIOCONTEXT_
#define SYNTH_AUDIOCONTEXT_

#include <stdlib.h>

#include "fixedpoint.h"
#include "pico/stdlib.h"

#define BUFFER_LENGTH 512
#define VOICES_LENGTH 1

typedef enum Waveform { SINE = 0, SQUARE, SAW, TRIANGLE, NOISE } Waveform_t;
typedef enum EnvelopeState {
    OFF = 0,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
} EnvelopeState_t;

typedef struct Voice {
    float detune;
    float frequency;
    Waveform_t waveform;
    fix16 wavetableStride;
    fix16 waveTableReadPointer;
    fix16 waveTablePhase;
} Voice_t;

typedef struct AudioContext {
    uint16_t* audioOut;
    size_t samplesElapsed;
    uint32_t sampleRate;
    Voice_t voices[VOICES_LENGTH];
    fix16 volume;

    bool filterEnabled;
    float filterCutoff;
    float filterResonance;

    bool triggerAttack;
    fix16 envelope;
    fix16 attack;
    fix16 decay;
    fix16 sustain;
    fix16 release;
} AudioContext_t;

static void synth_audiocontext_set_wavetable_stride(Voice_t* voice, uint32_t sampleRate) {
    voice->wavetableStride = float2fix16(voice->frequency * voice->detune / sampleRate);
}

#endif