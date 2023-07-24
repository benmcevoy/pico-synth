#include "include/midi.h"

#include <stdio.h>
#include <stdlib.h>

#include "include/envelope.h"

float synth_midi_frequency_from_midi_note(uint8_t n) {
    return PITCH_A4 * powf(2.f, (n - 69.f) / 12.f);
}

void note_on(AudioContext_t* context, uint8_t note, uint8_t velocity) {
    context->Voice.sustain = (float)velocity / 128.f;
    context->Voice.frequency = synth_midi_frequency_from_midi_note(note);

    synth_envelope_note_on(&context->Voice);
}

void note_off(AudioContext_t* context) {
    synth_envelope_note_off(&context->Voice);
}

void process_midi_command(AudioContext_t* context, uint8_t packet[4]) {
    uint8_t command = packet[1] & 0b11110000;

    switch (command) {
        case SYNTH_MIDI_NOTEON:
            note_on(context, packet[2], packet[3]);
            printf("note_on\n");
            break;
        case SYNTH_MIDI_NOTEOFF:
            note_off(context);
            printf("note_off");
        default:
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
