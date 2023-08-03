#include "../include/envelope.h"

static fix16 _sampleRate = 0;
static int SR_FACTOR = 4;
static EnvelopeState_t _envelopeState = OFF;
static fix16 _time = 0;
static fix16 _duration = 0;
static fix16 _envelopeStart = 0;

fix16 to_duration(fix16 value) {
    return multfix16(value, _sampleRate) << SR_FACTOR;
}

static fix16 elapsed(fix16 time, fix16 duration) {
    return divfix16((duration - time), duration);
}

static bool has_elapsed(fix16 time, fix16 duration) {
    return elapsed(time, duration) >= FIX16_UNIT;
}

static fix16 linear_easing(fix16 time, fix16 duration, fix16 start, fix16 end) {
    return has_elapsed(time, duration)
               ? end
               : start + multfix16(elapsed(time, duration), (end - start));
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
        _time = _duration;
        _envelopeState = ATTACK;
        _envelopeStart = context->envelope;
    }

    if (!context->triggerAttack &&
        (_envelopeState == ATTACK || _envelopeState == DECAY ||
         _envelopeState == SUSTAIN)) {
        _duration = to_duration(context->release);
        _time = _duration;
        _envelopeState = RELEASE;
        _envelopeStart = context->envelope;
    }

    switch (_envelopeState) {
        case OFF:
            return 0;

        case ATTACK:
            if (_time == 0) {
                _duration = to_duration(context->decay);
                _time = _duration;
                _envelopeState = DECAY;
                return FIX16_UNIT;
            }

            _time = _time - FIX16_UNIT;

            return linear_easing(_time, _duration, _envelopeStart, FIX16_UNIT);

        case DECAY:
            if (_time == 0) {
                _envelopeState = SUSTAIN;
                return context->sustain;
            }

            _time = _time - FIX16_UNIT;

            return linear_easing(_time, _duration, FIX16_UNIT,
                                 context->sustain);

        case SUSTAIN:
            return context->sustain;

        case RELEASE:
            if (_time == 0) {
                _envelopeState = OFF;
                return 0;
            }

            _time = _time - FIX16_UNIT;

            return linear_easing(_time, _duration, _envelopeStart, 0);

        default:
            return 0;
    }
}

void synth_envelope_init(uint32_t sampleRate) {
    // assume max sample rate of 250kHz
    // will not fit in fix16, so divide by 4
    _sampleRate = int2fix16((uint16_t)sampleRate >> SR_FACTOR);
}