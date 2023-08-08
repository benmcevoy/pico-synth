#ifndef SYNTH_CIRCULARBUFFER_
#define SYNTH_CIRCULARBUFFER_

#include "fixedpoint.h"

void synth_circularbuffer_init(uint16_t sampleRate);
fix16 synth_circularbuffer_read();
void synth_circularbuffer_write(fix16 value, uint16_t delay);

#endif
