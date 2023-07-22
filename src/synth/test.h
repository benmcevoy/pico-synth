#ifndef SYNTH_TEST_
#define SYNTH_TEST_

#include <math.h>
#include <stdlib.h>

#include "include/audiocontext.h"
#include "include/envelope.h"
#include "include/pitchtable.h"
#include "include/waveform.h"

float frequency_from_reference_pitch(float f, int pitch) {
    return f * powf(2.f, (float)pitch / 12.f);
}

void test_waveform(AudioContext_t* context, Waveform_t waveform) {
    context->Voice.waveform = waveform;

    context->Voice.frequency = PITCH_A4;
    synth_envelope_note_on(&(context->Voice));
    sleep_ms(500);
    synth_envelope_note_off(&(context->Voice));
    sleep_ms(300);

    context->Voice.frequency = PITCH_E4;
    synth_envelope_note_on(&(context->Voice));
    sleep_ms(500);
    synth_envelope_note_off(&(context->Voice));
    sleep_ms(300);

    context->Voice.frequency = PITCH_A5;
    synth_envelope_note_on(&(context->Voice));
    sleep_ms(500);
    synth_envelope_note_off(&(context->Voice));
    sleep_ms(300);

    context->Voice.frequency = PITCH_E4;
    synth_envelope_note_on(&(context->Voice));
    sleep_ms(500);
    synth_envelope_note_off(&(context->Voice));
    sleep_ms(300);
}

void test_sweep(AudioContext_t* context, Waveform_t waveform) {
    context->Voice.waveform = waveform;
    synth_envelope_note_on(&(context->Voice));
    for (size_t i = PITCH_A4; i < PITCH_A5; i = i + 4) {
        sleep_ms(10);
        context->Voice.frequency = i;
    }
    synth_envelope_note_off(&(context->Voice));
}

void test_pattern(AudioContext_t* context) {
    int pattern[6] = {0, 1, 3, 5, 3, 1};

    context->Voice.waveform = TRIANGLE;

    for (int i = 0; i < 6; i++) {
        context->Voice.frequency =
            frequency_from_reference_pitch(PITCH_C3, pattern[i]);
        synth_envelope_note_on(&(context->Voice));
        sleep_ms(220);
        synth_envelope_note_off(&(context->Voice));
    }
}

void synth_play_test(AudioContext_t* context) {
    context->Voice.attack = 0.3;
    context->Voice.decay = 0.3;
    context->Voice.sustain = 0.7;
    context->Voice.release = 0.3;

    context->Voice.waveform = TRIANGLE;
    context->Voice.frequency = PITCH_E4;
    synth_envelope_note_on(&(context->Voice));
    sleep_ms(1000);
    synth_envelope_note_off(&(context->Voice));
    sleep_ms(400);

    context->Voice.attack = 0.1;
    context->Voice.decay = 0.1;
    context->Voice.sustain = 0.8;
    context->Voice.release = 0.4;

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