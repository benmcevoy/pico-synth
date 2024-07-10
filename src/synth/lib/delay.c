#include "../include/delay.h"

#include <stdint.h>

static fix16 buffer[DELAY_BUFFER_SIZE] = {};
static int read_pointer;
static int write_pointer;
static fix16 feedback;

void synth_delay_init() {
  write_pointer = 0;
  read_pointer = 0;
}

void synth_delay_set_delay(delay_t* delay) {
  read_pointer = (write_pointer - delay->delay_in_samples + DELAY_BUFFER_SIZE) %
                 DELAY_BUFFER_SIZE;
  feedback = delay->feedback;
}

fix16 synth_delay_process(delay_t* delay, fix16 in) {
  // not sure, turn the control down to zero and it turns off?
  // seems ok.
  if (!delay->enabled || delay->delay_in_samples == 0) return 0;

  fix16 out = buffer[read_pointer];

  // apply feedback
  fix16 effect = in + multfix16(out, feedback);
  buffer[write_pointer] = effect;

  write_pointer++;
  if (write_pointer >= DELAY_BUFFER_SIZE) write_pointer = 0;

  read_pointer++;
  if (read_pointer >= DELAY_BUFFER_SIZE) read_pointer = 0;

  // at zero should be 100% dry
  // dry*in + wet*out
  fix16 dry = FIX16_ONE - delay->dry_wet_mix;
  return multfix16(dry, in) + multfix16(delay->dry_wet_mix, out);
}
