#include "../include/filter.h"

#include <math.h>
#include <string.h>

#include "../include/audiocontext.h"

// https://github.com/ddiakopoulos/MoogLadders/blob/master/src/MusicDSPModel.h
// This file is unlicensed and uncopyright as found at:
// http://www.musicdsp.org/showone.php?id=24
// Considering how widely this same code has been used in ~100 projects on
// GitHub with various licenses, it might be reasonable to suggest that the
// license is CC-BY-SA

float cutoff;
float resonance;
float sample_rate;
static float stage[4];
static float delay[4];
static float p;
static float k;
static float t1;
static float t2;

void synth_filter_set_resonance(float r) {
  resonance = r * (t2 + 6.0 * t1) / (t2 - 6.0 * t1);
}

void synth_filter_set_cutoff(float c) {
  cutoff = 2.0 * c / sample_rate;

  p = cutoff * (1.8 - 0.8 * cutoff);
  k = 2.0 * sinf(cutoff * M_PI_2) - 1.0;
  t1 = (1.0 - p) * 1.386249;
  t2 = 12.0 + t1 * t1;

  synth_filter_set_resonance(resonance);
}

void synth_filter_init(audio_context_t* context) {
  sample_rate = context->sample_rate;

  memset(stage, 0, sizeof(stage));
  memset(delay, 0, sizeof(delay));

  synth_filter_set_cutoff(1000.0f);
  synth_filter_set_resonance(0.10f);
}

void synth_filter_process(audio_context_t* context) {

  for (size_t s = 0; s < BUFFER_LENGTH; ++s) {
    // TODO: audio_out is a scaled uint16_t 
    // needs to be fix16 (which means rewriting this filter to operate on that)
    // or float (much easier but slower)
    float x = context->audio_out[s] - resonance * stage[3];

    // Four cascaded one-pole filters (bilinear transform)
    stage[0] = x * p + delay[0] * p - k * stage[0];
    stage[1] = stage[0] * p + delay[1] * p - k * stage[1];
    stage[2] = stage[1] * p + delay[2] * p - k * stage[2];
    stage[3] = stage[2] * p + delay[3] * p - k * stage[3];

    // Clipping band-limited sigmoid
    stage[3] -= (stage[3] * stage[3] * stage[3]) / 6.0;

    delay[0] = x;
    delay[1] = stage[0];
    delay[2] = stage[1];
    delay[3] = stage[2];

    context->audio_out[s] = stage[3];
  }
}
