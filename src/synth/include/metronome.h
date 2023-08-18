#ifndef SYNTH_METRONOME_
#define SYNTH_METRONOME_

#include "audiocontext.h"
#include "pitchtable.h"
#include "tempo.h"
#include "waveform.h"

static Voice_t _voice = {.detune = FIX16_ONE,
                         .frequency = PITCH_A4,
                         .gain = float2fix16(0.3f),
                         .waveform = SINE};

static Envelope_t _envelope = {.attack = 0,
                               .decay = 0,
                               .sustain = FIX16_ONE,
                               .release = 0,
                               .state = OFF,
                               .triggerAttack = false,
                               .envelope = FIX16_ONE};

fix16 synth_metronome_process(Tempo_t* tempo) {
    fix16 sample = synth_waveform_sample(&_voice);

    _envelope.triggerAttack = tempo->isBeat;

    sample = multfix16(_voice.gain,
                       multfix16(sample, synth_envelope_process(&_envelope)));

    return sample;
}

void synth_metronome_init() {
    synth_audiocontext_set_wavetable_stride(&_voice);
    _envelope.release = synth_audiocontext_to_duration(0.1f);
}

#endif