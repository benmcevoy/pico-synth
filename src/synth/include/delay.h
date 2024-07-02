#ifndef SYNTH_DELAY_
#define SYNTH_DELAY_

#include "audiocontext.h"
#include "fixedpoint.h"

void synth_delay_init(void);

fix16 synth_delay_process(delay_t* delay, fix16 amplitude);

#endif
