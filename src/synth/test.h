#ifndef SYNTH_TEST_
#define SYNTH_TEST_

#include <math.h>
#include <stdlib.h>

#include "include/audiocontext.h"
#include "include/envelope.h"
#include "include/midi.h"
#include "include/pitchtable.h"
#include "include/waveform.h"

// TODO: get rid of floats? I don't really even use this fucntion
static fix16 frequency_from_reference_pitch(fix16 reference, uint8_t pitch) {
    float freq = fix2float16(reference);
    return float2fix16(freq * powf(2.f, (float)pitch / 12.f));
}

static void set_frequency(audio_context_t* context, fix16 f,
                          waveform_t waveform) {
    for (int v = 0; v < VOICES_LENGTH; v++) {
        voice_t* voice = &context->voices[v];
        voice->frequency = f;
        voice->waveform = waveform;
        synth_audiocontext_set_wavetable_stride(voice);
    }
}

void test_waveform(audio_context_t* context, waveform_t waveform) {
    set_frequency(context, PITCH_A4, waveform);
    synth_envelope_note_on(&context->envelope);
    sleep_ms(500);
    synth_envelope_note_off(&context->envelope);
    sleep_ms(300);

    set_frequency(context, PITCH_E4, waveform);
    synth_envelope_note_on(&context->envelope);
    sleep_ms(500);
    synth_envelope_note_off(&context->envelope);
    sleep_ms(300);

    set_frequency(context, PITCH_A5, waveform);
    synth_envelope_note_on(&context->envelope);
    sleep_ms(500);
    synth_envelope_note_off(&context->envelope);
    sleep_ms(300);

    set_frequency(context, PITCH_E4, waveform);
    synth_envelope_note_on(&context->envelope);
    sleep_ms(500);
    synth_envelope_note_off(&context->envelope);
    sleep_ms(300);
}

static void test_sweep(audio_context_t* context, waveform_t waveform) {
    synth_envelope_note_on(&context->envelope);
    for (size_t i = PITCH_A4; i < PITCH_A5; i = i + 4) {
        sleep_ms(10);
        set_frequency(context, i, waveform);
    }
    synth_envelope_note_off(&context->envelope);
    sleep_ms(120);
}

static void test_pattern(audio_context_t* context, waveform_t waveform) {
    uint8_t pattern[6] = {0, 1, 3, 5, 3, 1};

    for (int i = 0; i < sizeof(pattern); i++) {
        set_frequency(context,
                      frequency_from_reference_pitch(PITCH_C3, pattern[i]),
                      waveform);
        synth_envelope_note_on(&context->envelope);
        sleep_ms(120);
        synth_envelope_note_off(&context->envelope);
        sleep_ms(220);
    }
}

static void test_midi_pattern(audio_context_t* context, waveform_t waveform) {
    // nicked from the tinyusb example
    uint8_t pattern[] = {74, 78, 81,  86, 90, 93, 98,  102, 57, 61, 66, 69, 73,
                         78, 81, 85,  88, 92, 97, 100, 97,  92, 88, 85, 81, 78,
                         74, 69, 66,  62, 57, 62, 66,  69,  74, 78, 81, 86, 90,
                         93, 97, 102, 97, 93, 90, 85,  81,  78, 73, 68, 64, 61,
                         56, 61, 64,  68, 74, 78, 81,  86,  90, 93, 98, 102};

    for (int i = 0; i < sizeof(pattern); i++) {
        set_frequency(context, synth_midi_frequency_from_midi_note[pattern[i]],
                      waveform);
        synth_envelope_note_on(&context->envelope);
        sleep_ms(120);
        synth_envelope_note_off(&context->envelope);
        sleep_ms(120);
    }
}

void synth_test_play(audio_context_t* context) {
    set_frequency(context, PITCH_C4, context->voices[0].waveform);
    synth_envelope_note_on(&context->envelope);
    sleep_ms(1000);
    synth_envelope_note_off(&context->envelope);
    sleep_ms(1200);

    test_pattern(context, context->voices[0].waveform);

    context->delay = SAMPLE_RATE /4.f;
    context->delay_gain = float2fix16(0.5f);

    test_midi_pattern(context, context->voices[0].waveform);

    context->delay = 0;
    context->delay_gain = 0;

    // test_waveform(context, NOISE);
    // test_waveform(context, TRIANGLE);
    // test_waveform(context, SAW);
    // test_waveform(context, SQUARE);
    // test_waveform(context, SINE);

    // test_sweep(context, NOISE);
    // test_sweep(context, TRIANGLE);
    // test_sweep(context, SAW);
    // test_sweep(context, SQUARE);
    // test_sweep(context, SINE);
}

#endif