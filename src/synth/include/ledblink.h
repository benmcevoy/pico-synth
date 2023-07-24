#ifndef SYNTH_LEDBLINK_
#define SYNTH_LEDBLINK_

#include <stdlib.h>

#include "bsp/board.h"

enum {
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

static uint32_t synth_led_blink_interval_ms = BLINK_NOT_MOUNTED;

void synth_led_blink_task(void) {
    static uint32_t start_ms = 0;
    static bool led_state = false;

    // Blink every interval ms
    if (board_millis() - start_ms < synth_led_blink_interval_ms)
        return;  // not enough time
    start_ms += synth_led_blink_interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state;  // toggle
}

#endif