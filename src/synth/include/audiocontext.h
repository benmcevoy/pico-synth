#ifndef SYNTH_AUDIOCONTEXT_
#define SYNTH_AUDIOCONTEXT_

#include <stdbool.h>
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

// 32767 is a fine sample rate anyway.
#define SAMPLE_RATE 32767
#define FIX16_SAMPLE_RATE 2147418112
#define BUFFER_LENGTH 128
// last time I checked I can handle 32 voices, which is pretty good
// could maybe do a polyphonic
// keep removing floats and refactor the fixed point integert to q2.14
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
  /// @brief fractional value between 0..1
  fix16 detune;
  fix16 frequency;
  waveform_t waveform;
  fix16 wavetable_stride;
  fix16 wavetable_read_pointer;
  fix16 wavetable_phase;
  fix16 gain;
} voice_t;

typedef struct {
  uint32_t duration_in_samples;
  bool isBeat;
} tempo_t;

typedef struct {
  uint16_t sample_rate;
  uint16_t* audio_out;
  fix16* raw;
  size_t samples_elapsed;
  fix16 gain;

  bool metronome_enabled;
  tempo_t tempo;

  bool delay_enabled;
  uint16_t delay;
  fix16 delay_gain;

  bool filter_enabled;
  fix16 cutoff;
  fix16 resonance;

  voice_t voices[VOICES_LENGTH];

  envelope_t envelope;
} audio_context_t;

static inline void synth_audiocontext_set_wavetable_stride(voice_t* voice) {
  voice->wavetable_stride =
      divfix16(multfix16(voice->frequency, voice->detune), FIX16_SAMPLE_RATE);
}

static inline fix16 synth_audiocontext_to_duration(float value) {
  return multfix16(float2fix16(value), FIX16_SAMPLE_RATE);
}

#endif
