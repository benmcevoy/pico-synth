#include "../include/controller.h"

#include <stdio.h>

#include "../include/delay.h"
#include "../include/envelope.h"
#include "../include/mcp3008.h"
#include "../include/waveform.h"

#define SPI_CLOCK 2
#define SPI_TX 3
#define SPI_RX 4
#define SPI_CS 5

#define CONTROLS_COUNT 4
#define MAX_VALUE 1024
#define FIX16_MAXVALUE 67108864
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

/// @brief scale 10bit value to fractional part of fix16
/// @param value
/// @return
static inline fix16 normal(uint16_t value) { return value << 6; }

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
      .action = ACTION_DELAY_FEEDBACK,
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

          synth_waveform_set_wavetable_stride(&(context->voices[0]),
                                              context->pitch_bend);
          synth_waveform_set_wavetable_stride(&(context->voices[1]),
                                              context->pitch_bend);
        } break;

        case ACTION_WIDTH: {
          fix16 width = multfix16(FIX16_PI, normal(value));

          context->voices[0].width = width;
          context->voices[1].width = width;
        } break;

        case ACTION_DELAY:
          // delay is proportional to sample rate
          context->delay.delay_in_samples =
              fix2int16(multfix16(DELAY_BUFFER_SIZE_FIX16, normal(value)));
          break;

        case ACTION_DELAY_FEEDBACK:
          // feedback can get close 1.0 which makes for whacky sounds
          context->delay.feedback = normal(value);
          break;

        case ACTION_DELAY_DRY_WET_MIX:
          context->delay.dry_wet_mix = normal(value);
          break;

        case ACTION_CUTOFF:
          // max is quarter sample rate, about 8kHz
          context->filter.cutoff =
              multfix16(FIX16_SAMPLE_RATE >> 3, normal(value));
          break;

        case ACTION_RESONANCE:
          context->filter.resonance = float2fix16(value / 2200.f);
          break;

        case ACTION_ATTACK:
          context->envelope.attack = synth_envelope_to_duration(normal(value));
          break;

        case ACTION_DECAY:
          context->envelope.decay = synth_envelope_to_duration(normal(value));
          break;

        case ACTION_RELEASE:
          context->envelope.release = synth_envelope_to_duration(normal(value));
          break;

        case ACTION_SUSTAIN:
          context->envelope.sustain = normal(value);
          break;

        default:
          break;
      }
    }
  }
}