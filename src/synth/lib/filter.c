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

static fix16 cutoff;
static fix16 resonance;
static fix16 sample_rate;
static fix16 stage[4];
static fix16 delay[4];
static fix16 p;
static fix16 k;
static fix16 t1;
static fix16 t2;

static void set_resonance(fix16 r) {
  resonance = multfix16(r, divfix16((t2 + multfix16(FIX16_6, t1)),
                                    (t2 - multfix16(FIX16_6, t1))));
}

static void set_cutoff(fix16 c) {
  cutoff = divfix16(multfix16(FIX16_2, c), sample_rate);

  p = multfix16(cutoff, (FIX16_1_POINT_8 - multfix16(FIX16_POINT_8, cutoff)));

  k = multfix16(FIX16_2, (sinfix16((multfix16(cutoff, FIX16_PI_2))))) -
      FIX16_ONE;

  t1 = multfix16((FIX16_ONE - p), FIX16_1_POINT_386249);
  t2 = FIX16_12 + multfix16(t1, t1);

  set_resonance(resonance);
}

void synth_filter_init(audio_context_t* context) {
  sample_rate = FIX16_SAMPLE_RATE;

  memset(stage, 0, sizeof(stage));
  memset(delay, 0, sizeof(delay));

  set_resonance(context->filter.resonance);
  set_cutoff(context->filter.cutoff);
}

void synth_filter_process(audio_context_t* context) {
  if (!context->filter.enabled) return;

  // follow voice envelope or filter envelope
  fix16 envelope = context->filter.follow_voice_envelope ? context->envelope.envelope
                                          : context->filter.envelope.envelope;

  set_resonance(context->filter.resonance);

  set_cutoff(multfix16(multfix16(context->filter.cutoff, envelope),
                       context->filter.envelope_depth));

  for (size_t s = 0; s < BUFFER_LENGTH; ++s) {
    fix16 x = context->raw[s] - multfix16(resonance, stage[3]);

    // Four cascaded one-pole filters (bilinear transform)
    stage[0] =
        multfix16(x, p) + multfix16(delay[0], p) - multfix16(k, stage[0]);
    stage[1] = multfix16(stage[0], p) + multfix16(delay[1], p) -
               multfix16(k, stage[1]);
    stage[2] = multfix16(stage[1], p) + multfix16(delay[2], p) -
               multfix16(k, stage[2]);
    stage[3] = multfix16(stage[2], p) + multfix16(delay[3], p) -
               multfix16(k, stage[3]);

    // Clipping band-limited sigmoid
    stage[3] -=
        divfix16(multfix16(multfix16(stage[3], stage[3]), stage[3]), FIX16_6);

    delay[0] = x;
    delay[1] = stage[0];
    delay[2] = stage[1];
    delay[3] = stage[2];

    context->raw[s] = stage[3];
  }
}
