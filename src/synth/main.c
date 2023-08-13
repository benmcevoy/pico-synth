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

static uint16_t _sampleRate = SAMPLE_RATE;
static uint16_t _buffer0[BUFFER_LENGTH] = {0};
static uint16_t _buffer1[BUFFER_LENGTH] = {0};
static bool _swap = false;
static int _pwmDmaChannel;
static AudioContext_t* _context;
static const fix16 MIXFACTOR = VOICES_LENGTH * FIX16_ONE;
static int _bitDepth = 12;

static void fill_write_buffer() {
    for (int i = 0; i < BUFFER_LENGTH; i++) {
        fix16 amplitude = 0;

        for (int v = 0; v < VOICES_LENGTH; v++) {
            Voice_t* voice = &_context->voices[v];

            fix16 sample = synth_waveform_sample(voice);

            amplitude += sample;
        }

        // ***** ENVELOPE ******
        // envelope modulation
        fix16 envelope = synth_envelope_process(_context);
        amplitude = multfix16(amplitude, envelope);
        // *********************

        // **** DELAY/ECHO *****
        // apply feedback
        amplitude = amplitude +
                    multfix16(synth_circularbuffer_read(), _context->delayGain);
        synth_circularbuffer_write(amplitude, _context->delay);
        // *********************

        // ******* GATE ********
        fix16 tmp = 0;
        for (int g = 0; g < GATES_LENGTH; g++) {
            fix16 gate = synth_envelope_gate(&_context->gates[g]);
            tmp += multfix16(gate, amplitude);
        }
        amplitude = tmp;
        // *********************

        // **** MIX/COMPRESS ***
        // mix/compress - tanh limits to -1..1 with a nice curve,
        // quite an angry distort.  also the float is a performance hit.
        amplitude = float2fix16(tanhf(fix2float16(amplitude)));

        // mix/compress - average - theoretically no clipping or distortion,
        // just quieter
        // amplitude = divfix16(amplitude, MIXFACTOR);

        // mix wet+dry
        // wet and dry are two additional parameters
        // the feedback value above would be
        //
        // fix16 feedback = multfix16(amplitude, dry) +
        // multfix16(synth_circularbuffer_read(), wet);
        //
        // and the delay buffer would have the vlaue above written to it
        // fix16 valueToWriteToDelayBuffer = amplitude +
        // multfix16(synth_circularbuffer_read(), _context->delayGain);
        // *********************

        // apply master volume
        amplitude = multfix16(_context->volume, amplitude);

        // scale to an "about 12" bit signal in a 16-bit container
        // the mix value is between -1..1
        // add 1 to move it 0..2
        // we should now have a number between 0..2 (or really 0..1.99999999999
        // need to scale to the bit depth as determined by the wrap calulated on
        // init
        amplitude = amplitude + FIX16_ONE;
        uint16_t out = (uint16_t)(amplitude >> _bitDepth);

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

    _context->samplesElapsed = 0;
    _context->triggerAttack = false;
    _context->volume = float2fix16(1);
    _context->attack = float2fix16(0.05);
    _context->decay = float2fix16(0.05f);
    _context->sustain = float2fix16(0.5f);
    _context->release = float2fix16(0.1f);
    _context->delay = 0;
    _context->delayGain = 0;

    for (int v = 0; v < VOICES_LENGTH; v++) {
        _context->voices[v].frequency = PITCH_C3;
        _context->voices[v].waveform = SINE;
        _context->voices[v].detune = float2fix16(1 + v * 0.001);
        synth_audiocontext_set_wavetable_stride(&_context->voices[v],
                                                sampleRate);

        printf("wts: %d \n", _context->voices[v].wavetableStride);
    }

    for (int g = 0; g < GATES_LENGTH; g++) {
        _context->gates[g].onDuration = 0;
        _context->gates[g].offDuration = 0;
        _context->gates[g].state = OFF;
        _context->gates[g].remaining = 0;
        _context->gates[g].duration = 0;
    }
}

static uint synth_pwm_init(uint16_t sampleRate) {
    // derive clk_div and wrap
    // set clk_div =1
    uint systemClockHz =
        frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) * 1000;

    uint16_t wrap = systemClockHz / sampleRate;
    printf("wrap: %hu\n", wrap);

    _bitDepth = 16 - floorf(log2(wrap + 1.f));

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

void init_all() {
    synth_audio_context_init(_sampleRate);
    synth_midi_init(_sampleRate);
    synth_circularbuffer_init();
    uint slice = synth_pwm_init(_sampleRate);
    synth_dma_init(slice);

    printf("SampleRate: %d\n", _sampleRate);

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

int main() {
    set_sys_clock_khz(240000, true);
    stdio_init_all();

    printf("\n----------------------\nSynth starting.\n");

    init_all();
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
