#ifndef SYNTH_TEST_
#define SYNTH_TEST_

#include <math.h>
#include <stdlib.h>

#include "include/audiocontext.h"
#include "include/envelope.h"
#include "include/midi.h"
#include "include/pitchtable.h"
#include "include/waveform.h"

void test_waveform(Voice_t* voice, Waveform_t waveform) {
    voice->waveform = waveform;

    voice->frequency = PITCH_A4;
    synth_envelope_note_on(voice);
    sleep_ms(500);
    synth_envelope_note_off(voice);
    sleep_ms(300);

    voice->frequency = PITCH_E4;
    synth_envelope_note_on(voice);
    sleep_ms(500);
    synth_envelope_note_off(voice);
    sleep_ms(300);

    voice->frequency = PITCH_A5;
    synth_envelope_note_on(voice);
    sleep_ms(500);
    synth_envelope_note_off(voice);
    sleep_ms(300);

    voice->frequency = PITCH_E4;
    synth_envelope_note_on(voice);
    sleep_ms(500);
    synth_envelope_note_off(voice);
    sleep_ms(300);
}

void test_sweep(Voice_t* voice, Waveform_t waveform) {
    voice->waveform = waveform;
    synth_envelope_note_on(voice);
    for (size_t i = PITCH_A4; i < PITCH_A5; i = i + 4) {
        sleep_ms(10);
        voice->frequency = i;
    }
    synth_envelope_note_off(voice);
}

void test_pattern(Voice_t* voice) {
    uint8_t pattern[6] = {0, 1, 3, 5, 3, 1};

    voice->waveform = SAW;

    for (int i = 0; i < sizeof(pattern); i++) {
        voice->frequency = frequency_from_reference_pitch(PITCH_C3, pattern[i]);
        synth_envelope_note_on(voice);
        sleep_ms(220);
        synth_envelope_note_off(voice);
    }
}

void test_midi_pattern(Voice_t* voice) {
    // nicked from the tinyusb example
    uint8_t pattern[] = {74, 78, 81,  86, 90, 93, 98,  102, 57, 61, 66, 69, 73,
                         78, 81, 85,  88, 92, 97, 100, 97,  92, 88, 85, 81, 78,
                         74, 69, 66,  62, 57, 62, 66,  69,  74, 78, 81, 86, 90,
                         93, 97, 102, 97, 93, 90, 85,  81,  78, 73, 68, 64, 61,
                         56, 61, 64,  68, 74, 78, 81,  86,  90, 93, 98, 102};

    voice->waveform = SAW;

    for (int i = 0; i < sizeof(pattern); i++) {
        voice->frequency = synth_midi_frequency_from_midi_note(pattern[i]);
        synth_envelope_note_on(voice);
        sleep_ms(120);
        synth_envelope_note_off(voice);
        sleep_ms(120);
    }
}

void synth_play_test(Voice_t* voice) {
    voice->attack = 0.3;
    voice->decay = 0.3;
    voice->sustain = 0.7;
    voice->release = 0.3;

    voice->waveform = TRIANGLE;
    voice->frequency = PITCH_E4;
    synth_envelope_note_on(voice);
    sleep_ms(1000);
    synth_envelope_note_off(voice);
    sleep_ms(400);

    voice->attack = 0.05;
    voice->decay = 0.05;
    voice->sustain = 0.8;
    voice->release = 0.2;

    test_pattern(voice);
    test_midi_pattern(voice);

    test_waveform(voice, NOISE);
    test_waveform(voice, TRIANGLE);
    test_waveform(voice, SAW);
    test_waveform(voice, SQUARE);
    test_waveform(voice, SINE);

    test_sweep(voice, NOISE);
    test_sweep(voice, TRIANGLE);
    test_sweep(voice, SAW);
    test_sweep(voice, SQUARE);
    test_sweep(voice, SINE);
}

#endif