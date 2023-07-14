#ifndef SYNTH_AUDIOCONTEXT_
#define SYNTH_AUDIOCONTEXT_

#include <stdlib.h>
#include "pico/stdlib.h"

#define BUFFER_SIZE 512

typedef struct AudioContext
{
    unsigned short (*AudioOut)[BUFFER_SIZE];
    uint64_t SamplesElapsed;
    uint SampleRate;
} AudioContext_t;

#endif 