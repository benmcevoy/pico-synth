#include "pico/stdlib.h"
#include <stdio.h>
#include <math.h>
#include "pico/time.h"
#include "pico/multicore.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"

#define PI 3.14159265358979323846
#define PIN 26
#define BUFFER_SIZE 1024

volatile uint64_t _samplesElapsed = 0;
volatile uint64_t _samplesElapsed1 = 0;
char _sample = 0;
unsigned char _buffer1[BUFFER_SIZE];
unsigned char _buffer2[BUFFER_SIZE];
volatile bool _swap = false;
int _f = 440;
uint _index = 0;

void fillBuffer()
{
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        char value = (sin(PI * 2.0 * _f * (_samplesElapsed1 + i) / 21371) + 1.0) * 128;

        if (_swap)
        {
            _buffer2[i] = value;
        }
        else
        {
            _buffer1[i] = value;
        }
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
        _samplesElapsed1 = _samplesElapsed;
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
    // period = (wrap+1)*(div + 15/16)
    //

    pwm_config_set_clkdiv(&config, 11.f);
    pwm_config_set_wrap(&config, 254);
    pwm_config_set_phase_correct(&config, true);
    pwm_init(slice, &config, true);

    // no pop
    pwm_set_gpio_level(PIN, 0);
    pwm_set_gpio_level(PIN + 1, 0);

    while (true)
        tight_loop_contents();
}
