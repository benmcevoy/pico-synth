#include "include/midi.h"

#include <stdio.h>
#include <stdlib.h>

#include "../usb_midi_host.h"
#include "include/envelope.h"
#include "include/fixedpoint.h"
#include "tusb.h"

static uint8_t note_priority[16] = {0};
static uint8_t note_priority_index = 0;

static void note_on(audio_context_t* context, uint8_t note, uint8_t velocity) {
  // scale to the fractional part of fix16
  fix16 gain = velocity << 8;
  fix16 pitch = synth_midi_frequency_from_midi_note[note];

  for (int i = 0; i < VOICES_LENGTH; i++) {
    voice_t* voice = &context->voices[i];

    voice->gain = gain;
    voice->frequency = pitch;
    synth_audiocontext_set_wavetable_stride(voice);
  }

  synth_envelope_note_on(&context->envelope);

  note_priority[note_priority_index] = note;

  if (note_priority_index < 14) note_priority_index++;
}

static void note_off(audio_context_t* context, uint8_t note) {
  bool found = false;

  // most recent note priority
  for (int i = 0; i < note_priority_index; i++) {
    if (note_priority[i] == note || found) {
      found = true;
      note_priority[i] = note_priority[i + 1];
    }
  }

  if (note_priority_index > 0) note_priority_index--;

  if (note_priority_index == 0) {
    synth_envelope_note_off(&context->envelope);
    return;
  }

  note = note_priority[note_priority_index - 1];
  fix16 pitch = synth_midi_frequency_from_midi_note[note];

  for (int i = 0; i < VOICES_LENGTH; i++) {
    voice_t* voice = &context->voices[i];

    voice->frequency = pitch;
    synth_audiocontext_set_wavetable_stride(voice);
  }
}

static void pitch_bend(audio_context_t* context, uint8_t lsb, uint8_t msb) {
  // value maps to =/- two semitones
  int16_t value = (msb << 7) + lsb;
  fix16 scaled = int2fix16(value - 8192);
  fix16 bend = divfix16(divfix16(scaled, FIX16_8192) << 1, FIX16_12);
  fix16 ratio = float2fix16(powf(2.f, fix2float16(bend)));

  for (int i = 0; i < VOICES_LENGTH; i++) {
    voice_t* voice = &context->voices[i];
    voice->pitch_bend = ratio;
    synth_audiocontext_set_wavetable_stride(voice);
  }
}


static void process_midi_command(audio_context_t* context, uint8_t packet[4]) {
  uint8_t command = packet[0] & 0b11110000;
printf("mod wheel: %u %u %u %u\n", packet[0],packet[1],packet[2],packet[3]);      
  switch (command) {
    case SYNTH_MIDI_NOTEON:
      note_on(context, packet[1], packet[2]);
      break;
    case SYNTH_MIDI_NOTEOFF:
      note_off(context, packet[1]);
      break;
    case SYNTH_MIDI_CC_PITCHBEND:
      // 14 bit value 0-16383
      pitch_bend(context, packet[1], packet[2]);
      break;
    case SYNTH_MIDI_CC_MODWHEEL:
      // control_change(context, packet[2], packet[3]);

      break;
  }
}

void synth_midi_init() {}

void synth_midi_task(audio_context_t* context, uint8_t* packet) {
  process_midi_command(context, packet);
}

void synth_midi_panic(audio_context_t* context) {
  synth_envelope_note_off(&context->envelope);
}
