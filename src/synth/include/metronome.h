#ifndef SYNTH_METRONOME_
#define SYNTH_METRONOME_

#include "audiocontext.h"
#include "pitchtable.h"
#include "waveform.h"

static voice_t voice = {.detune = FIX16_ONE,
                        .frequency = PITCH_A4,
                        .width = FIX16_POINT_5,
                        .waveform = SQUARE};

static envelope_t envelope = {.attack = 0,
                              .decay = 0,
                              .sustain = FIX16_ONE,
                              .release = 0,
                              .state = OFF,
                              .trigger_attack = false,
                              .envelope = FIX16_ONE};

static uint32_t interval = 0;

static void tick(metronome_t* metronome) {
  if (--interval == 0) {
    metronome->is_beat = true;
    interval = metronome->duration_in_samples;
    return;
  }

  metronome->is_beat = false;
  return;
}

void synth_metronome_set_bpm(metronome_t* metronome, uint8_t bpm) {
  metronome->duration_in_samples = 60 / (float)bpm * SAMPLE_RATE;
  interval = metronome->duration_in_samples;
}

void synth_metronome_init(metronome_t* metronome, uint8_t bpm) {
  synth_metronome_set_bpm(metronome, bpm);
  synth_waveform_set_wavetable_stride(&voice, 0);
  envelope.release = synth_envelope_to_duration(FIX16_POINT_1);
}

fix16 synth_metronome_process(metronome_t* metronome) {
  tick(metronome);

  if (!metronome->enabled) return 0;

  fix16 sample = synth_waveform_sample(&voice, 0);

  envelope.trigger_attack = metronome->is_beat;

  sample = multfix16(sample, synth_envelope_process(&envelope));

  return sample;
}

#endif