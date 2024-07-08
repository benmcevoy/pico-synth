#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "hardware/vreg.h"
#include "include/audiocontext.h"
#include "include/controller.h"
#include "include/delay.h"
#include "include/envelope.h"
#include "include/filter.h"
#include "include/fixedpoint.h"
#include "include/ledblink.h"
#include "include/metronome.h"
#include "include/midi.h"
#include "include/pitchtable.h"
#include "include/waveform.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "test.h"
#include "tusb.h"
#include "tusb_config.h"
#include "usb_midi_host.h"

#define PWM_PIN 26

// uncomment to use midi or comment out for the test code
#define USE_MIDI

static uint16_t buffer0[BUFFER_LENGTH] = {0};
static uint16_t buffer1[BUFFER_LENGTH] = {0};
static fix16 raw_buffer[BUFFER_LENGTH] = {0};
static bool swap = false;
static int pwm_dma_channel;
static audio_context_t* context;
static int bit_depth = 12;

static uint8_t midi_dev_addr = 0;

static fix16 previous_dither = 0;
static fix16 dither() {
  // mask off the bottom 8 bits
  uint8_t p = synth_waveform_noise() & 0x00000ff;

  // TODO: not sure about this.  guide is to use 2 LSB of dither
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
  fix16 result = dither - previous_dither;

  previous_dither = dither;

  return result;
}

static void fill_write_buffer() {
  for (int i = 0; i < BUFFER_LENGTH; i++) {
    fix16 amplitude = 0;

    for (int v = 0; v < VOICES_LENGTH; v++) {
      voice_t* voice = &context->voices[v];

      fix16 sample = synth_waveform_sample(voice, context->pitch_bend);

      amplitude += multfix16(context->velocity, sample);
    }

    // ***** ENVELOPE ******
    context->filter.envelope.envelope =
        synth_envelope_process(&context->filter.envelope);
    fix16 envelope = synth_envelope_process(&context->envelope);
    amplitude = multfix16(amplitude, envelope);
    // *********************

    // apply master gain
    // i have preserved the mix as just summation
    // gain can scale it down to avoid clipping
    amplitude = multfix16(context->gain, amplitude);

    // ***** DITHER ******
    // TODO: not sure I need to introduce noise when I use a breadboard :)
    // if/when I can get control of the sample rate/bit depth again
    // i'll drop it down to 8 or 6 bits (thanks Tony)
    // and hopefully the effect will be more obvious
    // currently I cannot tell the difference
    // amplitude += dither(amplitude);
    // *********************

    // final volume
    context->envelope.envelope = envelope;
    context->raw[i] = amplitude;
    context->samples_elapsed++;
  }

  // ***** FILTER ******
  synth_filter_process(context);
  // *********************

  synth_delay_set_delay(&context->delay);

  // ***** CONVERT TO PWM ******
  // final conversion to PWM
  // amplitude value is fix16 and should be roughly between -1..1
  // need to scale to 0..2^16
  for (size_t i = 0; i < BUFFER_LENGTH; i++) {
    fix16 amplitude = context->raw[i];

    // **** DELAY/ECHO *****
    amplitude += synth_delay_process(&context->delay, amplitude);
    // *********************

    // **** METRONOME *****
    // metronome added after effects
    amplitude += synth_metronome_process(&context->metronome);
    // *********************

    // ***** COMPRESS ******
    // compression, if the amplitude is with -1..1 then compress should not
    // effect too much
    amplitude = tanhfix16(amplitude);
    // *********************

    // the mix value is between -1..1
    // add 1 to move it 0..2
    amplitude = amplitude + FIX16_ONE;

    // scale to an "about 12" bit signal in a 16-bit container
    // we should now have a number between 0..2 (or really 0..1.99999999999
    // need to scale to the bit depth as determined by the wrap calulated on
    // init
    uint16_t out = (uint16_t)(amplitude >> bit_depth);

    context->pwm_out[i] = out;
  }
  // *********************
}

static void __isr __time_critical_func(dma_irq_handler)() {
  // cycle buffers
  context->pwm_out = swap ? buffer0 : buffer1;
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

  context->delay.enabled = true;
  context->metronome.enabled = false;
  context->filter.enabled = true;

  context->raw = raw_buffer;
  context->pwm_out = buffer1;
  context->sample_rate = SAMPLE_RATE;
  context->samples_elapsed = 0;
  context->gain = FIX16_POINT_7;
  context->mod_wheel = 0;
  context->pitch_bend = 0;
  context->velocity = FIX16_POINT_5;

  context->delay.delay_in_samples = 0;
  context->delay.feedback = 0;
  context->delay.dry_wet_mix = FIX16_POINT_5;

  context->filter.follow_voice_envelope = true;
  context->filter.resonance = 0;
  context->filter.cutoff = 0;
  context->filter.envelope.state = OFF;
  context->filter.envelope.envelope = 0;
  context->filter.envelope.elapsed = 0;
  context->filter.envelope.duration = 0;
  context->filter.envelope.attack =
      synth_envelope_to_duration(float2fix16(0.01f));
  context->filter.envelope.decay =
      synth_envelope_to_duration(float2fix16(0.03f));
  context->filter.envelope.sustain = FIX16_POINT_7;
  context->filter.envelope.release =
      synth_envelope_to_duration(float2fix16(0.4f));
  context->filter.envelope_depth = FIX16_ONE;

  context->envelope.state = OFF;
  context->envelope.envelope = 0;
  context->envelope.elapsed = 0;
  context->envelope.duration = 0;
  context->envelope.trigger_attack = false;
  context->envelope.attack = synth_envelope_to_duration(float2fix16(0.01f));
  context->envelope.decay = synth_envelope_to_duration(float2fix16(0.03f));
  context->envelope.sustain = FIX16_POINT_7;
  context->envelope.release = synth_envelope_to_duration(float2fix16(0.03f));

  for (int v = 0; v < VOICES_LENGTH; v++) {
    context->voices[v].frequency = PITCH_C3;
    context->voices[v].waveform = SAW;
    context->voices[v].detune = FIX16_ONE;
    context->voices[v].wavetable_phase = 0;
    context->voices[v].width = 0;
    synth_waveform_set_wavetable_stride(&context->voices[v], 0);
  }
}

void init_all() {
  board_init();
  synth_audio_context_init();
  synth_metronome_init(&context->metronome, 120);
  synth_filter_init(context);
  synth_delay_init();
  uint slice = synth_pwm_init();
  synth_dma_init(slice);
  synth_controller_init();

#ifdef USE_MIDI
  synth_midi_init(context);

  // init usb host
  tuh_init(BOARD_TUD_RHPORT);

  // read to initialise to the state of the physical controls
  synth_controller_task(context);
#endif
}

void core1_worker() {
  while (true) {
    
#ifndef USE_MIDI
    // TODO: the player has a load of sleeps so the controller does not update
    // synth_controller_task(context);
    // also cannot blink led for the same reason
    synth_test_play(context);
#else
    synth_led_blink_task();
    // tinyusb host task
    tuh_task();
    synth_controller_task(context);
#endif
  }
}

int main() {
  // can go to 420MHz set vreg to 1.3
  vreg_set_voltage(VREG_VOLTAGE_1_15);
  set_sys_clock_khz(320000, true);
  stdio_init_all();
  init_all();

uint systemClockHz = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) / 1000;

  printf("\n----------------------\nSynth starting.\n");
  printf("system clock: %uMHz\n", systemClockHz);
  
  printf("sample rate: %dHz\n", SAMPLE_RATE);
  printf("bit depth: %d\n", 16 - bit_depth);

  synth_audiocontext_debug(context);

  // multicore_launch_core1(core1_worker);
  core1_worker();

  while (1) {
    tight_loop_contents();
  }
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use.
// tuh_hid_parse_report_descriptor() can be used to parse common/simple enough
// descriptor. Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE,
// it will be skipped therefore report_desc = NULL, desc_len = 0
void tuh_midi_mount_cb(uint8_t dev_addr, uint8_t in_ep, uint8_t out_ep,
                       uint8_t num_cables_rx, uint16_t num_cables_tx) {
  printf(
      "MIDI device address = %u, IN endpoint %u has %u cables, OUT endpoint %u "
      "has %u cables\r\n",
      dev_addr, in_ep & 0xf, num_cables_rx, out_ep & 0xf, num_cables_tx);

  if (midi_dev_addr == 0) {
    // then no MIDI device is currently connected
    midi_dev_addr = dev_addr;
  } else {
    printf(
        "A different USB MIDI Device is already connected.\r\nOnly one device "
        "at a time is supported in this program\r\nDevice is disabled\r\n");
  }
}

// Invoked when device with hid interface is un-mounted
void tuh_midi_umount_cb(uint8_t dev_addr, uint8_t instance) {
  if (dev_addr == midi_dev_addr) {
    midi_dev_addr = 0;
    printf("MIDI device address = %d, instance = %d is unmounted\r\n", dev_addr,
           instance);
  } else {
    printf("Unused MIDI device address = %d, instance = %d is unmounted\r\n",
           dev_addr, instance);
  }
}

void tuh_midi_rx_cb(uint8_t dev_addr, uint32_t num_packets) {
  if (midi_dev_addr == dev_addr) {
    if (num_packets != 0) {
      uint8_t cable_num;
      uint8_t buffer[48];
      while (1) {
        uint32_t bytes_read =
            tuh_midi_stream_read(dev_addr, &cable_num, buffer, sizeof(buffer));
        if (bytes_read == 0) return;
        synth_midi_task(context, buffer);
      }
    }
  }
}

void tuh_midi_tx_cb(uint8_t dev_addr) { (void)dev_addr; }