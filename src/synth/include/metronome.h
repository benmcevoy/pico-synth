#ifndef SYNTH_METRONOME_
#define SYNTH_METRONOME_

#include "audiocontext.h"
#include "pitchtable.h"
#include "tempo.h"
#include "waveform.h"

static voice_t voice = {.detune = FIX16_ONE,
                        .frequency = PITCH_A4,
                        .gain = FIX16_POINT_3,
                        .waveform = SINE};

static envelope_t envelope = {.attack = 0,
                              .decay = 0,
                              .sustain = FIX16_ONE,
                              .release = 0,
                              .state = OFF,
                              .trigger_attack = false,
                              .envelope = FIX16_ONE};

fix16 synth_metronome_process(tempo_t* tempo) {
  fix16 sample = synth_waveform_sample(&voice);

  envelope.trigger_attack = tempo->isBeat;

  sample = multfix16(voice.gain,
                     multfix16(sample, synth_envelope_process(&envelope)));

  return sample;
}

void synth_metronome_init() {
  synth_waveform_set_wavetable_stride(&voice);
  envelope.release = synth_envelope_to_duration(0.1f);
}

#endif