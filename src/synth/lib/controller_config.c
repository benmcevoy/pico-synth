

#include <stdio.h>

#include "../include/mcp23s08.h"
#include "../include/mcp3008.h"

#define SPI_CLOCK 2
#define SPI_TX 3
#define SPI_RX 4
#define SPI_CS0 5
#define SPI_CS1 6
#define BAUD_RATE 3600000

/// @brief controller0 is mcp3008 analog
static spi_device_t analog_controller0;

/// @brief controller1 is mcp23s08 gpio
static spi_device_t gpio_controller0;

#define CONTROLS_COUNT 5
control_t* controls;

void synth_controller_config_init() {
  // use CS1
  gpio_controller0 =
      synth_mcp23s08_init(spi0, BAUD_RATE, SPI_CS1, SPI_CLOCK, SPI_TX, SPI_RX, 0b10101010);

  // use CS0
  analog_controller0 =
      synth_mcp3008_init(spi0, BAUD_RATE, SPI_CS0, SPI_CLOCK, SPI_TX, SPI_RX);

  controls = malloc(sizeof(control_t) * CONTROLS_COUNT);

  // TODO: it would be PERFECT if this can configurable
  // adding a boot into web server would be terrific
  // so the whole thing can be configured
  // e.g. the control surface
  // the wavetables
  // or just read a file then init
  controls[0] = (control_t){.channel = 0,
                            .value = 0,
                            .action = CONTROL_ACTION_DELAY,
                            .control_type = CONTROL_TYPE_ANALOG,
                            .spi_device = &analog_controller0};
  controls[1] = (control_t){.channel = 1,
                            .value = 0,
                            .action = CONTROL_ACTION_DELAY_FEEDBACK,
                            .control_type = CONTROL_TYPE_ANALOG,
                            .spi_device = &analog_controller0};
  controls[2] = (control_t){.channel = 2,
                            .value = 0,
                            .action = CONTROL_ACTION_DELAY_DRY_WET_MIX,
                            .control_type = CONTROL_TYPE_ANALOG,
                            .spi_device = &analog_controller0};
  controls[3] = (control_t){.channel = 3,
                            .value = 0,
                            .action = CONTROL_ACTION_DETUNE,
                            .control_type = CONTROL_TYPE_ANALOG,
                            .spi_device = &analog_controller0};
  controls[4] = (control_t){.channel = 2,
                            .value = 0,
                            .action = CONTROL_ACTION_DELAY_FEEDBACK_INFINITE,
                            .control_type = CONTROL_TYPE_MOMENTRY,
                            .spi_device = &gpio_controller0};
}
