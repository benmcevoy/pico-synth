#include <math.h>

#include "../include/waveform.h"
#include "../include/audiocontext.h"

volatile double _phase = 0;

double sign(double value)
{
    if (value == 0)
        return 0;

    return value < 0 ? -1 : 1;
}

double arccos(double x) {
   return (-0.69813170079773212 * x * x - 0.87266462599716477) * x + M_PI_2;
}

double sine(AudioContext_t *context)
{
    double frequency = context->Voice.frequency;
    double amplitude = context->Voice.amplitude;

    _phase += TWOPI * frequency / context->SampleRate;

    if (_phase >= TWOPI)
        _phase -= TWOPI;

    return sin(_phase) * amplitude;
}

double square(AudioContext_t *context)
{
    double frequency = context->Voice.frequency;
    double amplitude = context->Voice.amplitude;

    _phase += TWOPI * frequency / context->SampleRate;

    if (_phase >= TWOPI)
        _phase -= TWOPI;

    return amplitude * sign(sin(_phase));
}

double saw(AudioContext_t *context)
{
    double frequency = context->Voice.frequency;
    double amplitude = context->Voice.amplitude;
    double time = (double)context->SamplesElapsed / context->SampleRate;
    double period = 1.0 / frequency;

    return amplitude * (fmod(time, period) * frequency);
}

double triangle(AudioContext_t *context)
{
    double frequency = context->Voice.frequency;
    double amplitude = context->Voice.amplitude;

    _phase += TWOPI * frequency / context->SampleRate;

    if (_phase >= TWOPI)
        _phase -= TWOPI;

    return (arccos(cos(_phase)) / M_PI_2) * amplitude;
}

double noise(AudioContext_t *context)
{
    // random between -1..1
    return (rand() / (double)(RAND_MAX)) * 2 - 1;
}

double synth_waveform_sample(AudioContext_t *context)
{
    enum waveform waveform = context->Voice.waveform;

    if (waveform == SINE)
        return sine(context);

    if (waveform == SQUARE)
        return square(context);

    if (waveform == SAW)
        return saw(context);

    if (waveform == TRIANGLE)
        return triangle(context);

    if (waveform == NOISE)
        return noise(context);

    return 0;
}