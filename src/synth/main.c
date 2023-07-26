#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "include/audiocontext.h"
#include "include/envelope.h"
#include "include/filter.h"
#include "include/ledblink.h"
#include "include/midi.h"
#include "include/pitchtable.h"
#include "include/waveform.h"
#include "pico/stdlib.h"
#include "test.h"
#include "tusb.h"
#include "tusb_config.h"

#define PIN 26

static float clk_div = 40.f;

static unsigned short _buffer0[BUFFER_LENGTH] = {0};
static unsigned short _buffer1[BUFFER_LENGTH] = {0};
static bool _swap = false;
static int _pwmDmaChannel;
static AudioContext_t* _context;

static void fill_write_buffer() {
    for (int i = 0; i < BUFFER_LENGTH; i++) {
        float mix = 0.f;

        for (int v = 0; v < VOICES_LENGTH; v++) {
            Voice_t* voice = &_context->Voices[v];

            float sample = synth_waveform_sample(voice);

            // envelope modulation
            float envelope = synth_envelope_process(voice);

            // mix - average
            // TODO: this is naive?
            mix += sample * envelope;
        }

        mix /= (float)VOICES_LENGTH;

        if (_context->filterEnabled) {
            mix = synth_filter_low_pass(_context->filterCutoff,
                                        _context->filterResonance, mix);
        }

        // scale to 8 bit in a 16-bit container
        unsigned short out = (mix + 1.f) * 127.f;

        // final volume
        _context->AudioOut[i] = out * _context->Volume;
        _context->SamplesElapsed++;
    }
}

static void __isr __time_critical_func(dma_irq_handler)() {
    // cycle buffers
    _swap = !_swap;

    // ack irq
    dma_hw->ints0 = 1u << _pwmDmaChannel;
    // restart DMA
    dma_channel_transfer_from_buffer_now(
        _pwmDmaChannel, _swap ? _buffer0 : _buffer1, BUFFER_LENGTH);

    // fill write buffer
    _context->AudioOut = _swap ? _buffer1 : _buffer0;

    fill_write_buffer();
}

static void synth_audio_context_init(float clk_div) {
    // derive sample rate
    uint systemClockHz =
        frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) * 1000;

    _context = malloc(sizeof(AudioContext_t));
    _context->SampleRate = systemClockHz / (float)(clk_div * 255);
    _context->SamplesElapsed = 0;
    _context->Volume = 0.3;

    _context->filterEnabled = true;
    _context->filterCutoff = 1000.f;
    _context->filterResonance = 1.f;

    _context->Voices[0].envelopeState = OFF;
    _context->Voices[0].frequency = 440;
    _context->Voices[0].waveform = SAW;
    _context->Voices[0].attack = 0.05;
    _context->Voices[0].decay = 0.05;
    _context->Voices[0].sustain = 1.f;
    _context->Voices[0].release = 0.5f;
    _context->Voices[0].detune = 1.f;

    _context->Voices[1].envelopeState = OFF;
    _context->Voices[1].frequency = 440;
    _context->Voices[1].waveform = SAW;
    _context->Voices[1].attack = 0.05;
    _context->Voices[1].decay = 0.05;
    _context->Voices[1].sustain = 1.f;
    _context->Voices[1].release = 1.f;
    _context->Voices[1].detune = 2.01f;

    _context->Voices[2].envelopeState = OFF;
    _context->Voices[2].frequency = 440;
    _context->Voices[2].waveform = SAW;
    _context->Voices[2].attack = 0.05;
    _context->Voices[2].decay = 0.05;
    _context->Voices[2].sustain = 1.f;
    _context->Voices[2].release = 0.5f;
    _context->Voices[2].detune = 1.01f;
}

static uint synth_pwm_init(float clk_div) {
    // setup pwm
    gpio_set_function(PIN, GPIO_FUNC_PWM);
    gpio_set_function(PIN + 1, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PIN);
    pwm_config pwmConfig = pwm_get_default_config();

    // set sample rate/clock to 125Mhz / wrap+1 / div =   ~22kHz
    // rp2040 datasheet gives the actual formula

    pwm_config_set_clkdiv(&pwmConfig, clk_div);
    pwm_config_set_wrap(&pwmConfig, 254);
    // pwm_config_set_phase_correct(&pwmConfig, true);
    pwm_init(slice, &pwmConfig, true);

    return slice;
}

static void synth_dma_init(uint slice) {
    // setup dma
    _pwmDmaChannel = dma_claim_unused_channel(true);

    dma_channel_config dmaConfig =
        dma_channel_get_default_config(_pwmDmaChannel);
    // must be 16 bits
    channel_config_set_transfer_data_size(&dmaConfig, DMA_SIZE_16);
    channel_config_set_read_increment(&dmaConfig, true);
    channel_config_set_write_increment(&dmaConfig, false);
    channel_config_set_dreq(&dmaConfig, DREQ_PWM_WRAP0 + slice);

    dma_channel_configure(
        _pwmDmaChannel, &dmaConfig,
        &pwm_hw->slice[slice].cc,  // Write to PWM counter compare
        &_buffer0,                 // read from buffer
        BUFFER_LENGTH,             // number of transfers to perform
        true                       // start
    );

    dma_channel_set_irq0_enabled(_pwmDmaChannel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
}

int main() {
    stdio_init_all();

    printf("Synth starting.\n");
    sleep_ms(100);

    synth_audio_context_init(clk_div);
    synth_envelope_init(_context->SampleRate);
    synth_waveform_init(_context->SampleRate);
    synth_filter_init(_context->SampleRate);
    synth_midi_init(_context->SampleRate);

    uint slice = synth_pwm_init(clk_div);
    synth_dma_init(slice);
    board_init();

    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);

    while (true) {
        // synth_play_test(&_context->Voices[0]);

        tud_task();  // tinyusb device task
        synth_led_blink_task();
        synth_midi_task(_context);
    }
}

// Invoked when device is mounted
void tud_mount_cb(void) { synth_led_blink_interval_ms = BLINK_MOUNTED; }

// Invoked when device is unmounted
void tud_umount_cb(void) { synth_led_blink_interval_ms = BLINK_NOT_MOUNTED; }

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
    synth_led_blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) { synth_led_blink_interval_ms = BLINK_MOUNTED; }
