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

static uint8_t channel_mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};

static void write(spi_device_t* instance, uint8_t spi_register, uint8_t value) {
  uint8_t buf_out[3] = {ADDRESS_WRITE, spi_register, value};

  gpio_put(instance->chip_select_pin, 0);

  spi_write_blocking(instance->spi, buf_out, 3);

  gpio_put(instance->chip_select_pin, 1);
}

spi_device_t synth_mcp23s08_init(spi_inst_t* spi, size_t baud_rate,
                                 uint8_t chip_select_pin, uint8_t clock_pin,
                                 uint8_t tx_pin, uint8_t rx_pin,
                                 uint8_t io_mask) {
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

  spi_device_t instance = {.spi = spi,
                           .actual_baud_rate = actual_baud_rate,
                           .tx_pin = tx_pin,
                           .clock_pin = clock_pin,
                           .rx_pin = rx_pin,
                           .chip_select_pin = chip_select_pin};

  // set iodir  - 0 is output 1 is input
  write(&instance, IODIR, io_mask);
  // set pull up and use active low
  write(&instance, GPPU, io_mask);
  // invert
  write(&instance, IPOL, io_mask);
  // set all low
  write(&instance, GPIO, 0);

  return instance;
};

uint8_t synth_mcp23s08_read(spi_device_t* instance, uint8_t channel) {
  static uint8_t buf_in[3] = {0, 0, 0};
  static uint8_t buf_out[3] = {ADDRESS_READ, GPIO, 0};

  uint8_t mask = channel_mask[channel];

  buf_out[2] = mask;

  gpio_put(instance->chip_select_pin, 0);

  spi_write_read_blocking(instance->spi, buf_out, buf_in, 3);

  gpio_put(instance->chip_select_pin, 1);

  // TODO:
  // this is not super efficient - as it reads a byte with the state of all
  // input pins so i end up doing more reads then I need but OK for now return 0
  // or 1
  return (buf_in[2] & mask) == mask;
}

void synth_mcp23s08_write(spi_device_t* instance, uint8_t channel,
                          uint8_t value) {
  // read current from latch register
  static uint8_t buf_in[3] = {0, 0, 0};
  static uint8_t buf_out[3] = {ADDRESS_READ, OLAT, 0};

  gpio_put(instance->chip_select_pin, 0);
  spi_write_read_blocking(instance->spi, buf_out, buf_in, 3);
  gpio_put(instance->chip_select_pin, 1);
  uint8_t current = buf_in[2];

  // set channel bit
  uint8_t mask = channel_mask[channel];
  value = (value == 1 ? mask : 0);

  // ~ is complement/inverse
  write(instance, GPIO, (current & ~mask) | (value & mask));
}
