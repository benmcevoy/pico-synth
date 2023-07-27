#include "../include/envelope.h"

static float _sampleRate = 0.f;
static EnvelopeState_t envelopeState = OFF;
static uint32_t envelopeCounter = 0;
static uint32_t envelopeDuration = 0;
static float envelopeStart = 0.f;

static float elapsed(float time, float duration) {
    return (duration - time) / duration;
}

static bool has_elapsed(float time, float duration) {
    return elapsed(time, duration) >= 1.f;
}

static float linear_easing(float time, float duration, float start, float end) {
    return has_elapsed(time, duration)
               ? end
               : start + elapsed(time, duration) * (end - start);
}

void synth_envelope_note_on(AudioContext_t* context) {
    context->triggerAttack = true;
}

void synth_envelope_note_off(AudioContext_t* context) {
    context->triggerAttack = false;
}

float synth_envelope_process(AudioContext_t* context) {
    if (context->triggerAttack &&
        (envelopeState == OFF || envelopeState == RELEASE)) {
        envelopeDuration = context->attack * _sampleRate;
        envelopeCounter = envelopeDuration;
        envelopeState = ATTACK;
        envelopeStart = context->envelope;
    }

    if (!context->triggerAttack &&
        (envelopeState == ATTACK || envelopeState == DECAY ||
         envelopeState == SUSTAIN)) {
        envelopeDuration = context->release * _sampleRate;
        envelopeCounter = envelopeDuration;
        envelopeState = RELEASE;
        envelopeStart = context->envelope;
    }

    switch (envelopeState) {
        case OFF:
            return 0.f;

        case ATTACK:
            if (envelopeCounter == 0) {
                envelopeDuration = context->decay * _sampleRate;
                envelopeCounter = envelopeDuration;
                envelopeState = DECAY;
                return 1.f;
            }

            return linear_easing(envelopeCounter--, envelopeDuration,
                                 envelopeStart, 1.f);

        case DECAY:
            if (envelopeCounter == 0) {
                envelopeState = SUSTAIN;
                return context->sustain;
            }

            return linear_easing(envelopeCounter--, envelopeDuration, 1.f,
                                 context->sustain);

        case SUSTAIN:
            return context->sustain;

        case RELEASE:
            if (envelopeCounter == 0) {
                envelopeState = OFF;
                return 0.f;
            }

            return linear_easing(envelopeCounter--, envelopeDuration,
                                 envelopeStart, 0.f);

        default:
            return 0.f;
    }
}

void synth_envelope_init(float sampleRate) { _sampleRate = sampleRate; }