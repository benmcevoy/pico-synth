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
    double frequency;
    enum waveform waveform;
    double amplitude;

} Voice_t;

typedef struct AudioContext
{
    unsigned short *AudioOut;
    volatile uint64_t SamplesElapsed;
    double SampleRate;
    Voice_t Voice;
} AudioContext_t;



#endif 