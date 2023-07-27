#ifndef SYNTH_FILTER_
#define SYNTH_FILTER_

// TODO: DRY wrt waveform.h & why can't I use math.h for this?
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

void synth_filter_init(float sampleRate, float cutOffFrequency, float resonance);

void synth_filter_calculate_coefficients(float cutOffFrequency, float resonance);

float synth_filter_low_pass(float sample);

#endif