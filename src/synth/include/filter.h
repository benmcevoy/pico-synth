#ifndef SYNTH_FILTER_
#define SYNTH_FILTER_

#include <stdlib.h>
#include "audiocontext.h"

// TODO: porlly wnat a struct here as i want to add a filter envelope
// envelope
// envelope depth (contour)
// bypass - currently is a bool on audiocontext
// and follow * depth

void synth_filter_init(audio_context_t* context);
void synth_filter_process(audio_context_t* context);

#endif