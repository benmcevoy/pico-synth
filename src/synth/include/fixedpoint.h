#ifndef SYNTH_FIXEDPOINT_
#define SYNTH_FIXEDPOINT_

#include <stdint.h>

#define FIX16_ONE 65536
#define FIX16_NEGATIVE_ONE -65536
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
#define FIX16_2 131072
#define FIX16_6 393216
#define FIX16_8 524288
#define FIX16_12 786432
#define FIX16_1_POINT_8 117964
#define FIX16_1_POINT_386249 90849
#define FIX16_PI_2 102943
#define FIX16_8192 536870912

// TODO: hardware support is for 32x32 with a 32 bit result so we should
// get better performance with int16_t fix14 2.14dp or something
// this means anything using the sample rate probably has to now be uint32_t or
// similar 

// fixed point 16.16dp
typedef int32_t fix16;
#define multfix16(a, b) ((fix16)((((int64_t)(a)) * ((int64_t)(b))) >> 16))
#define float2fix16(a) ((fix16)((a) * FIX16_ONE))
#define fix2float16(a) ((float)(a) / FIX16_ONE)
#define fix2int16(a) ((int16_t)((a) >> 16))
#define int2fix16(a) ((fix16)((a) << 16))
#define divfix16(a, b) ((fix16)((((int64_t)(a) << 16) / (b))))
#define sqrtfix16(a) (float2fix16(sqrt(fix2float16(a))))
#define absfix16(a) abs(a)

static inline fix16 lerpfix16(fix16 fraction, fix16 start, fix16 end) {
  return fraction >= FIX16_ONE ? end
                               : start + multfix16(fraction, (end - start));
}

static inline fix16 tanhfix16(fix16 x) {
#define FIX16_27 1769472
#define FIX16_9 589824

  fix16 x2 = multfix16(x, x);

  return multfix16(
      x, divfix16((FIX16_27 + x2), (FIX16_27 + multfix16(FIX16_9, x2))));
}

static inline fix16 sinfix16(fix16 x)
{
    const fix16 B = 83443;
    const fix16 C = -26560;

    fix16 y = multfix16(B, x) + multfix16(multfix16(C, x), absfix16(x));

    return y;
}

#endif