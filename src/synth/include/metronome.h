#ifndef SYNTH_METRONOME_
#define SYNTH_METRONOME_

#include "audiocontext.h"
#include "pitchtable.h"
#include "tempo.h"
#include "waveform.h"

static Voice_t _voice;
static Envelope_t _envelope;

fix16 synth_metronome_process(Tempo_t* tempo) {
    fix16 sample = synth_waveform_sample(&_voice);

    _envelope.triggerAttack = tempo->isBeat;

    sample = multfix16(_voice.gain,
                       multfix16(sample, synth_envelope_process(&_envelope)));

    return sample;
}

void synth_metronome_init() {
    _voice.detune = FIX16_ONE;
    _voice.frequency = PITCH_A4;
    _voice.gain = float2fix16(0.75f);
    _voice.waveform = SINE;
    synth_audiocontext_set_wavetable_stride(&_voice);

    _envelope.attack = synth_envelope_to_duration(0);
    _envelope.decay = synth_envelope_to_duration(0);
    _envelope.sustain = FIX16_ONE;
    _envelope.release = synth_envelope_to_duration(float2fix16(0.1));
    _envelope.state = OFF;
    _envelope.triggerAttack = false;
    _envelope.envelope = FIX16_ONE;
}

#endif