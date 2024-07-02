#include "../include/delay.h"

#include <stdint.h>

// TODO: hard code to 1 seconds of samples, should maybe be fix16* and then
// malloc when initialised
#define CIRCULARBUFFER_WRAP SAMPLE_RATE

static fix16 buffer[CIRCULARBUFFER_WRAP] = {};
static int read_pointer;
static int write_pointer;

void synth_delay_init() {
  write_pointer = 0;
  read_pointer = 0;
}

static fix16 delay_read() { return buffer[read_pointer]; }

static void delay_write(fix16 value, uint16_t delay) {
  write_pointer++;
  if (write_pointer >= CIRCULARBUFFER_WRAP) write_pointer = 0;

  read_pointer = write_pointer - delay;
  while (read_pointer >= CIRCULARBUFFER_WRAP)
    read_pointer -= CIRCULARBUFFER_WRAP;
  while (read_pointer < 0) read_pointer += CIRCULARBUFFER_WRAP;

  buffer[write_pointer] = value;
}

fix16 synth_delay_process(delay_t* delay, fix16 amplitude) {
  if (!delay->enabled) return amplitude;

  // apply feedback
  amplitude += multfix16(delay_read() >> 1, delay->gain);

  delay_write(amplitude, delay->delay);

  return amplitude;
}
