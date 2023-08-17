#include "include/midi.h"

#include <stdio.h>
#include <stdlib.h>

#include "include/envelope.h"
#include "include/fixedpoint.h"
#include "tusb.h"

static uint16_t _sampleRate = 0;
static uint8_t _notePriority[16] = {0};
static uint8_t _notePriorityIndex = 0;

static void note_on(AudioContext_t* context, uint8_t note, uint8_t velocity) {
    fix16 gain = velocity << 9;
    fix16 pitch = synth_midi_frequency_from_midi_note[note];

    for (int i = 0; i < VOICES_LENGTH; i++) {
        Voice_t* voice = &context->voices[i];

        voice->gain = gain;
        voice->frequency = pitch;
        synth_audiocontext_set_wavetable_stride(voice);
    }

    synth_envelope_note_on(&context->envelope);

    _notePriority[_notePriorityIndex] = note;

    if (_notePriorityIndex < 14) _notePriorityIndex++;
}

static void note_off(AudioContext_t* context, uint8_t note) {
    bool found = false;

    // most recent note priority
    for (int i = 0; i < _notePriorityIndex; i++) {
        if (_notePriority[i] == note || found) {
            found = true;
            _notePriority[i] = _notePriority[i + 1];
        }
    }

    if (_notePriorityIndex > 0) _notePriorityIndex--;

    if (_notePriorityIndex == 0) {
        synth_envelope_note_off(&context->envelope);
        return;
    }

    note = _notePriority[_notePriorityIndex - 1];
    fix16 pitch = synth_midi_frequency_from_midi_note[note];

    for (int i = 0; i < VOICES_LENGTH; i++) {
        Voice_t* voice = &context->voices[i];

        voice->frequency = pitch;
        synth_audiocontext_set_wavetable_stride(voice);
    }
}

void control_change(AudioContext_t* context, uint8_t command,
                    uint8_t parameter) {
    switch (command) {
        case SYNTH_MIDI_CC_VOLUME: {
            // 7 bit number needs to end up in bits 9-15
            fix16 value = parameter << 9;
            context->gain = value;
            break;
        }

        case SYNTH_MIDI_CC_CUTOFF: {
            fix16 value = parameter << 9;
            context->delayGain = value;
            break;
        }

        case SYNTH_MIDI_CC_RESONANCE: {
            uint16_t value = _sampleRate * parameter / 128.f;
            context->delay = value;
            break;
        }

        case SYNTH_MIDI_CC_ATTACK: {
            break;
        }

        case SYNTH_MIDI_CC_RELEASE: {
            break;
        }

        case SYNTH_MIDI_CC_MODWHEEL: {
            break;
        }
        
        default:
            break;
    }
}

static void process_midi_command(AudioContext_t* context, uint8_t packet[4]) {
    uint8_t command = packet[1] & 0b11110000;

    // printf("midi: %d %d %d %d %d\n", command,
    // packet[0],packet[1],packet[2],packet[3]);

    switch (command) {
        case SYNTH_MIDI_NOTEON:
            note_on(context, packet[2], packet[3]);
            break;
        case SYNTH_MIDI_NOTEOFF:
            note_off(context, packet[2]);
            break;
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

void synth_midi_init(uint16_t sampleRate) { _sampleRate = sampleRate; }
