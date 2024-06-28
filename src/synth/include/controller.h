#ifndef SYNTH_CONTROLLER_
#define SYNTH_CONTROLLER_

#include <stdint.h>
#include "audiocontext.h"

typedef enum { 
    ACTION_NONE = 0, 
    
    ACTION_DELAY, 
    ACTION_DELAYGAIN, 
    
    ACTION_DETUNE, 
    ACTION_WIDTH, 
    
    ACTION_LEVEL, 
    ACTION_ATTACK,
    ACTION_DECAY,
    ACTION_SUSTAIN,
    ACTION_RELEASE
     } control_action_t;

typedef struct {
    uint8_t channel;
    uint16_t value;
    control_action_t action;
} control_t;

extern control_t* controls;

void synth_controller_init(uint16_t sampleRate);

void synth_controller_task(AudioContext_t* context);

#endif