#include "include/midi.h"

#include <stdio.h>
#include <stdlib.h>

#include "include/envelope.h"
#include "include/filter.h"
#include "tusb.h"

static float _sampleRate;
static float _maxFilterCutoff;
static float _notePriority[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t _notePriorityIndex = 0;

float synth_midi_frequency_from_midi_note(uint8_t note) {
    return PITCH_A4 * powf(2.f, (note - 69) / 12.f);
}

static void note_on(AudioContext_t* context, uint8_t note, uint8_t velocity) {
    float sustain = (float)velocity / 128.f;
    float pitch = synth_midi_frequency_from_midi_note(note);

    for (int i = 0; i < VOICES_LENGTH; i++) {
        Voice_t* voice = &context->voices[i];

        voice->sustain = sustain;
        voice->frequency = pitch;

        synth_envelope_note_on(voice);
    }

    if (_notePriorityIndex < 11) _notePriorityIndex++;

    _notePriority[_notePriorityIndex] = pitch;
}

static void note_off(AudioContext_t* context) {
    float priorPitch = 0.f;

    if (_notePriorityIndex > 0) _notePriorityIndex--;

    priorPitch = _notePriority[_notePriorityIndex];
    _notePriority[_notePriorityIndex] = 0;

    for (int i = 0; i < VOICES_LENGTH; i++) {
        Voice_t* voice = &context->voices[i];

        if (priorPitch == 0.f)
            synth_envelope_note_off(voice);
        else
            voice->frequency = priorPitch;
    }
}

void control_change(AudioContext_t* context, uint8_t command,
                    uint8_t parameter) {
    switch (command) {
        case SYNTH_MIDI_CC_CUTOFF:
            context->filterCutoff = _maxFilterCutoff * (float)parameter / 128.f;
            synth_filter_calculate_coefficients(context->filterCutoff,
                                                context->filterResonance);
            break;
        case SYNTH_MIDI_CC_RESONANCE: {
            float resonance = (float)parameter / 128.f;
            if (resonance < 2.f) resonance++;
            context->filterResonance = resonance;
            synth_filter_calculate_coefficients(context->filterCutoff,
                                                context->filterResonance);
            break;
        }
        default:
            break;
    }
}

static void process_midi_command(AudioContext_t* context, uint8_t packet[4]) {
    uint8_t command = packet[1] & 0b11110000;
    uint8_t channel = packet[1] & 0b00001111;

    //    if (channel >= VOICES_LENGTH) return;

    printf("midi: %d %d %d\n", command, packet[2], packet[3]);

    switch (command) {
        case SYNTH_MIDI_NOTEON:
            note_on(context, packet[2], packet[3]);
            break;
        case SYNTH_MIDI_NOTEOFF:
            note_off(context);
        case SYNTH_MIDI_CC:
            control_change(context, packet[2], packet[3]);
            break;
    }
}

void synth_midi_task(AudioContext_t* context) {
    uint8_t packet[4];
    while (tud_midi_available()) {
        // reads into the packet array
        tud_midi_packet_read(packet);
        process_midi_command(context, packet);
    }
}

void synth_midi_init(float sampleRate) {
    _sampleRate = sampleRate;
    _maxFilterCutoff = 3000.f;
}
