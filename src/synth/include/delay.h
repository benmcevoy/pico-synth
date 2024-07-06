#ifndef SYNTH_DELAY_
#define SYNTH_DELAY_

#include "audiocontext.h"
#include "fixedpoint.h"

#define DELAY_BUFFER_SIZE (SAMPLE_RATE)
#define DELAY_BUFFER_SIZE_FIX16 (DELAY_BUFFER_SIZE * 65536)

void synth_delay_init(void);

fix16 synth_delay_process(delay_t* delay, fix16 amplitude);

void synth_delay_set_delay(delay_t* delay);

#endif
