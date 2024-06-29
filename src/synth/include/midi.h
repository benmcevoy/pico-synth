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

#define SYNTH_MIDI_CC_MODWHEEL 1
#define SYNTH_MIDI_CC_VOLUME 7

// - MIDI CC 74	Sound Controller 5	Allocated to the filter cutoff frequency Hz value.
#define SYNTH_MIDI_CC_CUTOFF 74
// - MIDI CC 71	Sound Controller 2	Allocated to filter resonance/Q.
#define SYNTH_MIDI_CC_RESONANCE 71

// - MIDI CC 72	Sound Controller 3	Allocated to the amp envelope release time. Changes how long notes fade out.
#define SYNTH_MIDI_CC_RELEASE 72
// - MIDI CC 73	Sound Controller 4	Allocated to the amp envelope attack time. Changes how fast the volume rises from the keypress to max volume.
#define SYNTH_MIDI_CC_ATTACK 73

void synth_midi_task(audio_context_t* context);
void synth_midi_init();

static fix16 synth_midi_frequency_from_midi_note[128] = {
    0,         0,         0, 
    0,         0,         0,         0,         0,         0,         0,
    0,         0,         0,         0,         0,         0,         0,
    0,         0,         0,         0,         0,         0,         0,
    PITCH_C0,  PITCH_Db0, PITCH_D0,  PITCH_Eb0, PITCH_E0,  PITCH_F0,  PITCH_Gb0,
    PITCH_G0,  PITCH_Ab1, PITCH_A1,  PITCH_Bb1, PITCH_B1,  PITCH_C1,  PITCH_Db1,
    PITCH_D1,  PITCH_Eb1, PITCH_E1,  PITCH_F1,  PITCH_Gb1, PITCH_G1,  PITCH_Ab2,
    PITCH_A2,  PITCH_Bb2, PITCH_B2,  PITCH_C2,  PITCH_Db2, PITCH_D2,  PITCH_Eb2,
    PITCH_E2,  PITCH_F2,  PITCH_Gb2, PITCH_G2,  PITCH_Ab3, PITCH_A3,  PITCH_Bb3,
    PITCH_B3,  PITCH_C3,  PITCH_Db3, PITCH_D3,  PITCH_Eb3, PITCH_E3,  PITCH_F3,
    PITCH_Gb3, PITCH_G3,  PITCH_Ab4, PITCH_A4,  PITCH_Bb4, PITCH_B4,  PITCH_C4,
    PITCH_Db4, PITCH_D4,  PITCH_Eb4, PITCH_E4,  PITCH_F4,  PITCH_Gb4, PITCH_G4,
    PITCH_Ab5, PITCH_A5,  PITCH_Bb5, PITCH_B5,  PITCH_C5,  PITCH_Db5, PITCH_D5,
    PITCH_Eb5, PITCH_E5,  PITCH_F5,  PITCH_Gb5, PITCH_G5,  PITCH_Ab6, PITCH_A6,
    PITCH_Bb6, PITCH_B6,  PITCH_C6,  PITCH_Db6, PITCH_D6,  PITCH_Eb6, PITCH_E6,
    PITCH_F6,  PITCH_Gb6, PITCH_G6,  PITCH_Ab7, PITCH_A7,  PITCH_Bb7, PITCH_B7,
    PITCH_C7,  PITCH_Db7, PITCH_D7,  PITCH_Eb7, PITCH_E7,  PITCH_F7,  PITCH_Gb7,
    PITCH_G7,  PITCH_Ab8, PITCH_A8,  PITCH_Bb8, PITCH_B8,  PITCH_C8,  PITCH_Db8,
    PITCH_D8,  PITCH_Eb8, PITCH_E8,  PITCH_F8,  PITCH_Gb8, PITCH_G8
    };

#endif
