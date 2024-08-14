#include "../include/controller.h"

#include <stdio.h>

#include "../include/delay.h"
#include "../include/envelope.h"
#include "../include/waveform.h"
#include "./controller_config.c"

#define MAX_VALUE 1023
#define FIX16_MAXVALUE 67108864
#define THRESHOLD 10

#define NEAR_ZERO (THRESHOLD * 2)
#define NEAR_ONE (MAX_VALUE - 2 * THRESHOLD)

static inline uint16_t snap(uint16_t value) {
  if (value < NEAR_ZERO) return 0;
  if (value > NEAR_ONE) return MAX_VALUE;
  return value;
}

/// @brief scale 10bit value to fractional part of fix16
/// @param value
/// @return
static inline fix16 normal(uint16_t value) { return value << 6; }

void synth_controller_init() { synth_controller_config_init(); }

// static void write(audio_context_t* context) {
//   synth_mcp23s08_write(&controller1, 1);
// }

static void read(audio_context_t* context) {
  // read channels
  for (size_t i = 0; i < CONTROLS_COUNT; i++) {
    // TODO: this method is too long
    // synth_mcp3008_read should be
    // controls[i].controller->read(controls[i].controller) the snap funciton is
    // part of the analog implementation - mcp3008 so i think i need something
    // to abstract two implementations

    control_t* control = &controls[i];
    uint16_t value;
    bool value_has_changed = false;

    if (control->control_type == CONTROL_TYPE_ANALOG) {
      value = snap(synth_mcp3008_read(control->spi_device, control->channel));

      if (abs(control->value - value) > THRESHOLD) {
        control->value = value;
        value_has_changed = true;
      }
    }

    if (control->control_type == CONTROL_TYPE_MOMENTRY) {
      value = synth_mcp23s08_read(control->spi_device, control->channel);

      if (control->value != value) {
        value_has_changed = true;
      }

      control->value = value;
    }

    if (control->control_type == CONTROL_TYPE_TOGGLE) {
      value = synth_mcp23s08_read(control->spi_device, control->channel);

      if (control->value == 1) {
        control->value = control->value == 1 ? 0 : 1;
        value_has_changed = true;
      }
    }

    if (!value_has_changed) continue;

    // apply immediately as efficient to do that here
    // and makes the sound responsive
    switch (control->action) {
      case CONTROL_ACTION_DETUNE: {
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

      case CONTROL_ACTION_WIDTH: {
        fix16 width = multfix16(FIX16_PI, normal(value));

        context->voices[0].width = width;
        context->voices[1].width = width;
      } break;

      case CONTROL_ACTION_DELAY:
        // delay is proportional to sample rate
        context->delay.delay_in_samples =
            fix2int16(multfix16(DELAY_BUFFER_SIZE_FIX16, normal(value)));
        break;

      case CONTROL_ACTION_DELAY_FEEDBACK:
        // feedback can get close 1.0 which makes for whacky sounds
        context->delay.feedback = normal(value);
        break;

      case CONTROL_ACTION_DELAY_FEEDBACK_INFINITE:
        // TODO: ramp(envelope?) from feedback to 1 while button is held down
        // then ramp back to feedback on release
        if(control->value)printf(".");
        break;

      case CONTROL_ACTION_DELAY_DRY_WET_MIX:
        context->delay.dry_wet_mix = normal(value);
        break;

      case CONTROL_ACTION_CUTOFF:
        // max is quarter sample rate, about 8kHz
        context->filter.cutoff =
            multfix16(FIX16_SAMPLE_RATE >> 3, normal(value));
        break;

      case CONTROL_ACTION_RESONANCE:
        // TODO: is 2200 the MAX_RESONANCE?
        context->filter.resonance = float2fix16(value / 2200.f);
        break;

      case CONTROL_ACTION_ATTACK:
        context->envelope.attack = synth_envelope_to_duration(normal(value));
        break;

      case CONTROL_ACTION_DECAY:
        context->envelope.decay = synth_envelope_to_duration(normal(value));
        break;

      case CONTROL_ACTION_RELEASE:
        context->envelope.release = synth_envelope_to_duration(normal(value));
        break;

      case CONTROL_ACTION_SUSTAIN:
        context->envelope.sustain = normal(value);
        break;

      default:
        break;
    }
  }
}

void synth_controller_task(audio_context_t* context) {
  // TODO: there isn't really a read/write
  // taking a cue from the PIO I can read pin of the GPIO extender and write the
  // state to pin+1 (side set)
  read(context);
  // write(context);
}