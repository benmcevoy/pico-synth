#ifndef SYNTH_ENVELOPE_
#define SYNTH_ENVELOPE_

#include "audiocontext.h"

void synth_envelope_init(float sampleRate);

float synth_envelope_process(Voice_t* voice);

void synth_envelope_note_on(Voice_t* voice);

void synth_envelope_note_off(Voice_t* voice);

#endif