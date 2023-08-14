#include "../include/envelope.h"

#include <stdio.h>

static EnvelopeState_t _envelopeState = OFF;
static fix16 _envelopeRemaining = 0;
static fix16 _envelopeDuration = 0;
static fix16 _envelopeStart = 0;

static fix16 linear_easing(fix16 remain, fix16 duration, fix16 start,
                           fix16 end) {
    fix16 elapsed = divfix16((duration - remain), duration);
    return lerp(elapsed, start, end);
}

fix16 synth_envelope_to_duration(fix16 value) {
    return multfix16(value, FIX16_SAMPLE_RATE);
}

void synth_envelope_note_on(AudioContext_t* context) {
    context->triggerAttack = true;
}

void synth_envelope_note_off(AudioContext_t* context) {
    context->triggerAttack = false;
}

fix16 synth_envelope_process(AudioContext_t* context) {
    if (context->triggerAttack &&
        (_envelopeState == OFF || _envelopeState == RELEASE)) {
        _envelopeDuration = context->envelope.attack;
        _envelopeRemaining = _envelopeDuration;
        _envelopeState = ATTACK;
        _envelopeStart = context->envelope.envelope;
    }

    if (!context->triggerAttack &&
        (_envelopeState == ATTACK || _envelopeState == DECAY ||
         _envelopeState == SUSTAIN)) {
        _envelopeDuration = context->envelope.release;
        _envelopeRemaining = _envelopeDuration;
        _envelopeState = RELEASE;
        _envelopeStart = context->envelope.envelope;
    }

    switch (_envelopeState) {
        case OFF:
            return 0;

        case ATTACK:
            if (_envelopeRemaining <= EPSILON) {
                _envelopeDuration = context->envelope.decay;
                _envelopeRemaining = _envelopeDuration;
                _envelopeState = DECAY;
                return FIX16_ONE;
            }

            _envelopeRemaining -= FIX16_ONE;

            return linear_easing(_envelopeRemaining, _envelopeDuration,
                                 _envelopeStart, FIX16_ONE);

        case DECAY:
            if (_envelopeRemaining <= EPSILON) {
                _envelopeState = SUSTAIN;
                return context->envelope.sustain;
            }

            _envelopeRemaining -= FIX16_ONE;

            return linear_easing(_envelopeRemaining, _envelopeDuration,
                                 FIX16_ONE, context->envelope.sustain);

        case SUSTAIN:
            return context->envelope.sustain;

        case RELEASE:
            if (_envelopeRemaining <= EPSILON) {
                _envelopeState = OFF;
                return 0;
            }

            _envelopeRemaining -= FIX16_ONE;

            return linear_easing(_envelopeRemaining, _envelopeDuration,
                                 _envelopeStart, 0);

        default:
            return 0;
    }
}

fix16 synth_envelope_gate(Envelope_t* gate) {
    switch (gate->state) {
        case ATTACK:
            if (gate->remaining <= EPSILON) {
                gate->duration = gate->decay;
                gate->remaining = gate->duration;
                gate->state = SUSTAIN;
                return FIX16_ONE;
            }

            gate->remaining -= FIX16_ONE;

            return linear_easing(gate->remaining, gate->duration, 0, FIX16_ONE);

        case SUSTAIN:
            if (gate->remaining <= EPSILON) {
                gate->duration = gate->release;
                gate->remaining = gate->duration;
                gate->state = RELEASE;
                return FIX16_ONE;
            }

            gate->remaining -= FIX16_ONE;

            return FIX16_ONE;

        case RELEASE:
            if (gate->remaining <= EPSILON) {
                gate->duration = gate->sustain;
                gate->remaining = gate->duration;
                gate->state = OFF;
                return FIX16_ONE;
            }

            gate->remaining -= FIX16_ONE;

            return linear_easing(gate->remaining, gate->duration, FIX16_ONE, 0);

        case OFF:
            if (gate->remaining <= EPSILON) {
                gate->duration = gate->attack;
                gate->remaining = gate->duration;
                gate->state = ATTACK;
                return 0;
            }

            gate->remaining -= FIX16_ONE;

            return 0;

        default:
            return 0;
    }
}
