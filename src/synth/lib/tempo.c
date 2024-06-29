#include "../include/tempo.h"

static uint32_t interval = 0;

void synth_tempo_set_bpm(tempo_t* tempo, uint8_t bpm) {
    tempo->duration_in_samples = 60 / (float)bpm * SAMPLE_RATE;
    interval = tempo->duration_in_samples;
}

void synth_tempo_process(tempo_t* tempo) {
    if (--interval == 0) {
        tempo->isBeat = true;
        interval = tempo->duration_in_samples;
        return;
    }

    tempo->isBeat = false;
    return;
}

void synth_tempo_init(tempo_t* tempo, uint8_t bpm) {
    synth_tempo_set_bpm(tempo, bpm);
}