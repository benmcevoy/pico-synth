#ifndef SYNTH_FILTER_
#define SYNTH_FILTER_

#include <stdlib.h>
#include "audiocontext.h"

void synth_filter_init(audio_context_t* context);
void synth_filter_process(audio_context_t* context);

#endif