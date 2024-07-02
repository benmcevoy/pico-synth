#ifndef SYNTH_ENVELOPE_
#define SYNTH_ENVELOPE_

#include "audiocontext.h"
#include "fixedpoint.h"

fix16 synth_envelope_process(envelope_t* envelope);

void synth_envelope_note_on(envelope_t* envelope);

void synth_envelope_note_off(envelope_t* envelope);

static inline fix16 synth_envelope_to_duration(fix16 value) {
  return multfix16(value, FIX16_SAMPLE_RATE);
}

#endif