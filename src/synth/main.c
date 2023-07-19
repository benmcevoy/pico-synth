#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"

#include "include/audiocontext.h"
#include "include/waveform.h"
#include "include/pitchtable.h"

#include "test.h"

#define PIN 26

unsigned short _buffer0[BUFFER_LENGTH] = {0};
unsigned short _buffer1[BUFFER_LENGTH] = {0};
bool _swap = false;
int _pwmDmaChannel;
AudioContext_t *_context;

void synth_fill_write_buffer()
{
    for (int i = 0; i < BUFFER_LENGTH; i++)
    {
        float sample = synth_waveform_sample(_context);

        // scale to 8 bit in a 16-bit container
        unsigned short value = (sample + 1.f) * 127.f;

        _context->AudioOut[i] = value * _context->Volume;
        _context->SamplesElapsed++;
    }
}

static void __isr __time_critical_func(synth_dma_irq_handler)()
{
    // cycle buffers
    _swap = !_swap;

    // ack irq
    dma_hw->ints0 = 1u << _pwmDmaChannel;
    // restart DMA
    dma_channel_transfer_from_buffer_now(_pwmDmaChannel, _swap ? _buffer0 : _buffer1, BUFFER_LENGTH);

    // fill write buffer
    _context->AudioOut = _swap ? _buffer1 : _buffer0;
    synth_fill_write_buffer();
}

int main()
{
    stdio_init_all();

    _context = malloc(sizeof(AudioContext_t));

    // setup pwm
    gpio_set_function(PIN, GPIO_FUNC_PWM);
    gpio_set_function(PIN + 1, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PIN);
    pwm_config pwmConfig = pwm_get_default_config();

    // set sample rate/clock to 125Mhz / wrap+1 / div =   ~22kHz
    // rp2040 datasheet gives the actual formula
    float clk_div = 8.f;
    pwm_config_set_clkdiv(&pwmConfig, clk_div);
    pwm_config_set_wrap(&pwmConfig, 254);
    // pwm_config_set_phase_correct(&pwmConfig, true);
    pwm_init(slice, &pwmConfig, true);

    // derive sample rate
    uint systemClockHz = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) * 1000;
    _context->SampleRate = systemClockHz / (double)(clk_div * 255);
    _context->SamplesElapsed = 0;
    _context->Volume = 0.3;
    _context->Voice.frequency = 440;
    _context->Voice.waveform = SAW;

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
        &_buffer0,                // read from buffer
        BUFFER_LENGTH,            // number of transfers to perform
        true                      // start
    );

    dma_channel_set_irq0_enabled(_pwmDmaChannel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, synth_dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    while (true)
    {
        synth_play_test(_context);
    }
}
