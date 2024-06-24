#include <stdio.h>
#include <stdlib.h>

#include "hardware/spi.h"
#include "mcp3008.h"
#include "pico/stdlib.h"

#define spi_clock 2
#define spi_tx 3
#define spi_rx 4
#define spi_cs 5

int main() {
  stdio_init_all();

// baud rate depends on VDD 200kps is recommended > 4.7V

  mcp3008_t instance =
      mcp3008_init(spi0, 200000, spi_cs, spi_clock, spi_tx, spi_rx);

  uint16_t oldValue = 0;
  uint16_t value = 0;
  uint8_t threshold = 13;

  while (1) {
    value = mcp3008_read(&instance, 0);

    if (abs(oldValue - value) > threshold) {
      oldValue = value;
      printf("value = %u\n", value);
    }

    sleep_ms(10);
  }
}