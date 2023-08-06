#include "../include/envelope.h"

#include <stdio.h>

#define SR_SCALE_FACTOR 2

static fix16 _sampleRate = 0;
static EnvelopeState_t _envelopeState = OFF;
static fix16 _remaining = 0;
static fix16 _duration = 0;
static fix16 _envelopeStart = 0;
static fix16 _zeroThreshold = 1000;

fix16 to_duration(fix16 value) {
    return multfix16(value, _sampleRate) << SR_SCALE_FACTOR;
}

static fix16 elapsed(fix16 remain, fix16 duration) {
    return divfix16((duration - remain), duration);
}

static bool has_elapsed(fix16 remain, fix16 duration) {
    return elapsed(remain, duration) >= FIX16_UNIT;
}

static fix16 linear_easing(fix16 remain, fix16 duration, fix16 start,
                           fix16 end) {
    return has_elapsed(remain, duration)
               ? end
               : start + multfix16(elapsed(remain, duration), (end - start));
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
        _duration = to_duration(context->attack);
        _remaining = _duration;
        _envelopeState = ATTACK;
        _envelopeStart = context->envelope;
    }

    if (!context->triggerAttack &&
        (_envelopeState == ATTACK || _envelopeState == DECAY ||
         _envelopeState == SUSTAIN)) {
        _duration = to_duration(context->release);
        _remaining = _duration;
        _envelopeState = RELEASE;
        _envelopeStart = context->envelope;
    }

    switch (_envelopeState) {
        case OFF:
            return 0;

        case ATTACK:
            if (_remaining <= _zeroThreshold) {
                _duration = to_duration(context->decay);
                _remaining = _duration;
                _envelopeState = DECAY;
                return FIX16_UNIT;
            }

            _remaining = _remaining - FIX16_UNIT;

            return linear_easing(_remaining, _duration, _envelopeStart,
                                 FIX16_UNIT);

        case DECAY:
            if (_remaining <= _zeroThreshold) {
                _envelopeState = SUSTAIN;
                return context->sustain;
            }

            _remaining = _remaining - FIX16_UNIT;

            return linear_easing(_remaining, _duration, FIX16_UNIT,
                                 context->sustain);

        case SUSTAIN:
            return context->sustain;

        case RELEASE:
            if (_remaining <= _zeroThreshold) {
                _envelopeState = OFF;
                return 0;
            }

            _remaining = _remaining - FIX16_UNIT;

            return linear_easing(_remaining, _duration, _envelopeStart, 0);

        default:
            return 0;
    }
}

void synth_envelope_init(uint16_t sampleRate) {
    // target sample rate is 44.1k which may not fit in a 16 bit value
    // once it is multiplied by a duration that is greater than ~1.5seconds
    // so scale it down
    _sampleRate = int2fix16(sampleRate >> SR_SCALE_FACTOR);
}