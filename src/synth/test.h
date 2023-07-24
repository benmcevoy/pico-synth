#ifndef SYNTH_TEST_
#define SYNTH_TEST_

#include <math.h>
#include <stdlib.h>

#include "include/audiocontext.h"
#include "include/envelope.h"
#include "include/pitchtable.h"
#include "include/waveform.h"
#include "include/midi.h"

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
    uint8_t pattern[6] = {0, 1, 3, 5, 3, 1};

    context->Voice.waveform = TRIANGLE;

    for (int i = 0; i < sizeof(pattern); i++) {
        context->Voice.frequency =
            frequency_from_reference_pitch(PITCH_C3, pattern[i]);
        synth_envelope_note_on(&(context->Voice));
        sleep_ms(220);
        synth_envelope_note_off(&(context->Voice));
    }
}

void test_midi_pattern(AudioContext_t* context) {
    // nicked from the tinyusb example
    uint8_t pattern[] = {74, 78, 81,  86, 90, 93, 98,  102, 57, 61, 66, 69, 73,
                         78, 81, 85,  88, 92, 97, 100, 97,  92, 88, 85, 81, 78,
                         74, 69, 66,  62, 57, 62, 66,  69,  74, 78, 81, 86, 90,
                         93, 97, 102, 97, 93, 90, 85,  81,  78, 73, 68, 64, 61,
                         56, 61, 64,  68, 74, 78, 81,  86,  90, 93, 98, 102};

    context->Voice.waveform = SAW;

    for (int i = 0; i < sizeof(pattern); i++) {
        context->Voice.frequency = synth_midi_frequency_from_midi_note(pattern[i]);
        synth_envelope_note_on(&(context->Voice));
        sleep_ms(120);
        synth_envelope_note_off(&(context->Voice));
        sleep_ms(120);
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

    context->Voice.attack = 0.05;
    context->Voice.decay = 0.05;
    context->Voice.sustain = 0.8;
    context->Voice.release = 0.2;

    test_pattern(context);
    test_midi_pattern(context);

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