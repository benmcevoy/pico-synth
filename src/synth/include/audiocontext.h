#ifndef SYNTH_AUDIOCONTEXT_
#define SYNTH_AUDIOCONTEXT_

#include <stdlib.h>
#include "pico/stdlib.h"

#define BUFFER_LENGTH 512

enum waveform {
    SINE = 0,
    SQUARE,
    SAW,
    TRIANGLE,
    NOISE
};

typedef struct Voice
{
    float frequency;
    enum waveform waveform;
} Voice_t;

typedef struct AudioContext
{
    unsigned short *AudioOut;
    uint64_t SamplesElapsed;
    float SampleRate;
    Voice_t Voice;
    float Volume;
} AudioContext_t;

#endif 