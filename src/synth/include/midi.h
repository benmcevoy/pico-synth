#ifndef SYNTH_MIDI_
#define SYNTH_MIDI_

#include <math.h>
#include <stdlib.h>

#include "audiocontext.h"
#include "pitchtable.h"

#define SYNTH_MIDI_NOTEON 144  // 0b1001 0000
#define SYNTH_MIDI_NOTEOFF 128 // 0b1000 0000

// control command
#define SYNTH_MIDI_CC 176 // 0xB0
// - MIDI CC 71	Sound Controller 2	Allocated to filter resonance/Q.
#define SYNTH_MIDI_CC_RESONANCE 71
// - MIDI CC 72	Sound Controller 3	Allocated to the amp envelope release time. Changes how long notes fade out.
#define SYNTH_MIDI_CC_RELEASE 72
// - MIDI CC 73	Sound Controller 4	Allocated to the amp envelope attack time. Changes how fast the volume rises from the keypress to max volume.
#define SYNTH_MIDI_CC_ATTACK 73
// - MIDI CC 74	Sound Controller 5	Allocated to the filter cutoff frequency Hz value.
#define SYNTH_MIDI_CC_CUTOFF 74


float synth_midi_frequency_from_midi_note(uint8_t note); 

void synth_midi_task(AudioContext_t* context);

void synth_midi_init(float sampleRate);

#endif
