#ifndef SYNTH_ENVELOPE_
#define SYNTH_ENVELOPE_

#include "audiocontext.h"
#include "fixedpoint.h"

void synth_envelope_init(uint32_t sampleRate);

fix16 synth_envelope_process(AudioContext_t* context);

void synth_envelope_note_on(AudioContext_t* context);

void synth_envelope_note_off(AudioContext_t* context);

#endif