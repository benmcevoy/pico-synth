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
    float attack;
    float decay;
    float sustain;
    float release;
    bool triggerAttack;
    EnvelopeState_t envelopeState;
    uint32_t envelopeCounter;
    uint32_t envelopeDuration;
    float envelopeReadPointer;
    float envelopePhase;
    float detune;
    float wavetableStride;
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
} AudioContext_t;

#endif