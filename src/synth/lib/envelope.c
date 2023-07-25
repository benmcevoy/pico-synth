#include "../include/envelope.h"

static float _sampleRate = 0.f;

static  float elapsed(float time, float duration) {
    return (duration - time) / duration;
}

static bool has_elapsed(float time, float duration) {
    return elapsed(time, duration) >= 1.f;
}

static  float linear_easing(float time, float duration, float start, float end) {
    return has_elapsed(time, duration)
               ? end
               : start + elapsed(time, duration) * (end - start);
}

void synth_envelope_note_on(Voice_t* voice) { voice->triggerAttack = true; }

void synth_envelope_note_off(Voice_t* voice) { voice->triggerAttack = false; }

float synth_envelope_process(Voice_t* voice) {
    if (voice->triggerAttack && (voice->envelopeState == OFF || voice->envelopeState == RELEASE)) {
        voice->envelopeDuration = voice->attack * _sampleRate;
        voice->envelopeCounter = voice->envelopeDuration;
        voice->envelopeState = ATTACK;
    }

    if (!voice->triggerAttack &&
        (voice->envelopeState == ATTACK || voice->envelopeState == DECAY || voice->envelopeState == SUSTAIN)) {
        voice->envelopeDuration = voice->release * _sampleRate;
        voice->envelopeCounter = voice->envelopeDuration;
        voice->envelopeState = RELEASE;
    }

    switch (voice->envelopeState) {
        case OFF:
            return 0.f;

        case ATTACK:
            if (voice->envelopeCounter == 0) {
                voice->envelopeDuration = voice->decay * _sampleRate;
                voice->envelopeCounter = voice->envelopeDuration;
                voice->envelopeState = DECAY;
                return 1.f;
            }

            return linear_easing(voice->envelopeCounter--, voice->envelopeDuration, 0.f, 1.f);

        case DECAY:
            if (voice->envelopeCounter == 0) {
                voice->envelopeState = SUSTAIN;
                return voice->sustain;
            }

            return linear_easing(voice->envelopeCounter--, voice->envelopeDuration, 1.f, voice->sustain);

        case SUSTAIN:
            return voice->sustain;

        case RELEASE:
            if (voice->envelopeCounter == 0) {
                voice->envelopeState = OFF;
                return 0.f;
            }

            return linear_easing(voice->envelopeCounter--, voice->envelopeDuration, voice->sustain, 0.f);

        default:
            return 0.f;
    }
}

void synth_envelope_init(float sampleRate) { _sampleRate = sampleRate; }