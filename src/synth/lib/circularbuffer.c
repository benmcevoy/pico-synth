#include "../include/circularbuffer.h"

#include <stdint.h>

// TODO: hard code to 1 seconds of samples, should maybe be fix16* and then
// malloc when initialised
#define CIRCULARBUFFER_WRAP SAMPLE_RATE

static fix16 _buffer[CIRCULARBUFFER_WRAP] = {};
static int _readPointer;
static int _writePointer;

void synth_circularbuffer_init() {
    _writePointer = 0;
    _readPointer = 0;
}

fix16 synth_circularbuffer_read() { return _buffer[_readPointer]; }

void synth_circularbuffer_write(fix16 value, uint16_t delay) {
    _writePointer++;
    if (_writePointer >= CIRCULARBUFFER_WRAP) _writePointer = 0;

    _readPointer = _writePointer - delay;
    while (_readPointer >= CIRCULARBUFFER_WRAP) _readPointer -= CIRCULARBUFFER_WRAP;
    while (_readPointer < 0) _readPointer += CIRCULARBUFFER_WRAP;

    _buffer[_writePointer] = value;
}
