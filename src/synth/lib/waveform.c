#include "../include/waveform.h"

#include <math.h>
#include <stdio.h>

static uint16_t seed = 17;
static int a = 1;
static int b = 5;
static int c = 2;

static fix16 read_from_wt(voice_t* voice, fix16* waveTable) {
  int16_t indexBelow = fix2int16(voice->wavetable_read_pointer);
  int16_t indexAbove = indexBelow + 1;

  if (indexAbove >= WAVE_TABLE_LENGTH) indexAbove -= WAVE_TABLE_LENGTH;

  fix16 fraction = voice->wavetable_read_pointer - int2fix16(indexBelow);
  fix16 value = lerp(fraction, waveTable[indexBelow], waveTable[indexAbove]);

  voice->wavetable_read_pointer +=
      multfix16(FIX16_WAVE_TABLE_LENGTH, voice->wavetable_stride);

  while (voice->wavetable_read_pointer >= FIX16_WAVE_TABLE_LENGTH)
    voice->wavetable_read_pointer -= FIX16_WAVE_TABLE_LENGTH;

  return value;
}

static fix16 square(voice_t* voice) {
  // TODO: I wanted to have a duty cycle here for pulse width
  // but seems not good when audio is PWM
  // maybe if I use I2S instead?

  fix16 value =
      (voice->wavetable_phase < FIX16_PI) ? FIX16_ONE : FIX16_NEGATIVE_ONE;

  voice->wavetable_phase += multfix16(FIX16_TWOPI, voice->wavetable_stride);

  if (voice->wavetable_phase > FIX16_TWOPI) voice->wavetable_phase -= FIX16_TWOPI;

  return value;
}

// https://en.wikipedia.org/wiki/Xorshift
fix16 synth_waveform_noise() {
  seed ^= (seed << a);
  seed ^= (seed >> b);
  seed ^= (seed << c);
  // just return it as it's random anyways...
  return seed;
}

fix16 synth_waveform_sample(voice_t* voice) {
  switch (voice->waveform) {
    case SINE:
      return read_from_wt(voice, sine_wave_table);
    case SQUARE:
      return square(voice);
    case SAW:
      return read_from_wt(voice, saw_wave_table);
    case TRIANGLE:
      return read_from_wt(voice, triangle_wave_table);
    case NOISE:
      return synth_waveform_noise();

    default:
      return 0;
  }
}
