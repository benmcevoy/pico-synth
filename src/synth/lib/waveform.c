#include <math.h>

#include "../include/waveform.h"
#include "../include/audiocontext.h"

double _phase = 0;

double sine(AudioContext_t *context, double frequency, double amplitude)
{
    _phase += TWOPI * frequency / context->SampleRate;

    if (_phase >= TWOPI)
        _phase -= TWOPI;

    return sin(_phase) * amplitude;
}

double square(AudioContext_t *context, double frequency, double amplitude)
{
    return 0;
}

double saw(AudioContext_t *context, double frequency, double amplitude)
{
    return 0;
}

double triangle(AudioContext_t *context, double frequency, double amplitude)
{
    // TODO: well this don't sound right...
    _phase += TWOPI * frequency / context->SampleRate;

    if (_phase >= TWOPI)
        _phase -= TWOPI;

    return (asin(cos(_phase)) / M_PI_2) * amplitude;
}

double noise(AudioContext_t *context, double frequency, double amplitude)
{
    // random between -1..1
    return (rand() / (double)(RAND_MAX)) * 2 - 1;
}

double synth_waveform_sample(AudioContext_t *context, enum waveform waveform, double frequency, double amplitude)
{
    if (waveform == SINE)
        return sine(context, frequency, amplitude);

    if (waveform == SQUARE)
        return square(context, frequency, amplitude);

    if (waveform == SAW)
        return saw(context, frequency, amplitude);

    if (waveform == TRIANGLE)
        return triangle(context, frequency, amplitude);

    if (waveform == NOISE)
        return noise(context, frequency, amplitude);

    return 0;
}