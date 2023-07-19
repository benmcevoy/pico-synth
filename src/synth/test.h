#ifndef SYNTH_TEST_
#define SYNTH_TEST_

#include <stdlib.h>
#include <math.h>

#include "include/audiocontext.h"
#include "include/waveform.h"
#include "include/pitchtable.h"

void test_waveform(AudioContext_t *context, Waveform_t waveform)
{
    context->Voice.waveform = waveform;

    context->Voice.frequency = PITCH_A4;
    sleep_ms(200);

    context->Voice.frequency = PITCH_E4;
    sleep_ms(200);

    context->Voice.frequency = PITCH_A5;
    sleep_ms(200);

    context->Voice.frequency = PITCH_E4;
    sleep_ms(200);
}

void test_sweep(AudioContext_t *context, Waveform_t waveform)
{
    context->Voice.waveform = waveform;
    for (size_t i = PITCH_A4; i < PITCH_A5; i++)
    {
        sleep_ms(10);
        context->Voice.frequency = i;
    }
}

float frequency_from_reference_pitch(float f, int pitch){
    return f * powf(2.f, (float)pitch / 12.f);
}

void test_pattern(AudioContext_t *context){
    int pattern[6] = { 0, 1, 3, 5, 3, 1 };

    context->Voice.waveform = TRIANGLE;

    for(int i =0; i< 6; i++)
    {
        context->Voice.frequency = frequency_from_reference_pitch(PITCH_C3, pattern[i]);
        sleep_ms(220);
    }
}

void synth_play_test(AudioContext_t *context)
{
    test_pattern(context);

    test_waveform(context, NOISE);
    test_waveform(context, TRIANGLE);
    test_waveform(context, SAW);
    test_waveform(context, SQUARE);
    test_waveform(context, SINE);

    test_sweep(context, NOISE);
    test_sweep(context, TRIANGLE);
    test_sweep(context, SAW);
    test_sweep(context, SQUARE);
    test_sweep(context, SINE);
}

#endif