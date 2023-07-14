#include <math.h>

#include "../include/waveform.h"
#include "../include/audiocontext.h"

static double _phase = 0;

static double sine(AudioContext_t *context, double frequency, double amplitude)
{
    _phase += M_PI * 2.0 * frequency / context->SampleRate;

    if (_phase >= 2.0 * M_PI)
    {
        _phase -= 2.0 * M_PI;
    }

    return sin(_phase) * amplitude;
}

static double square(AudioContext_t *context, double frequency, double amplitude)
{
    return 0;
}

static double saw(AudioContext_t *context, double frequency, double amplitude)
{
    return 0;
}

static double triangle(AudioContext_t *context, double frequency, double amplitude)
{
    return 0;
}

static double noise(AudioContext_t *context, double frequency, double amplitude)
{
    return 0;
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