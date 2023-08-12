#include "../include/envelope.h"

#include <stdio.h>

static EnvelopeState_t _envelopeState = OFF;
static fix16 _envelopeRemaining = 0;
static fix16 _envelopeDuration = 0;
static fix16 _envelopeStart = 0;

static fix16 elapsed(fix16 remain, fix16 duration) {
    return divfix16((duration - remain), duration);
}

static bool has_elapsed(fix16 remain, fix16 duration) {
    return elapsed(remain, duration) >= FIX16_ONE;
}

static fix16 linear_easing(fix16 remain, fix16 duration, fix16 start,
                           fix16 end) {
    return has_elapsed(remain, duration)
               ? end
               : start + multfix16(elapsed(remain, duration), (end - start));
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
        _envelopeDuration = synth_envelope_to_duration(context->attack);
        _envelopeRemaining = _envelopeDuration;
        _envelopeState = ATTACK;
        _envelopeStart = context->envelope;
    }

    if (!context->triggerAttack &&
        (_envelopeState == ATTACK || _envelopeState == DECAY ||
         _envelopeState == SUSTAIN)) {
        _envelopeDuration = synth_envelope_to_duration(context->release);
        _envelopeRemaining = _envelopeDuration;
        _envelopeState = RELEASE;
        _envelopeStart = context->envelope;
    }

    switch (_envelopeState) {
        case OFF:
            return 0;

        case ATTACK:
            if (_envelopeRemaining <= EPSILON) {
                _envelopeDuration = synth_envelope_to_duration(context->decay);
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
                return context->sustain;
            }

            _envelopeRemaining -= FIX16_ONE;

            return linear_easing(_envelopeRemaining, _envelopeDuration,
                                 FIX16_ONE, context->sustain);

        case SUSTAIN:
            return context->sustain;

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

fix16 synth_envelope_gate(Gate_t* gate) {
    switch (gate->state) {
        case ATTACK:
            if (gate->remaining <= EPSILON) {
                gate->duration = gate->onDuration >> 1;
                gate->remaining = gate->duration;
                gate->state = SUSTAIN;
                return FIX16_ONE;
            }

            gate->remaining -= FIX16_ONE;

            return linear_easing(gate->remaining, gate->duration, 0, FIX16_ONE);

        case SUSTAIN:
            if (gate->remaining <= EPSILON) {
                gate->duration = gate->onDuration;
                gate->remaining = gate->duration;
                gate->state = RELEASE;
                return FIX16_ONE;
            }

            gate->remaining -= FIX16_ONE;

            return FIX16_ONE;

        case RELEASE:
            if (gate->remaining <= EPSILON) {
                gate->duration = gate->offDuration;
                gate->remaining = gate->duration;
                gate->state = OFF;
                return FIX16_ONE;
            }

            gate->remaining -= FIX16_ONE;

            return linear_easing(gate->remaining, gate->duration, FIX16_ONE, 0);

        case OFF:
            if (gate->remaining <= EPSILON) {
                gate->duration = gate->onDuration >> 1;
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
