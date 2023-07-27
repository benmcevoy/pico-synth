#ifndef SYNTH_ENVELOPE_
#define SYNTH_ENVELOPE_

#include "audiocontext.h"

void synth_envelope_init(float sampleRate);

float synth_envelope_process(AudioContext_t* context);

void synth_envelope_note_on(AudioContext_t* context);

void synth_envelope_note_off(AudioContext_t* context);

#endif