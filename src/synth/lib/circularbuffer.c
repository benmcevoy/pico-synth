#include "../include/circularbuffer.h"

#include <stdint.h>

// TODO: hard code to 1 seconds of samples, should maybe be fix16* and then
// malloc when initialised
#define CIRCULARBUFFER_WRAP SAMPLE_RATE

static fix16 buffer[CIRCULARBUFFER_WRAP] = {};
static int readPointer;
static int writePointer;

void synth_circularbuffer_init() {
    writePointer = 0;
    readPointer = 0;
}

fix16 synth_circularbuffer_read() { return buffer[readPointer]; }

void synth_circularbuffer_write(fix16 value, uint16_t delay) {
    writePointer++;
    if (writePointer >= CIRCULARBUFFER_WRAP) writePointer = 0;

    readPointer = writePointer - delay;
    while (readPointer >= CIRCULARBUFFER_WRAP) readPointer -= CIRCULARBUFFER_WRAP;
    while (readPointer < 0) readPointer += CIRCULARBUFFER_WRAP;

    buffer[writePointer] = value;
}
