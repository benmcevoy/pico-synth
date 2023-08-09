#ifndef SYNTH_AUDIOCONTEXT_
#define SYNTH_AUDIOCONTEXT_

#include <stdlib.h>

#include "fixedpoint.h"
#include "pico/stdlib.h"

#define SR_SCALE_FACTOR 2
#define BUFFER_LENGTH 64
#define VOICES_LENGTH 3

typedef enum Waveform { SINE = 0, SQUARE, SAW, TRIANGLE, NOISE } Waveform_t;
typedef enum EnvelopeState {
    OFF = 0,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
} EnvelopeState_t;

typedef struct Voice {
    fix16 detune;
    fix16 frequency;
    Waveform_t waveform;
    fix16 wavetableStride;
    fix16 waveTableReadPointer;
    fix16 waveTablePhase;
} Voice_t;

typedef struct AudioContext {
    uint16_t* audioOut;
    size_t samplesElapsed;
    uint16_t sampleRate;
    Voice_t voices[VOICES_LENGTH];
    fix16 volume;

    uint16_t delay;
    fix16 delayGain;

    bool triggerAttack;
    fix16 envelope;
    fix16 attack;
    fix16 decay;
    fix16 sustain;
    fix16 release;
} AudioContext_t;

static void synth_audiocontext_set_wavetable_stride(Voice_t* voice,
                                                    uint16_t sampleRate) {
    // samplerate exceeds signed 16 bit so do some twiddling
    voice->wavetableStride =
        divfix16(multfix16(voice->frequency, voice->detune),
                 int2fix16(sampleRate >> SR_SCALE_FACTOR)) >>
        SR_SCALE_FACTOR;
}

#endif
