#ifndef SYNTH_METRONOME_
#define SYNTH_METRONOME_

#include "audiocontext.h"
#include "pico/stdlib.h"
#include "pitchtable.h"
#include "waveform.h"

const uint LED_PIN = 16;
static uint32_t interval = 0;
static uint32_t led_on_interval = 0;
static fix16 velocity = FIX16_POINT_5;

static voice_t voice = {.detune = 0,
                        .frequency = PITCH_A4,
                        .width = FIX16_POINT_5,
                        .waveform = SINE};

static envelope_t envelope = {.attack = 0,
                              .decay = 0,
                              .sustain = FIX16_POINT_5,
                              .release = 0,
                              .state = OFF,
                              .trigger_attack = false,
                              .envelope = FIX16_ONE};

static void tick(metronome_t* metronome) {
  if (--interval == 0) {
    metronome->is_beat = true;
    interval = metronome->duration_in_samples;
    led_on_interval = 2000;
    return;
  }

  metronome->is_beat = false;

  if (led_on_interval > 0) led_on_interval--;

  return;
}

void synth_metronome_set_bpm(metronome_t* metronome, uint8_t bpm) {
  metronome->duration_in_samples = 60 / (float)bpm * SAMPLE_RATE;
  interval = metronome->duration_in_samples;
}

void synth_metronome_init(metronome_t* metronome, uint8_t bpm) {
  synth_metronome_set_bpm(metronome, bpm);
  synth_waveform_set_wavetable_stride(&voice, FIX16_ONE);
  envelope.release = synth_envelope_to_duration(FIX16_POINT_1);

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
}

fix16 synth_metronome_process(metronome_t* metronome) {
  tick(metronome);

  if (!metronome->enabled) return 0;

  fix16 sample = synth_waveform_sample(&voice, FIX16_ONE);

  sample = multfix16(sample, velocity);

  envelope.trigger_attack = metronome->is_beat;

  gpio_put(LED_PIN, led_on_interval > 0);

  sample = multfix16(sample, synth_envelope_process(&envelope));

  return sample;
}

#endif