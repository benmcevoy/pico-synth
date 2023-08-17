#include <stdio.h>
#include "../include/envelope.h"

static fix16 _envelopeStart = 0;

static fix16 linear_easing(fix16 remain, fix16 duration, fix16 start,
                           fix16 end) {
    fix16 elapsed = divfix16((duration - remain), duration);
    return lerp(elapsed, start, end);
}

fix16 synth_envelope_to_duration(fix16 value) {
    return multfix16(value, FIX16_SAMPLE_RATE);
}

void synth_envelope_note_on(Envelope_t* envelope) {
    envelope->triggerAttack = true;
}

void synth_envelope_note_off(Envelope_t* envelope) {
    envelope->triggerAttack = false;
}

fix16 synth_envelope_process(Envelope_t* envelope) {
    if (envelope->triggerAttack &&
        (envelope->state == OFF || envelope->state == RELEASE)) {
        envelope->duration = envelope->attack;
        envelope->remaining = envelope->duration;
        envelope->state = ATTACK;
        _envelopeStart = envelope->envelope;
    }

    if (!envelope->triggerAttack &&
        (envelope->state == ATTACK || envelope->state == DECAY ||
         envelope->state == SUSTAIN)) {
        envelope->duration = envelope->release;
        envelope->remaining = envelope->duration;
        envelope->state = RELEASE;
        _envelopeStart = envelope->envelope;
    }

    switch (envelope->state) {
        case OFF:
            return 0;

        case ATTACK:
            if (envelope->remaining <= EPSILON) {
                envelope->duration = envelope->decay;
                envelope->remaining = envelope->duration;
                envelope->state = DECAY;
                return FIX16_ONE;
            }

            envelope->remaining -= FIX16_ONE;

            return linear_easing(envelope->remaining, envelope->duration,
                                 _envelopeStart, FIX16_ONE);

        case DECAY:
            if (envelope->remaining <= EPSILON) {
                envelope->state = SUSTAIN;
                return envelope->sustain;
            }

            envelope->remaining -= FIX16_ONE;

            return linear_easing(envelope->remaining, envelope->duration,
                                 FIX16_ONE, envelope->sustain);

        case SUSTAIN:
            return envelope->sustain;

        case RELEASE:
            if (envelope->remaining <= EPSILON) {
                envelope->state = OFF;
                return 0;
            }

            envelope->remaining -= FIX16_ONE;

            return linear_easing(envelope->remaining, envelope->duration,
                                 _envelopeStart, 0);

        default:
            return 0;
    }
}