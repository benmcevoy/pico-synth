#ifndef SYNTH_WAVEFORM_
#define SYNTH_WAVEFORM_

#include "../include/audiocontext.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum waveform {
    SINE = 0,
    SQUARE,
    SAW,
    TRIANGLE,
    NOISE
};

double synth_waveform_sample(AudioContext_t *context, enum waveform waveform, double frequency, double amplitude);

#endif