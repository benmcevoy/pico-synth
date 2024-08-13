#include "../include/mcp23s08.h"

#include <stdio.h>

// address a0, a1 is 0,0
#define ADDRESS_WRITE 0x40
#define ADDRESS_READ 0x41
// spi registers - refer datasheet
#define IODIR 0x00
#define IPOL 0x01
#define GPINTEN 0x02
#define DEFVAL 0x03
#define INTCON 0x04
#define IOCON 0x05
#define GPPU 0x06
#define INTF 0x07
#define INTCAP 0x08
#define GPIO 0x09
#define OLAT 0x0A

static void write(mcp23s08_t* instance, uint8_t spi_register, uint8_t value) {
  uint8_t buf_out[3] = {ADDRESS_WRITE, spi_register, value};

  gpio_put(instance->chip_select_pin, 0);
  
  spi_write_blocking(instance->spi, buf_out, 3);

  gpio_put(instance->chip_select_pin, 1);
}

mcp23s08_t synth_mcp23s08_init(spi_inst_t* spi, size_t baud_rate,
                               uint8_t chip_select_pin, uint8_t clock_pin,
                               uint8_t tx_pin, uint8_t rx_pin) {
  gpio_init(chip_select_pin);
  gpio_set_dir(chip_select_pin, GPIO_OUT);
  uint actual_baud_rate = spi_init(spi, baud_rate);

  spi_set_format(spi,         // SPI
                 8,           // bits per transfer
                 SPI_CPOL_0,  // Polarity
                 SPI_CPHA_0,  // Phase
                 SPI_MSB_FIRST);

  // Chip select is active-low, initialise to high
  gpio_put(chip_select_pin, 1);

  gpio_set_function(clock_pin, GPIO_FUNC_SPI);
  gpio_set_function(tx_pin, GPIO_FUNC_SPI);
  gpio_set_function(rx_pin, GPIO_FUNC_SPI);

  mcp23s08_t instance = {.spi = spi,
                         .actual_baud_rate = actual_baud_rate,
                         .tx_pin = tx_pin,
                         .clock_pin = clock_pin,
                         .rx_pin = rx_pin,
                         .chip_select_pin = chip_select_pin};

  // TODO: iodir mask should be a parameter
  // set iodir to output for all pins
  // e.g. write(instance, IODIR, 0b00001000);  sets pin 4 to input
  write(&instance, IODIR, 0);

  return instance;
};

uint8_t synth_mcp23s08_read(mcp23s08_t* instance, uint8_t channel) {
  // TODO: this is just a copy from mcp3008
  static uint8_t buf_in[3] = {0, 0, 0};
  static uint8_t buf_out[3] = {ADDRESS_READ, GPIO, 0};

  // single ended mode
  buf_out[1] = (8 + channel) << 4;

  gpio_put(instance->chip_select_pin, 0);

  spi_write_read_blocking(instance->spi, buf_out, buf_in, 3);

  gpio_put(instance->chip_select_pin, 1);

  return ((buf_in[1] & 0x03) << 8) | buf_in[2];
}

void synth_mcp23s08_write(mcp23s08_t* instance, uint8_t gpio_mask) {
  // write gpio
  write(instance, GPIO, 1);
}
