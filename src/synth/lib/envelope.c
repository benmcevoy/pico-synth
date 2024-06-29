#include "../include/envelope.h"

#include <stdio.h>

static fix16 envelope_start = 0;

static fix16 linear_easing(fix16 elapsed, fix16 duration, fix16 start,
                           fix16 end) {
    fix16 fraction = divfix16(elapsed, duration);
    return lerp(fraction, start, end);
}

void synth_envelope_note_on(envelope_t* envelope) {
    envelope->trigger_attack = true;
}

void synth_envelope_note_off(envelope_t* envelope) {
    envelope->trigger_attack = false;
}

fix16 synth_envelope_process(envelope_t* envelope) {
    if (envelope->trigger_attack &&
        (envelope->state == OFF || envelope->state == RELEASE)) {
        envelope->duration = envelope->attack;
        envelope->elapsed = 0;
        envelope->state = ATTACK;
        envelope_start = envelope->envelope;
    }

    if (!envelope->trigger_attack &&
        (envelope->state == ATTACK || envelope->state == DECAY ||
         envelope->state == SUSTAIN)) {
        envelope->duration = envelope->release;
        envelope->elapsed = 0;
        envelope->state = RELEASE;
        envelope_start = envelope->envelope;
    }

    switch (envelope->state) {
        case OFF:
            return 0;

        case ATTACK:
            if (envelope->elapsed >= envelope->duration) {
                envelope->duration = envelope->decay;
                envelope->elapsed = 0;
                envelope->state = DECAY;
                return FIX16_ONE;
            }

            envelope->elapsed += FIX16_ONE;

            return linear_easing(envelope->elapsed, envelope->duration,
                                 envelope_start, FIX16_ONE);

        case DECAY:
            if (envelope->elapsed >= envelope->duration) {
                envelope->state = SUSTAIN;
                return envelope->sustain;
            }

            envelope->elapsed += FIX16_ONE;

            return linear_easing(envelope->elapsed, envelope->duration,
                                 FIX16_ONE, envelope->sustain);

        case SUSTAIN:
            return envelope->sustain;

        case RELEASE:
            if (envelope->elapsed >= envelope->duration) {
                envelope->state = OFF;
                return 0;
            }

            envelope->elapsed += FIX16_ONE;

            return linear_easing(envelope->elapsed, envelope->duration,
                                 envelope_start, 0);

        default:
            return 0;
    }
}