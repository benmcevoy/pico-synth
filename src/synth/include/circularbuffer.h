#ifndef SYNTH_CIRCULARBUFFER_
#define SYNTH_CIRCULARBUFFER_

#include "audiocontext.h"
#include "fixedpoint.h"

void synth_circularbuffer_init(void);
fix16 synth_circularbuffer_read();
void synth_circularbuffer_write(fix16 value, uint16_t delay);

#endif
