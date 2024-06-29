#ifndef SYNTH_ENVELOPE_
#define SYNTH_ENVELOPE_

#include "audiocontext.h"
#include "fixedpoint.h"

fix16 synth_envelope_process(envelope_t* envelope);

void synth_envelope_note_on(envelope_t* envelope);

void synth_envelope_note_off(envelope_t* envelope);

#endif