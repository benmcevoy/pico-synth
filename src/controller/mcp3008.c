#include "mcp3008.h"

mcp3008_t mcp3008_init(spi_inst_t* spi, size_t baudRate, uint8_t chip_select_pin,
                       uint8_t clock_pin, uint8_t tx_pin, uint8_t rx_pin) {
  gpio_init(chip_select_pin);
  gpio_set_dir(chip_select_pin, GPIO_OUT);
  uint actualBaudRate = spi_init(spi, baudRate);

  spi_set_format(
      spi,  // SPI
      8,    // bits per transfer (8 bits not 10 because mcp3008 deals in 8 bits)
      SPI_CPOL_0,  // Polarity
      SPI_CPHA_0,  // Phase
      SPI_MSB_FIRST);

  // Chip select is active-low, so we'll initialise it to a driven-high state
  gpio_put(chip_select_pin, 1);

  gpio_set_function(clock_pin, GPIO_FUNC_SPI);
  gpio_set_function(tx_pin, GPIO_FUNC_SPI);
  gpio_set_function(rx_pin, GPIO_FUNC_SPI);

  return (mcp3008_t){.spi = spi,
                     .actualBaudRate = actualBaudRate,
                     .tx_pin = tx_pin,
                     .clock_pin = clock_pin,
                     .rx_pin = rx_pin,
                     .chip_select_pin = chip_select_pin};
};

uint16_t mcp3008_read(mcp3008_t* instance, uint8_t channel) {
  static uint8_t buf_in[3] = {0, 0, 0};
  static uint8_t buf_out[3] = {0, 0, 0};

  buf_out[0] = 1;
  // single ended mode
  buf_out[1] = (8 + channel) << 4;

  gpio_put(instance->chip_select_pin, 0);

  spi_write_read_blocking(instance->spi, buf_out, buf_in, 3);

  gpio_put(instance->chip_select_pin, 1);

  return ((buf_in[1] & 0x03) << 8) | buf_in[2];
}
