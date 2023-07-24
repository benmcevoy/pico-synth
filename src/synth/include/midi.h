#ifndef SYNTH_MIDI_
#define SYNTH_MIDI_

#include <math.h>
#include <stdlib.h>

#include "audiocontext.h"
#include "pitchtable.h"
#include "tusb.h"

#define SYNTH_MIDI_NOTEON 144
#define SYNTH_MIDI_NOTEOFF 128

float synth_midi_frequency_from_midi_note(uint8_t n); 

void synth_midi_task(AudioContext_t* context);

#endif