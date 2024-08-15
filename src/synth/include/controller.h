#ifndef SYNTH_CONTROLLER_
#define SYNTH_CONTROLLER_

#include <stdint.h>

#include "audiocontext.h"
#include "hardware/spi.h"

typedef enum {
  CONTROL_ACTION_NONE = 0,

  CONTROL_ACTION_DELAY_TIME,
  CONTROL_ACTION_DELAY_ENABLED,
  CONTROL_ACTION_DELAY_FEEDBACK,
  CONTROL_ACTION_DELAY_FEEDBACK_INFINITE,
  CONTROL_ACTION_DELAY_DRY_WET_MIX,

  CONTROL_ACTION_DETUNE,
  CONTROL_ACTION_WIDTH,

  CONTROL_ACTION_FILTER_CUTOFF,
  CONTROL_ACTION_FILTER_ENABLED,
  CONTROL_ACTION_FILTER_RESONANCE,

  CONTROL_ACTION_VOICE0_WAVEFORM,
  CONTROL_ACTION_VOICE1_WAVEFORM,

  // voice0/1 tune? semitones
  CONTROL_ACTION_LEVEL,
  CONTROL_ACTION_ATTACK,
  CONTROL_ACTION_DECAY,
  CONTROL_ACTION_SUSTAIN,
  CONTROL_ACTION_RELEASE
} control_action_t;

typedef enum {
  CONTROL_TYPE_ANALOG = 0,
  CONTROL_TYPE_TOGGLE,
  CONTROL_TYPE_MOMENTRY
} control_type_t;

typedef struct {
  spi_inst_t* spi;
  uint8_t chip_select_pin;
  uint8_t clock_pin;
  uint8_t tx_pin;
  uint8_t rx_pin;
  uint actual_baud_rate;
} spi_device_t;

typedef struct {
  uint8_t channel;
  uint16_t value;
  control_action_t action;
  control_type_t control_type;
  spi_device_t* spi_device;
} control_t;

extern control_t* controls;

void synth_controller_init();

void synth_controller_task(audio_context_t* context);

#endif