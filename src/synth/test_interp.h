#ifndef SYNTH_TEST_INTERP_
#define SYNTH_TEST_INTERP_

#include <stdio.h>
#include <stdlib.h>

#include "hardware/interp.h"
#include "hardware/structs/systick.h"
#include "include/fixedpoint.h"

// #include "include/lerp.h"
void synth_lerp_init() {
    interp_config cfg = interp_default_config();
    interp_config_set_blend(&cfg, true);
    interp_set_config(interp0, 0, &cfg);

    cfg = interp_default_config();
    interp_set_config(interp0, 1, &cfg);
}

fix16 synth_lerp(fix16 fraction, fix16 a, fix16 b) {
    interp0->base[0] = a;
    interp0->base[1] = b;
    // move bits 9-16 of the fix16 to bits 0-8
    // this can/should be done in the hardware, it has a shift and mask for this
    interp0->accum[1] = fraction >> 8;

    return interp0->peek[1];
}
// #end include/lerp.h

static fix16 linear_easing(fix16 remain, fix16 duration, fix16 start,
                           fix16 end) {
    fix16 elapsed = divfix16((duration - remain), duration);
    return elapsed >= FIX16_ONE ? end
                                : start + multfix16(elapsed, (end - start));
}

static fix16 lerpfix16(fix16 remain, fix16 duration, fix16 start, fix16 end) {
    fix16 elapsed = divfix16((duration - remain), duration);
    return elapsed >= FIX16_ONE ? end : synth_lerp(elapsed, start, end);
}

void test_interp() {
    systick_hw->cvr = 0;    // clear the count to force initial reload
    systick_hw->csr = 0x1;  // Enable Systic

    fix16 a = float2fix16(0.4);
    fix16 b = float2fix16(143.8);

#define LOOP 1000

    fix16 duration = float2fix16(LOOP);
    fix16 remain = duration;

    synth_lerp_init();

   

    uint32_t start1 = systick_hw->cvr;

    for (int j = 0; j < 10000; j++) {
        duration = float2fix16(LOOP);
        remain = duration;

        for (int i = 0; i <= LOOP; i++) {
            fix16 value = lerpfix16(remain, duration, a, b);
            // printf("lerpfix16: %f\n", fix2float16(value));

            remain -= FIX16_ONE;
        }
    }
    uint32_t end1 = systick_hw->cvr;

 systick_hw->cvr = 0; 

 uint32_t start2 = systick_hw->cvr;
    for (int j = 0; j < 10000; j++) {
        duration = float2fix16(LOOP);
        remain = duration;

        for (int i = 0; i <= LOOP; i++) {
            fix16 value = linear_easing(remain, duration, a, b);
            
            // printf("linear_easing: %f\n", fix2float16(value));

            remain -= FIX16_ONE;
        }
    }

    uint32_t end2 = systick_hw->cvr;

    // printf("%d\n", start1);
    // printf("%d\n", end1);

    // so this logs as the hardware version being slower than fixed point??
    // including the shift >>8 might help
    // no doubt i am measuring it wrong
    // but there's not much in it... the code way is * >> + which are all fast
    // if comparing to float I asume it is way faster
    // printf("%d\n", start2);
    // printf("%d\n", end2);

    printf("hw: %d\n", (start1 - end1));
    printf("sw: %d\n", (start2 - end2));


}

#endif
