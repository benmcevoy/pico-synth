#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "include/audiocontext.h"
#include "include/circularbuffer.h"
#include "include/envelope.h"
#include "include/fixedpoint.h"
#include "include/ledblink.h"
#include "include/midi.h"
#include "include/pitchtable.h"
#include "include/waveform.h"
#include "pico/stdlib.h"
#include "test.h"
#include "tusb.h"
#include "tusb_config.h"

#define PIN 26

// uncomment to use midi or comment out for the test code
#define USE_MIDI

static uint16_t _sampleRate = 48000;
static uint16_t _buffer0[BUFFER_LENGTH] = {0};
static uint16_t _buffer1[BUFFER_LENGTH] = {0};
static bool _swap = false;
static int _pwmDmaChannel;
static AudioContext_t* _context;
static const fix16 MIXFACTOR = VOICES_LENGTH * FIX16_UNIT;

static void fill_write_buffer() {
    for (int i = 0; i < BUFFER_LENGTH; i++) {
        fix16 mix = 0;

        for (int v = 0; v < VOICES_LENGTH; v++) {
            Voice_t* voice = &_context->voices[v];

            fix16 sample = synth_waveform_sample(voice);

            mix += sample;
        }

        // envelope modulation
        fix16 envelope = synth_envelope_process(_context);

        // apply envelope
        fix16 amplitude = multfix16(envelope, mix);

        // apply feedback
        fix16 feedback = amplitude +
            multfix16(synth_circularbuffer_read(),
                      _context->delayGain);

        // limits to -1..1 with a nice curve, nicer mixing
        feedback = float2fix16(tanhf(fix2float16(feedback)));

        synth_circularbuffer_write(feedback, _context->delay);

        // apply master volume
        amplitude = multfix16(_context->volume, feedback);

        // scale to an 8 bit volume in a 16-bit container
        // the mix value is between -1..1
        // add 1 to move it 0..2
        amplitude = amplitude + FIX16_UNIT;

        // we should now have a number between 0..2 (or really 0..1.99999999999
        // etc) scale to 8 bits by shifting >> 9  and then cast to uint16
        // shifting by 9 moves the first whole number (at bit 16), which is a
        // zero or a one into the top of the lower byte at bit 7
        uint16_t out = (uint16_t)(amplitude >> 9);

        // final volume
        _context->envelope = envelope;
        _context->audioOut[i] = out;
        _context->samplesElapsed++;
    }
}

static void __isr __time_critical_func(dma_irq_handler)() {
    // cycle buffers
    _context->audioOut = _swap ? _buffer0 : _buffer1;
    _swap = !_swap;

    // ack irq
    dma_hw->ints0 = 1u << _pwmDmaChannel;
    // restart DMA
    dma_channel_transfer_from_buffer_now(
        _pwmDmaChannel, _swap ? _buffer0 : _buffer1, BUFFER_LENGTH);

    fill_write_buffer();
}

static void synth_audio_context_init(uint16_t sampleRate) {
    _context = malloc(sizeof(AudioContext_t));

    _context->sampleRate = sampleRate;
    _context->samplesElapsed = 0;
    _context->volume = float2fix16(1);
    _context->attack = float2fix16(0.05);
    _context->decay = float2fix16(0.05f);
    _context->sustain = float2fix16(0.5f);
    _context->release = float2fix16(0.5f);
    _context->delay = 0;
    _context->delayGain = float2fix16(0.5f);

    for (int v = 0; v < VOICES_LENGTH; v++) {
        _context->voices[v].frequency = PITCH_C3;
        _context->voices[v].waveform = SINE;
        _context->voices[v].detune = 1.f + v * 0.01;
        synth_audiocontext_set_wavetable_stride(&_context->voices[v],
                                                _context->sampleRate);
    }
}

static uint synth_pwm_init(uint16_t sampleRate) {
    // derive clk_div and wrap
    // set clk_div =1
    uint systemClockHz =
        frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) * 1000;

    uint16_t wrap = systemClockHz / sampleRate;
    printf("wrap: %hu\n", wrap);

    // setup pwm
    gpio_set_function(PIN, GPIO_FUNC_PWM);
    gpio_set_function(PIN + 1, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PIN);
    pwm_config pwmConfig = pwm_get_default_config();

    // set sample rate/clock to 125Mhz / wrap+1 / div =   ~22kHz
    // rp2040 datasheet gives the actual formula

    pwm_config_set_clkdiv(&pwmConfig, 1);
    pwm_config_set_wrap(&pwmConfig, wrap);
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
    set_sys_clock_khz(240000, true);
    stdio_init_all();

    printf("\n----------------------\nSynth starting.\n");

    synth_audio_context_init(_sampleRate);
    synth_envelope_init(_sampleRate);
    synth_midi_init(_sampleRate);
    synth_waveform_init();
    synth_circularbuffer_init(_sampleRate);
    uint slice = synth_pwm_init(_sampleRate);
    synth_dma_init(slice);

    printf("SampleRate: %d\n", _context->sampleRate);

#ifdef USE_MIDI
    board_init();
    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);
#endif

    while (true) {
#ifndef USE_MIDI
        synth_test_play(_context);
#else
        tud_task();  // tinyusb device task
        synth_led_blink_task();
        synth_midi_task(_context);
#endif
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
