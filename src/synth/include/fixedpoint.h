#ifndef SYNTH_FIXEDPOINT_
#define SYNTH_FIXEDPOINT_

#include <stdint.h>

#define EPSILON 8

#define FIX16_ONE 65536
#define FIX16_NEGATIVE_ONE 4294901760
#define FIX16_POINT_1 6553
#define FIX16_POINT_2 13107
#define FIX16_POINT_3 19660
#define FIX16_POINT_4 26214
#define FIX16_POINT_5 32768
#define FIX16_POINT_6 39321
#define FIX16_POINT_7 45875
#define FIX16_POINT_8 52428
#define FIX16_POINT_9 58982

#define FIX16_PI 205887
#define FIX16_TWOPI 411774

// TODO: hardware support is for 32x32 with a 32 bit result so we should
// get better performance with int16_t fix14 2.14dp or something
// this means anything using the sample rate probably has to now be uint32_t or similar
// fixed point 16.16dp
typedef int32_t fix16;
#define multfix16(a, b) ((fix16)((((int64_t)(a)) * ((int64_t)(b))) >> 16))
#define float2fix16(a) ((fix16)((a)*FIX16_ONE))
#define fix2float16(a) ((float)(a) / FIX16_ONE)
#define fix2int16(a) ((int16_t)((a) >> 16))
#define int2fix16(a) ((fix16)((a) << 16))
#define divfix16(a, b) ((fix16)((((int64_t)(a) << 16) / (b))))
#define sqrtfix16(a) (float2fix16(sqrt(fix2float16(a))))
#define absfix16(a) abs(a)

#endif