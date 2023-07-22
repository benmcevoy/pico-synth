#include "../include/envelope.h"

#define OFF 0
#define ATTACK 1
#define DECAY 2
#define SUSTAIN 3
#define RELEASE 4

// TODO: this state will be a problem if/when there are multiple voices
float _sampleRate = 0.f;
float _counter = 0.f;
float _duration = 0.f;
char _state = OFF;

float elapsed(float time, float duration) {
    return (duration - time) / duration;
}

bool has_elapsed(float time, float duration) {
    return elapsed(time, duration) >= 1.f;
}

float linear_easing(float time, float duration, float start, float end) {
    return has_elapsed(time, duration)
               ? end
               : start + elapsed(time, duration) * (end - start);
}

void synth_envelope_note_on(Voice_t* voice) { voice->triggerAttack = true; }

void synth_envelope_note_off(Voice_t* voice) { voice->triggerAttack = false; }

float synth_envelope_process(Voice_t* voice) {
    if (voice->triggerAttack && (_state == OFF || _state == RELEASE)) {
        _duration = voice->attack * _sampleRate;
        _counter = _duration;
        _state = ATTACK;
    }

    if (!voice->triggerAttack &&
        (_state == ATTACK || _state == DECAY || _state == SUSTAIN)) {
        _duration = voice->release * _sampleRate;
        _counter = _duration;
        _state = RELEASE;
    }

    switch (_state) {
        case OFF:
            return 0.f;

        case ATTACK:
            if (_counter == 0) {
                _duration = voice->decay * _sampleRate;
                _counter = _duration;
                _state = DECAY;
                return 1.f;
            }

            return linear_easing(_counter--, _duration, 0.f, 1.f);

        case DECAY:
            if (_counter == 0) {
                _state = SUSTAIN;
                return voice->sustain;
            }

            return linear_easing(_counter--, _duration, 1.f, voice->sustain);

        case SUSTAIN:
            return voice->sustain;

        case RELEASE:
            if (_counter == 0) {
                _state = OFF;
                return 0.f;
            }

            return linear_easing(_counter--, _duration, voice->sustain, 0.f);

        default:
            return 0.f;
    }
}

void synth_envelope_init(float sampleRate) { _sampleRate = sampleRate; }