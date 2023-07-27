#include "include/midi.h"

#include <stdio.h>
#include <stdlib.h>

#include "include/envelope.h"
#include "include/filter.h"
#include "tusb.h"

static float _sampleRate;
static float _maxFilterCutoff;
static float _notePriority[12] = {0};
static uint8_t _notePriorityIndex = 0;

float synth_midi_frequency_from_midi_note(uint8_t note) {
    return PITCH_A4 * powf(2.f, (note - 69) / 12.f);
}

static void note_on(AudioContext_t* context, uint8_t note, uint8_t velocity) {
    float sustain = (float)velocity / 128.f;
    float pitch = synth_midi_frequency_from_midi_note(note);

    context->sustain = sustain;

    for (int i = 0; i < VOICES_LENGTH; i++) {
        Voice_t* voice = &context->voices[i];

        voice->frequency = pitch;
        voice->wavetableStride =
            (voice->frequency * voice->detune) / _sampleRate;
    }

    synth_envelope_note_on(context);

    if (_notePriorityIndex < 11) _notePriorityIndex++;

    _notePriority[_notePriorityIndex] = pitch;
}

static void note_off(AudioContext_t* context) {
    float priorPitch = 0.f;

    if (_notePriorityIndex > 0) _notePriorityIndex--;

    priorPitch = _notePriority[_notePriorityIndex];
    _notePriority[_notePriorityIndex] = 0;

    if (priorPitch == 0.f) {
        synth_envelope_note_off(context);
        return;
    }

    for (int i = 0; i < VOICES_LENGTH; i++) {
        Voice_t* voice = &context->voices[i];

        voice->frequency = priorPitch;
        voice->wavetableStride =
            (voice->frequency * voice->detune) / _sampleRate;
    }
}

void control_change(AudioContext_t* context, uint8_t command,
                    uint8_t parameter) {
    switch (command) {
        case SYNTH_MIDI_CC_CUTOFF: {
            float filterCutoff = (float)parameter / 128.f;
            if (filterCutoff < 0.01f) filterCutoff = 0.01f;
            context->filterCutoff = _maxFilterCutoff * filterCutoff;
            synth_filter_calculate_coefficients(context->filterCutoff,
                                                context->filterResonance);
            break;
        }
        case SYNTH_MIDI_CC_RESONANCE: {
            float resonance = (float)parameter / 128.f;
            if (resonance < 0.01f) resonance = 0.01f;
            context->filterResonance = resonance;
            synth_filter_calculate_coefficients(context->filterCutoff,
                                                context->filterResonance);
            break;
        }
        case SYNTH_MIDI_CC_VOLUME:
            context->volume = (float)parameter / 128.f;
            break;

        case SYNTH_MIDI_CC_MODWHEEL: {
            float detune = (float)parameter / 64.f;

            context->voices[1].detune = detune;
            context->voices[1].wavetableStride =
                (context->voices[1].frequency * context->voices[1].detune) /
                _sampleRate;
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
    _maxFilterCutoff = 2000.f;
}
