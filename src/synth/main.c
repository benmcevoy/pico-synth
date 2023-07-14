#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"

#include "include/audiocontext.h"
#include "include/waveform.h"

#define PIN 26

unsigned short _buffer1[BUFFER_SIZE] = {0};
unsigned short _buffer2[BUFFER_SIZE] = {0};
bool _swap = true;
int _pwmDmaChannel;

AudioContext_t *_context;

void synth_fillbuffer()
{
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        double sample = synth_waveform_sample(_context, TRIANGLE, 440, 0.3);

        // scale to 8-bit
        char value = (sample + 1.0) * 127;

        (*_context->AudioOut)[i] = value;

        _context->SamplesElapsed += 1;
    }
}

void synth_dma_irq_handler()
{
    // ack irq
    dma_hw->ints0 = 1u << _pwmDmaChannel;

    // swap buffers
    _swap = !_swap;

    _context->AudioOut = _swap ? &_buffer2 : &_buffer1;

    // restart DMA
    dma_channel_transfer_from_buffer_now(_pwmDmaChannel, _swap ? &_buffer1 : &_buffer2, BUFFER_SIZE);

    // fill write buffer
    synth_fillbuffer();
}

int main()
{
    stdio_init_all();
    // sleep_ms(1000);
    printf("Starting main\n");

    _context = malloc(sizeof(AudioContext_t));

    // setup pwm
    gpio_set_function(PIN, GPIO_FUNC_PWM);
    gpio_set_function(PIN + 1, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PIN);
    pwm_config pwmConfig = pwm_get_default_config();

    // set sample rate/clock to 125Mhz / wrap+1 / div =   ~22kHz
    // rp2040 datasheet gives the actual formula
    pwm_config_set_clkdiv(&pwmConfig, 11.f);
    pwm_config_set_wrap(&pwmConfig, 254);
    pwm_config_set_phase_correct(&pwmConfig, true);
    pwm_init(slice, &pwmConfig, true);

    // derive sample rate
    uint systemClockHz = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) * 1000;
    _context->SampleRate = systemClockHz / (11 * 2 * 255);

    // setup dma
    _pwmDmaChannel = dma_claim_unused_channel(true);

    dma_channel_config dmaConfig = dma_channel_get_default_config(_pwmDmaChannel);
    // must be 16 bits
    channel_config_set_transfer_data_size(&dmaConfig, DMA_SIZE_16);
    channel_config_set_read_increment(&dmaConfig, true);
    channel_config_set_write_increment(&dmaConfig, false);
    channel_config_set_dreq(&dmaConfig, DREQ_PWM_WRAP0 + slice);

    dma_channel_configure(
        _pwmDmaChannel,
        &dmaConfig,
        &pwm_hw->slice[slice].cc, // Write to PWM counter compare
        NULL,                     // read from buffer
        BUFFER_SIZE,              // number of transfers to perform
        true                      // start
    );

    dma_channel_set_irq0_enabled(_pwmDmaChannel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, synth_dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    while (true)
        tight_loop_contents();
}
