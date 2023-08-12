#ifndef SYNTH_ENVELOPE_
#define SYNTH_ENVELOPE_

#include "audiocontext.h"
#include "fixedpoint.h"

fix16 synth_envelope_to_duration(fix16 value);

fix16 synth_envelope_process(AudioContext_t* context);

fix16 synth_envelope_gate(Gate_t* gate);

void synth_envelope_note_on(AudioContext_t* context);

void synth_envelope_note_off(AudioContext_t* context);

#endif