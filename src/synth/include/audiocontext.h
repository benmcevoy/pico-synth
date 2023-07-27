#ifndef SYNTH_AUDIOCONTEXT_
#define SYNTH_AUDIOCONTEXT_

#include <stdlib.h>

#include "pico/stdlib.h"

#define BUFFER_LENGTH 512
#define VOICES_LENGTH 2

typedef enum Waveform { SINE = 0, SQUARE, SAW, TRIANGLE, NOISE } Waveform_t;
typedef enum EnvelopeState {
    OFF = 0,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
} EnvelopeState_t;

typedef struct Voice {
    float frequency;
    Waveform_t waveform;
    float detune;
    float wavetableStride;
    float waveTableReadPointer;
    float waveTablePhase;
} Voice_t;

typedef struct AudioContext {
    unsigned short* audioOut;
    size_t samplesElapsed;
    float sampleRate;
    Voice_t voices[VOICES_LENGTH];
    float volume;
    bool filterEnabled;
    float filterCutoff;
    float filterResonance;
    float attack;
    float decay;
    float sustain;
    float release;
    bool triggerAttack;
    float envelope;
} AudioContext_t;

#endif