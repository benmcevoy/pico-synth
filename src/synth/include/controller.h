#ifndef SYNTH_CONTROLLER_
#define SYNTH_CONTROLLER_

#include <stdint.h>
#include "audiocontext.h"

typedef struct Control {
    uint8_t channel;
    uint16_t value;
} control_t;

extern control_t* controls;

void synth_controller_init(uint16_t sampleRate);

void synth_controller_task(AudioContext_t* context);

#endif