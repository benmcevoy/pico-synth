#include "../include/controller.h"

#include <stdio.h>

#include "../include/mcp3008.h"

#define spi_clock 2
#define spi_tx 3
#define spi_rx 4
#define spi_cs 5

#define CONTROLS_COUNT 3

static uint16_t _sampleRate = 0;
const uint8_t _threshold = 10;
static mcp3008_t _controller;
control_t* controls;

static inline float normal(uint16_t value) { return value / 1024.f; }
static inline float tofix16(uint16_t value) { return value << 6; }

void synth_controller_init(uint16_t sampleRate) {
  _sampleRate = sampleRate;
  _controller =
      synth_mcp3008_init(spi0, 120000, spi_cs, spi_clock, spi_tx, spi_rx);

  controls = malloc(sizeof(control_t) * CONTROLS_COUNT);

  controls[0] = (control_t){
      .channel = 0,
      .value = 0,
      .action = ACTION_DETUNE,
  };
  controls[1] = (control_t){
      .channel = 1,
      .value = 0,
      .action = ACTION_DELAY,
  };
  controls[2] = (control_t){
      .channel = 2,
      .value = 0,
      .action = ACTION_DELAYGAIN,
  };
}

void synth_controller_task(AudioContext_t* context) {
  // read channels
  for (size_t i = 0; i < CONTROLS_COUNT; i++) {
    uint16_t value = synth_mcp3008_read(&_controller, controls[i].channel);

    if (abs(controls[i].value - value) > _threshold) {
      controls[i].value = value;

      // apply immediately as efficient to do that here
      // and makes the sound responsive
      switch (controls[i].action) {
        case ACTION_DETUNE: {
          // map raw value to small range 0 to 0.1
          fix16 d = float2fix16(value / 10240.f);
          // +/- detune each voice.
          context->voices[0].detune = FIX16_ONE - d;
          context->voices[1].detune = FIX16_ONE + d;

          synth_audiocontext_set_wavetable_stride(&(context->voices[0]));
          synth_audiocontext_set_wavetable_stride(&(context->voices[1]));
        } break;

        case ACTION_WIDTH:
          // TODO: pulse width
          break;

        case ACTION_DELAY:
          // delay is proportional to sample rate
          context->delay = _sampleRate * normal(controls[i].value);
          break;

        case ACTION_DELAYGAIN:
          // shift 6 to make 10 bit number a 16 bit number
          context->delayGain = tofix16(controls[i].value);
          break;

        case ACTION_ATTACK:
          context->envelope.attack =
              synth_audiocontext_to_duration(normal(controls[i].value));
          break;

        case ACTION_DECAY:
          context->envelope.decay =
              synth_audiocontext_to_duration(normal(controls[i].value));
          break;

        case ACTION_RELEASE:
          context->envelope.release =
              synth_audiocontext_to_duration(normal(controls[i].value));
          break;

        case ACTION_SUSTAIN:
          context->envelope.sustain = float2fix16(normal(controls[i].value));
          break;

        default:
          break;
      }
    }
  }
}