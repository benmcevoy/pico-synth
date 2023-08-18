#ifndef SYNTH_ENVELOPE_
#define SYNTH_ENVELOPE_

#include "audiocontext.h"
#include "fixedpoint.h"

fix16 synth_envelope_process(Envelope_t* envelope);

void synth_envelope_note_on(Envelope_t* envelope);

void synth_envelope_note_off(Envelope_t* envelope);

#endif