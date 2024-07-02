#include "../include/controller.h"

#include <stdio.h>

#include "../include/mcp3008.h"

#define SPI_CLOCK 2
#define SPI_TX 3
#define SPI_RX 4
#define SPI_CS 5

#define CONTROLS_COUNT 4
#define MAX_VALUE 1024
#define THRESHOLD 10

#define NEAR_ZERO (THRESHOLD * 2)
#define NEAR_ONE (MAX_VALUE - 2 * THRESHOLD)

static mcp3008_t controller;
control_t* controls;

static inline uint16_t snap(uint16_t value) {
  if (value < NEAR_ZERO) return 0;
  if (value > NEAR_ONE) return MAX_VALUE;
  return value;
}

static inline float normal(uint16_t value) { return value / (float)MAX_VALUE; }
static inline float tofix16(uint16_t value) { return value << 6; }

void synth_controller_init() {
  controller =
      synth_mcp3008_init(spi0, 120000, SPI_CS, SPI_CLOCK, SPI_TX, SPI_RX);

  controls = malloc(sizeof(control_t) * CONTROLS_COUNT);

  controls[0] = (control_t){
      .channel = 0,
      .value = 0,
      .action = ACTION_DELAY,
  };
  controls[1] = (control_t){
      .channel = 1,
      .value = 0,
      .action = ACTION_DELAYGAIN,
  };  
  controls[2] = (control_t){
      .channel = 2,
      .value = 0,
      .action = ACTION_CUTOFF,
  };
  controls[3] = (control_t){
      .channel = 3,
      .value = 0,
      .action = ACTION_RESONANCE,
  };
}

void synth_controller_task(audio_context_t* context) {
  // read channels
  for (size_t i = 0; i < CONTROLS_COUNT; i++) {
    uint16_t value = snap(synth_mcp3008_read(&controller, controls[i].channel));

    if (abs(controls[i].value - value) > THRESHOLD) {
      controls[i].value = value;

      // apply immediately as efficient to do that here
      // and makes the sound responsive
      switch (controls[i].action) {
        case ACTION_DETUNE: {
          // map raw value to small range 0 to 0.005
          fix16 d = float2fix16(value / 40960.f);
          // +/- detune each voice.
          context->voices[0].detune = -d;
          context->voices[1].detune = +d;

          synth_audiocontext_set_wavetable_stride(&(context->voices[0]));
          synth_audiocontext_set_wavetable_stride(&(context->voices[1]));
        } break;

        case ACTION_DELAY:
          // delay is proportional to sample rate
          context->delay = context->sample_rate * normal(value);
          break;

        case ACTION_DELAYGAIN:
          // shift 6 to make 10 bit number a 16 bit number
          context->delay_gain = tofix16(value);
          break;

        case ACTION_CUTOFF:
          // max is quarter sample rate, about 8kHz
          context->cutoff =
              float2fix16((context->sample_rate >> 3) * normal(value));
          break;

        case ACTION_RESONANCE:
          context->resonance = float2fix16(value / 2200.f);
          break;

        case ACTION_ATTACK:
          context->envelope.attack =
              synth_audiocontext_to_duration(normal(value));
          break;

        case ACTION_DECAY:
          context->envelope.decay =
              synth_audiocontext_to_duration(normal(value));
          break;

        case ACTION_RELEASE:
          context->envelope.release =
              synth_audiocontext_to_duration(normal(value));
          break;

        case ACTION_SUSTAIN:
          context->envelope.sustain = float2fix16(normal(value));
          break;

        default:
          break;
      }
    }
  }
}