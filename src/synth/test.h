#ifndef SYNTH_TEST_
#define SYNTH_TEST_

#include <stdlib.h>

#include "include/audiocontext.h"
#include "include/waveform.h"
#include "include/pitchtable.h"

void test_waveform(AudioContext_t *context, enum waveform waveform)
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

void test_sweep(AudioContext_t *context, enum waveform waveform)
{
    context->Voice.waveform = waveform;
    for (size_t i = PITCH_A4; i < PITCH_A5; i++)
    {
        sleep_ms(10);
        context->Voice.frequency = i;
    }
}

void synth_play_test(AudioContext_t *context)
{
    test_waveform(context, NOISE);
    test_waveform(context, TRIANGLE);
    test_waveform(context, SAW);
    test_waveform(context, SQUARE);
    test_waveform(context, NOISE);

    test_sweep(context, SINE);
    test_sweep(context, TRIANGLE);
    test_sweep(context, SAW);
    test_sweep(context, SQUARE);
    test_sweep(context, NOISE);
}

#endif