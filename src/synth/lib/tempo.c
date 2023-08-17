#include "../include/tempo.h"

static uint32_t _interval = 0;

void synth_tempo_set_bpm(Tempo_t* tempo, uint8_t bpm) {
    tempo->durationInSamples = 60 / (float)bpm * SAMPLE_RATE;
    _interval = tempo->durationInSamples;
}

void synth_tempo_process(Tempo_t* tempo) {
    if (--_interval == 0) {
        tempo->isBeat = true;
        _interval = tempo->durationInSamples;
        return;
    }

    tempo->isBeat = false;
    return;
}

void synth_tempo_init(Tempo_t* tempo, uint8_t bpm) {
    synth_tempo_set_bpm(tempo, bpm);
}