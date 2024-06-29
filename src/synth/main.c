#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// #include "hardware/vreg.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "include/audiocontext.h"
#include "include/circularbuffer.h"
#include "include/controller.h"
#include "include/envelope.h"
#include "include/filter.h"
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

#define PWM_PIN 26

// uncomment to use midi or comment out for the test code
#define USE_MIDI
// #define USE_METRONOME

static uint16_t buffer0[BUFFER_LENGTH] = {0};
static uint16_t buffer1[BUFFER_LENGTH] = {0};
static bool swap = false;
static int pwm_dma_channel;
static audio_context_t* context;
static int bit_depth = 12;

static uint8_t midi_dev_addr = 0;

fix16 _previousDither = 0;
static fix16 dither() {
  // mask off the bottom 8 bits
  uint8_t p = synth_waveform_noise() & 0x00000ff;

  // TODO: not sure abouth this.  guide is to use 2 LSB of dither
  // not sure if that is 2 LSB on this 32 bit number or on the 12 bit quantized
  // value later? I *think* this setting of _bitDepth + 3 sounds better? but I
  // could be deluding myself I looked at some frequency spectrums with this
  // on/off and various settings the noise floor is raised and skewed to higher
  // frequency might be more noticeable with I2S .  have to wait and see.
  uint8_t depth = bit_depth + 3;
  fix16 dither;

  // these numbers were derived by pissing around in a spreadsheet
  // apply triangular probability
  if (p < 4)
    dither = -57344 >> depth;
  else if (p < 11)
    dither = -49152 >> depth;
  else if (p < 21)
    dither = -40960 >> depth;
  else if (p < 36)
    dither = -32768 >> depth;
  else if (p < 53)
    dither = -24576 >> depth;
  else if (p < 75)
    dither = -16384 >> depth;
  else if (p < 100)
    dither = -8192 >> depth;
  // from here (>=100)
  else if (p < 128)
    dither = 0;
  else if (p < 156)
    dither = 0;
  // to the above accounts for about 22% (56/256)
  else if (p < 181)
    dither = 8192 >> depth;
  else if (p < 203)
    dither = 16384 >> depth;
  else if (p < 220)
    dither = 24576 >> depth;
  else if (p < 235)
    dither = 32768 >> depth;
  else if (p < 245)
    dither = 40960 >> depth;
  else if (p < 252)
    dither = 49152 >> depth;
  else if (p < 256)
    dither = 57344 >> depth;

  // noise shaping
  fix16 result = dither - _previousDither;

  _previousDither = dither;

  return result;
}

static void fill_write_buffer() {
  for (int i = 0; i < BUFFER_LENGTH; i++) {
    fix16 amplitude = 0;

    for (int v = 0; v < VOICES_LENGTH; v++) {
      voice_t* voice = &context->voices[v];

      fix16 sample = synth_waveform_sample(voice);

      amplitude += multfix16(voice->gain, sample);
    }

    synth_tempo_process(&context->tempo);

    // ***** ENVELOPE ******
    fix16 envelope = synth_envelope_process(&context->envelope);
    amplitude = multfix16(amplitude, envelope);
    // *********************

    // **** DELAY/ECHO *****
    // apply feedback
    amplitude = amplitude +
                multfix16(multfix16(synth_circularbuffer_read(), FIX16_POINT_7),
                          context->delay_gain);
    synth_circularbuffer_write(amplitude, context->delay);
    // *********************

#ifdef USE_METRONOME
    amplitude += synth_metronome_process(&context->tempo);
#endif

    // apply master gain
    // i have preserved the mix as just summation
    // gain can scale it down to avoid clipping
    amplitude = multfix16(context->gain, amplitude);

    // TODO: not sure I need to introduce noise when I use a breadboard :)
    // if/when I can get control of the sample rate/bit depth again
    // i'll drop it down to 8 or 6 bits (thanks Tony)
    // and hopefully the effect will be more obvious
    // currently I cannot tell the difference
    // amplitude += dither(amplitude);

    // compression, if the amplitude is with -1..1 then compress should not
    // effect too much
    // TODO: remove float - need tanh for filter as well
    // https://github.com/ARM-software/optimized-routines/blob/master/pl/math/tanhf_2u6.c
    amplitude = float2fix16(tanhf(fix2float16(amplitude)));

    // scale to an "about 12" bit signal in a 16-bit container
    // the mix value is between -1..1
    // add 1 to move it 0..2
    // we should now have a number between 0..2 (or really 0..1.99999999999
    // need to scale to the bit depth as determined by the wrap calulated on
    // init
    amplitude = amplitude + FIX16_ONE;
    uint16_t out = (uint16_t)(amplitude >> bit_depth);

    // final volume
    context->envelope.envelope = envelope;
    context->audio_out[i] = out;
    context->samples_elapsed++;
  }

  // ***** FILTER ******
  // TODO: operates on the buffer
  // synth_filter(context->audioOut, cutoff, resonance);
  // *********************
}

static void __isr __time_critical_func(dma_irq_handler)() {
  // cycle buffers
  context->audio_out = swap ? buffer0 : buffer1;
  swap = !swap;

  // ack irq
  dma_hw->ints0 = 1u << pwm_dma_channel;
  // restart DMA
  dma_channel_transfer_from_buffer_now(pwm_dma_channel,
                                       swap ? buffer0 : buffer1, BUFFER_LENGTH);

  fill_write_buffer();
}

static uint synth_pwm_init() {
  // derive clk_div and wrap
  // keep clk_div at 1
  uint systemClockHz = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) * 1000;

  uint16_t wrap = systemClockHz / SAMPLE_RATE;
  bit_depth = 16 - floorf(log2f(wrap + 1));

  // setup pwm
  gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
  gpio_set_function(PWM_PIN + 1, GPIO_FUNC_PWM);
  uint slice = pwm_gpio_to_slice_num(PWM_PIN);
  pwm_config pwmConfig = pwm_get_default_config();

  // set sample rate/clock to 125Mhz / wrap+1 / div =   ~22kHz
  // rp2040 datasheet gives the actual formula

  pwm_config_set_clkdiv(&pwmConfig, 1.f);
  pwm_config_set_wrap(&pwmConfig, wrap);
  // pwm_config_set_phase_correct(&pwmConfig, true);
  pwm_init(slice, &pwmConfig, true);

  return slice;
}

static void synth_dma_init(uint slice) {
  // setup dma
  pwm_dma_channel = dma_claim_unused_channel(true);

  dma_channel_config dmaConfig =
      dma_channel_get_default_config(pwm_dma_channel);
  // must be 16 bits
  channel_config_set_transfer_data_size(&dmaConfig, DMA_SIZE_16);
  channel_config_set_read_increment(&dmaConfig, true);
  channel_config_set_write_increment(&dmaConfig, false);
  channel_config_set_dreq(&dmaConfig, DREQ_PWM_WRAP0 + slice);

  dma_channel_configure(
      pwm_dma_channel, &dmaConfig,
      &pwm_hw->slice[slice].cc,  // Write to PWM counter compare
      &buffer0,                  // read from buffer
      BUFFER_LENGTH,             // number of transfers to perform
      true                       // start
  );

  dma_channel_set_irq0_enabled(pwm_dma_channel, true);
  irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
  irq_set_enabled(DMA_IRQ_0, true);
}

static void synth_audio_context_init() {
  context = malloc(sizeof(audio_context_t));

  context->sample_rate = SAMPLE_RATE;
  context->samples_elapsed = 0;
  context->gain = FIX16_ONE;
  context->delay = 0;
  context->delay_gain = 0;

  context->envelope.state = OFF;
  context->envelope.elapsed = 0;
  context->envelope.duration = 0;
  context->envelope.trigger_attack = false;
  context->envelope.attack = synth_audiocontext_to_duration(0.01f);
  context->envelope.decay = synth_audiocontext_to_duration(0.03f);
  context->envelope.sustain = FIX16_POINT_7;
  context->envelope.release = synth_audiocontext_to_duration(0.3f);

  for (int v = 0; v < VOICES_LENGTH; v++) {
    context->voices[v].gain = FIX16_POINT_5;
    context->voices[v].frequency = PITCH_C3;
    context->voices[v].waveform = SAW;
    context->voices[v].detune = 0;
    context->voices[v].width = FIX16_PI;
    context->voices[v].wavetable_phase = 0;
    synth_audiocontext_set_wavetable_stride(&context->voices[v]);
  }
}

void init_all() {
  synth_audio_context_init();
  synth_tempo_init(&context->tempo, 120);
  synth_metronome_init();
  synth_midi_init(&context);
  synth_circularbuffer_init();
  uint slice = synth_pwm_init();
  synth_dma_init(slice);
  synth_controller_init(&context);

#ifdef USE_MIDI
  board_init();

  // init usb device
  tud_init(BOARD_TUD_RHPORT);

  // read to initialise to the state of the physical controls
  synth_controller_task(context);
#endif
}

void core1_worker() {
  while (true) {
#ifndef USE_MIDI
    // TODO: the player has a load of sleeps so the controller does not update
    // synth_controller_task(context);
    synth_test_play(context);
#else
    // tinyusb device task
    tud_task();
    synth_led_blink_task();
    synth_controller_task(context);
    synth_midi_task(context);
#endif
  }
}

int main() {
  // vreg_set_voltage(VREG_VOLTAGE_1_15);
  set_sys_clock_khz(240000, true);
  stdio_init_all();
  init_all();

  printf("\n----------------------\nSynth starting.\n");
  printf("sample rate: %d\n", SAMPLE_RATE);
  printf("bit depth: %d\n", 16 - bit_depth);

  // multicore_launch_core1(core1_worker);
  // midi
  core1_worker();
  // controller
  while (1) {
    tight_loop_contents();
  }
}
