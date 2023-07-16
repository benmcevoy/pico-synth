#ifndef SYNTH_AUDIOCONTEXT_
#define SYNTH_AUDIOCONTEXT_

#include <stdlib.h>
#include "pico/stdlib.h"

#define BUFFER_LENGTH 512

typedef struct AudioContext
{
    unsigned short *AudioOut;
    uint64_t SamplesElapsed;
    double SampleRate;
} AudioContext_t;

#endif 