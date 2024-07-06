#ifndef SYNTH_CONTROLLER_
#define SYNTH_CONTROLLER_

#include <stdint.h>

#include "audiocontext.h"

typedef enum {
  ACTION_NONE = 0,

  ACTION_DELAY,
  ACTION_DELAY_FEEDBACK,

  ACTION_DETUNE,
  ACTION_WIDTH, 

  ACTION_CUTOFF,
  ACTION_RESONANCE,

  ACTION_VOICE0_WAVEFORM,
  ACTION_VOICE1_WAVEFORM,
  // voice0/1 tune? semitones

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

void synth_controller_init();

void synth_controller_task(audio_context_t* context);

#endif