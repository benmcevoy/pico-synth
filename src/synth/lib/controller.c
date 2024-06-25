#include "../include/controller.h"

#include <stdio.h>

#include "../include/mcp3008.h"

#define spi_clock 2
#define spi_tx 3
#define spi_rx 4
#define spi_cs 5

#define CONTROLS_COUNT 3

static uint16_t _sampleRate = 0;
const uint8_t _threshold = 13;
static mcp3008_t _controller;
control_t* controls;

void synth_controller_init(uint16_t sampleRate) {
  _sampleRate = sampleRate;
  controls = malloc(sizeof(control_t) * CONTROLS_COUNT);

  controls[0] = (control_t){.channel = 0, .value = 0};
  controls[1] = (control_t){.channel = 1, .value = 0};
  controls[2] = (control_t){.channel = 2, .value = 0};

  _controller =
      synth_mcp3008_init(spi0, 120000, spi_cs, spi_clock, spi_tx, spi_rx);
}

void synth_controller_task(AudioContext_t* context) {
  // read channels
  for (size_t i = 0; i < CONTROLS_COUNT; i++) {
    uint16_t value = synth_mcp3008_read(&_controller, controls[i].channel);

    if (abs(controls[i].value - value) > _threshold) {
      controls[i].value = value;
    }
  }

  // update state
  // delay is rated to sample rate
  context->delay = _sampleRate * controls[1].value / 1024.f;

  // shift 6 to make 10 bit number a 16 bit number
  context->delayGain = controls[2].value << 6;
}