#include <stdlib.h>
#include <math.h>

#include "pico/stdlib.h"

#include "include/pitchtable.h"

float frequency_from_reference_pitch(float f, uint8_t pitch) {
    return f * powf(2.f, (float)pitch / 12.f);
}
