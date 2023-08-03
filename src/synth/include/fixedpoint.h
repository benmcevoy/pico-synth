#ifndef SYNTH_FIXEDPOINT_
#define SYNTH_FIXEDPOINT_

#include <stdint.h>

#define FIX16_UNIT 65536
#define FIX30_UNIT 1073741824

#define FIX30_TWO 2147483648
#define FIX30_HALF 536870912

// fixed point 16.16dp
typedef int32_t fix16;
#define multfix16(a, b) ((fix16)((((int64_t)(a)) * ((int64_t)(b))) >> 16))
#define float2fix16(a) ((fix16)((a)*FIX16_UNIT))
#define fix2float16(a) ((float)(a) / FIX16_UNIT)
#define fix2int16(a) ((int16_t)((a) >> 16))
#define int2fix16(a) ((fix16)((a) << 16))
#define divfix16(a, b) ((fix16)((((int64_t)(a) << 16) / (b))))
#define sqrtfix16(a) (float2fix16(sqrt(fix2float16(a))))
#define absfix16(a) abs(a)

// fixed point 2.30dp
typedef int32_t fix30;
#define multfix30(a, b) ((fix30)((((int64_t)(a)) * ((int64_t)(b))) >> 30))
#define divfix30(a, b) ((fix30)((((int64_t)(a) << 30) / (b))))
#define float2fix30(a) ((fix30)((a)*FIX30_UNIT))
#define fix2float30(a) ((float)(a) / FIX30_UNIT)

#endif