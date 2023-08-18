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
#include "include/metronome.h"
#include "include/midi.h"
#include "include/pitchtable.h"
#include "include/tempo.h"
#include "include/waveform.h"
#include "pico/multicore.h"
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
static int _bitDepth = 12;

fix16 _previousDither = 0;
static fix16 dither() {
    // mask off the bottom 8 bits
    uint8_t p = synth_waveform_noise() & 0x00000ff;

    // TODO: not sure abouth this.  guide is to use 2 LSB of dither
    // not sure if that is 2 LSB on this 32 bit number or on the 12 bit quantized value later?
    // I *think* this setting of _bitDepth + 3 sounds better? but I could be deluding myself
    // I looked at some frequency spectrums with this on/off and various settings
    // the noise floor is raised and skewed to higher frequency
    // might be more noticeable with I2S .  have to wait and see.
    uint8_t depth = _bitDepth + 3;
    fix16 dither;

    // these numbers were derived by pissing around in a spreadsheet
    // apply triangular probability
    if (p < 4) dither = -57344 >> depth;            
    else if (p < 11) dither = -49152 >> depth;      
    else if (p < 21) dither = -40960 >> depth;
    else if (p < 36) dither = -32768 >> depth;
    else if (p < 53) dither = -24576 >> depth;
    else if (p < 75) dither = -16384 >> depth;
    else if (p < 100) dither = -8192 >> depth;
    // from here (>=100)
    else if (p < 128) dither = 0;
    else if (p < 156) dither = 0;
    // to the above accounts for about 22% (56/256)
    else if (p < 181) dither = 8192 >> depth;
    else if (p < 203) dither = 16384 >> depth;
    else if (p < 220) dither = 24576 >> depth;
    else if (p < 235) dither = 32768 >> depth;
    else if (p < 245) dither = 40960 >> depth;
    else if (p < 252) dither = 49152 >> depth;
    else if (p < 256) dither = 57344 >> depth;

    // noise shaping
    fix16 result = dither - _previousDither;

    _previousDither = dither;

    return result;
}

static void fill_write_buffer() {
    for (int i = 0; i < BUFFER_LENGTH; i++) {
        fix16 amplitude = 0;

        for (int v = 0; v < VOICES_LENGTH; v++) {
            Voice_t* voice = &_context->voices[v];

            fix16 sample = synth_waveform_sample(voice);

            amplitude += multfix16(voice->gain, sample);
        }

        synth_tempo_process(&_context->tempo);

        // ***** ENVELOPE ******
        fix16 envelope = synth_envelope_process(&_context->envelope);
        amplitude = multfix16(amplitude, envelope);
        // *********************

        // **** DELAY/ECHO *****
        // apply feedback
        amplitude = amplitude +
                    multfix16(synth_circularbuffer_read(), _context->delayGain);
        synth_circularbuffer_write(amplitude, _context->delay);
        // *********************

        amplitude += synth_metronome_process(&_context->tempo);

        // apply master gain
        // i have preserved the mix as just summation
        // gain can scale it down to avoid clipping
        amplitude = multfix16(_context->gain, amplitude);
       
        // TODO: not sure I need to introduce noise when I use a breadboard :)
        // if/when I can get control of the sample rate/bit depth again
        // i'll drop it down to 8 or 6 bits (thanks Tony)
        // and hopefully the effect will be more obvious
        // currently I cannot tell the difference
        //amplitude += dither(amplitude);

        // compression, if the amplitude is with -1..1 then compress should not
        // effect too much
        // TODO: remove float
        amplitude = float2fix16(tanhf(fix2float16(amplitude)));

        // scale to an "about 12" bit signal in a 16-bit container
        // the mix value is between -1..1
        // add 1 to move it 0..2
        // we should now have a number between 0..2 (or really 0..1.99999999999
        // need to scale to the bit depth as determined by the wrap calulated on
        // init
        amplitude = amplitude + FIX16_ONE;
        uint16_t out = (uint16_t)(amplitude >> _bitDepth);

        // final volume
        _context->envelope.envelope = envelope;
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

static uint synth_pwm_init(uint16_t sampleRate) {
    // derive clk_div and wrap
    // keep clk_div at 1
    uint systemClockHz =
        frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) * 1000;

    uint16_t wrap = systemClockHz / sampleRate;
    _bitDepth = 16 - floorf(log2f(wrap + 1));

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

static void synth_audio_context_init(uint16_t sampleRate) {
    _context = malloc(sizeof(AudioContext_t));

    _context->samplesElapsed = 0;
    _context->gain = float2fix16(1);
    _context->delay = 0;
    _context->delayGain = 0;

    _context->envelope.state = OFF;
    _context->envelope.elapsed = 0;
    _context->envelope.duration = 0;
    _context->envelope.triggerAttack = false;
    _context->envelope.attack = synth_audiocontext_to_duration(0.05);
    _context->envelope.decay = synth_audiocontext_to_duration(0.05f);
    _context->envelope.sustain = float2fix16(0.5f);
    _context->envelope.release = synth_audiocontext_to_duration(0.2f);

    for (int v = 0; v < VOICES_LENGTH; v++) {
        _context->voices[v].gain = float2fix16(0.5f);
        _context->voices[v].frequency = PITCH_C3;
        _context->voices[v].waveform = SAW;
        _context->voices[v].detune = float2fix16(1 + v * 0.001);
        synth_audiocontext_set_wavetable_stride(&_context->voices[v]);
    }

    // fat detune drop octave
    //_context->voices[1].detune = float2fix16(0.5);
}

void init_all() {
    synth_audio_context_init(_sampleRate);
    synth_tempo_init(&_context->tempo, 120);
    synth_metronome_init();
    synth_midi_init(_sampleRate);
    synth_circularbuffer_init();
    uint slice = synth_pwm_init(_sampleRate);
    synth_dma_init(slice);

#ifdef USE_MIDI
    board_init();
    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);
#endif
}
void core1_worker() {
    while (true) {
#ifndef USE_MIDI
        synth_test_play(_context);
#else
        // tinyusb device task
        tud_task();
        synth_led_blink_task();
        synth_midi_task(_context);
#endif
    }
}

int main() {
    set_sys_clock_khz(240000, true);
    stdio_init_all();
    init_all();

    printf("\n----------------------\nSynth starting.\n");
    printf("sample rate: %d\n", _sampleRate);
    printf("bit depth: %d\n", 16 - _bitDepth);

    // multicore_launch_core1(core1_worker);
    core1_worker();
    while (true) tight_loop_contents();
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
