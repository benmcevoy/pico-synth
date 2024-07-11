#ifndef SYNTH_AUDIOCONTEXT_
#define SYNTH_AUDIOCONTEXT_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "fixedpoint.h"

// TODO: why can't I use math.h for this?
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#define M_PI_2 1.57079632679489661923
#endif

// if sample rate is greater than 32767 it will not fit in a signed fix16, so
// shift the decimal point to resolve this
// #define SAMPLE_RATE_SCALE_FACTOR 1
// FIX16_SCALED_SAMPLE_RATE is (SAMPLE_RATE >> SAMPLE_RATE_SCALE_FACTOR) as
// fix16 e.g << 16

// 32767 is 0111 1111 1111 1111 0000 0000 0000 0000  which is the maximum
// integer value we can fit in fix16 32767 is a fine sample rate anyway.
#define SAMPLE_RATE 32767
#define FIX16_SAMPLE_RATE 2147418112
#define BUFFER_LENGTH 32
// last time I checked I can handle 32 oscillators, which is pretty good
// could maybe do a polyphonic
// keep removing floats and refactor the fixed point integer to q2.14
#define VOICES_LENGTH 2

typedef enum { SINE = 0, SQUARE, SAW, TRIANGLE, NOISE } waveform_t;
typedef enum { OFF = 0, ATTACK, DECAY, SUSTAIN, RELEASE } envelope_state_t;

typedef struct {
  bool trigger_attack;
  envelope_state_t state;
  fix16 elapsed;
  fix16 duration;
  fix16 envelope;

  fix16 attack;
  fix16 decay;
  fix16 sustain;
  fix16 release;
} envelope_t;

typedef struct {
  fix16 width;
  fix16 detune;
  fix16 frequency;
  waveform_t waveform;

  fix16 wavetable_stride;
  fix16 wavetable_read_pointer;
  fix16 wavetable_phase;
} voice_t;

typedef struct {
  uint32_t duration_in_samples;
  bool is_beat;
  bool enabled;
} metronome_t;

typedef struct {
  fix16 cutoff;
  fix16 resonance;
  envelope_t envelope;
  fix16 envelope_depth;
  bool enabled;
  bool follow_voice_envelope;
} filter_t;

typedef struct {
  bool enabled;
  /// @brief number of samples to delay by
  uint16_t delay_in_samples;
  fix16 feedback;
  fix16 dry_wet_mix;
} delay_t;

typedef struct {
  size_t samples_elapsed;
  uint16_t sample_rate;

  uint16_t* pwm_out;
  fix16* raw;

  fix16 gain;
  fix16 mod_wheel;
  fix16 velocity;
  fix16 pitch_bend;

  metronome_t metronome;
  filter_t filter;
  delay_t delay;
  envelope_t envelope;

  voice_t voices[VOICES_LENGTH];

} audio_context_t;

static uint8_t synth_audiocontext_debug_duration_to_bpm(uint16_t no_of_samples){
  return 60.f/((float)no_of_samples/SAMPLE_RATE);
}

static float synth_audiocontext_debug_duration_to_millis(fix16 no_of_samples){
  return 1000.f * (fix2float16(no_of_samples)/SAMPLE_RATE);
}

static char* synth_audiocontext_debug_waveform_to_string(waveform_t w) {
  switch (w) {
    case SINE:
      return "SINE";
    case TRIANGLE:
      return "TRIANGLE";
    case SQUARE:
      return "SQUARE";
    case SAW:
      return "SAW";
    case NOISE:
      return "NOISE";
  }
  return "UNKNOWN";
}

static void synth_audiocontext_debug(audio_context_t* context) {
  printf("audio_context: {\n");

  printf("\tgain: %f,\n", fix2float16(context->gain));
  printf("\tsample_rate: %u,\n", (context->sample_rate));
  printf("\tmod_wheel: %f,\n", fix2float16(context->mod_wheel));
  printf("\tvelocity: %f,\n", fix2float16(context->velocity));
  printf("\tpitch_bend: %f,\n", fix2float16(context->pitch_bend));

  printf("\tdelay: {\n");
  printf("\t\tenabled: %d,\n", (context->delay.enabled));
  printf("\t\tdelay_in_samples: %u,\n", (context->delay.delay_in_samples));
  printf("\t\tfeedback: %f,\n", fix2float16(context->delay.feedback));
  printf("\t\tdry_wet_mix: %f,\n", fix2float16(context->delay.dry_wet_mix));
  printf("\t}\n");

  printf("\tenvelope: {\n");
  printf("\t\tattack: %fms,\n", synth_audiocontext_debug_duration_to_millis(context->envelope.attack));
  printf("\t\tdecay: %fms,\n", synth_audiocontext_debug_duration_to_millis(context->envelope.decay));
  printf("\t\tsustain: %f,\n", fix2float16(context->envelope.sustain));
  printf("\t\trelease: %fms,\n", synth_audiocontext_debug_duration_to_millis(context->envelope.release));
  printf("\t}\n");

  printf("\tvoices[0]: {\n");
  printf("\t\tdetune: %f,\n", fix2float16(context->voices[0].detune));
  printf("\t\twidth: %f,\n", fix2float16(context->voices[0].width));
  printf("\t\tfrequency: %f,\n", fix2float16(context->voices[0].frequency));
  printf("\t\tvwaveform: %s,\n", synth_audiocontext_debug_waveform_to_string(context->voices[0].waveform));
  printf("\t}\n");

  printf("\tvoices[1]: {\n");
  printf("\t\tdetune: %f,\n", fix2float16(context->voices[1].detune));
  printf("\t\twidth: %f,\n", fix2float16(context->voices[1].width));
  printf("\t\tfrequency: %f,\n", fix2float16(context->voices[1].frequency));
  printf("\t\twaveform: %s,\n", synth_audiocontext_debug_waveform_to_string(context->voices[1].waveform));
  printf("\t}\n");

  printf("\tfilter: {\n");
  printf("\t\tenabled: %d,\n", (context->filter.enabled));
  printf("\t\tcutoff: %f,\n", fix2float16(context->filter.cutoff));
  printf("\t\tresonance: %f,\n", fix2float16(context->filter.resonance));
  printf("\t\tfollow_voice_envelope: %d,\n",
         (context->filter.follow_voice_envelope));
  printf("\t}\n");

  printf("\tmetronome: {\n");
  printf("\t\tenabled: %d,\n", (context->metronome.enabled));
  printf("\t\tBPM: %d,\n", synth_audiocontext_debug_duration_to_bpm(context->metronome.duration_in_samples));
  printf("\t}\n");

  printf("}\n");
}

#endif
