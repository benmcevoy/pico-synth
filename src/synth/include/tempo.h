#ifndef SYNTH_TEMPO_
#define SYNTH_TEMPO_

#include <stdint.h>

#include "audiocontext.h"

void synth_tempo_set_bpm(Tempo_t* tempo, uint8_t bpm);

void synth_tempo_process(Tempo_t* tempo);

void synth_tempo_init(Tempo_t* tempo, uint8_t bpm);

#endif