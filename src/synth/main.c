#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/multicore.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define PI 3.14159265358979323846
#define PIN 26
#define BUFFER_SIZE 256

uint64_t _samplesElapsed = 0;
unsigned char _sample = 0;
unsigned char _buffer1[BUFFER_SIZE];
unsigned char _buffer2[BUFFER_SIZE];
bool _swap = false;
uint _f = 440;
uint _index = 0;

typedef struct AudioContext
{
    unsigned char (*AudioOut)[BUFFER_SIZE];
    uint64_t SamplesElapsed;
    uint SampleRate;
} AudioContext_t;

AudioContext_t *Context;

void fillBuffer()
{
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        char value = (sin(PI * 2.0 * _f * (Context->SamplesElapsed + i) / Context->SampleRate) + 1.0) * 128;

        (*Context->AudioOut)[i] = value;
    }
}

void onPwmInterrupt()
{
    pwm_clear_irq(pwm_gpio_to_slice_num(PIN));

    if (_samplesElapsed % BUFFER_SIZE == 0)
    {
        _swap = !_swap;
        _index = 0;
        // TODO: trigger fill IRQ
        Context->SamplesElapsed = _samplesElapsed;
        Context->AudioOut = _swap ? &_buffer2 : &_buffer1;

        multicore_reset_core1();
        multicore_launch_core1(fillBuffer);
    }

    _sample = _swap ? _buffer1[_index] : _buffer2[_index];

    _index++;
    _samplesElapsed++;

    pwm_set_gpio_level(PIN, _sample);
    pwm_set_gpio_level(PIN + 1, _sample);
}

int main()
{
    stdio_init_all();
    sleep_ms(1000);
    printf("Starting main\n");

    Context = malloc(sizeof(AudioContext_t));

    gpio_set_function(PIN, GPIO_FUNC_PWM);
    gpio_set_function(PIN + 1, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PIN);

    // Mask our slice's IRQ output into the PWM block's single interrupt line,
    // and register our interrupt handler
    pwm_clear_irq(slice);
    pwm_set_irq_enabled(slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, onPwmInterrupt);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_config config = pwm_get_default_config();

    // set sample rate/clock to 125Mhz / wrap / div =   ~22kHz
    // rp2040 datasheet gives the actual formula, ignoring the phase correction bit
    // period = (wrap+1)*(div + fraction)
    // fraction is 4 bit resolution hence they divide by 16 in the datasheet

    pwm_config_set_clkdiv(&config, 11.f);
    pwm_config_set_wrap(&config, 254);
    pwm_config_set_phase_correct(&config, true);
    pwm_init(slice, &config, true);

    uint systemClockHz = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) * 1000;
    // to my ear this sounds exactly like the wikipedia reference sample @440Hz
    // no discernable beat frequency
    // assume 125Mhz sys_clk gives us ~22281.64Hz

    Context->SampleRate = systemClockHz / (11 * 2 * 255);

    // no pop
    pwm_set_gpio_level(PIN, 0);
    pwm_set_gpio_level(PIN + 1, 0);

    while (true)
        tight_loop_contents();
}
