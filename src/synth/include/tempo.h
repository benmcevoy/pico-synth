#ifndef SYNTH_TEMPO_
#define SYNTH_TEMPO_

#include <stdint.h>

#include "audiocontext.h"

void synth_tempo_set_bpm(tempo_t* tempo, uint8_t bpm);

void synth_tempo_process(tempo_t* tempo);

void synth_tempo_init(tempo_t* tempo, uint8_t bpm);

#endif