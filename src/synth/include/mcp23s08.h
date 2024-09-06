#ifndef SYNTH_MCP23S08_
#define SYNTH_MCP23S08_

#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "controller.h"

/// @brief initialise a mcp23s08
/// @param spi 
/// @param baudRate 
/// @param chip_select_pin 
/// @param clock_pin 
/// @param tx_pin 
/// @param rx_pin 
/// @return 
spi_device_t synth_mcp23s08_init(spi_inst_t* spi, size_t baud_rate, uint8_t chip_select_pin,
                       uint8_t clock_pin, uint8_t tx_pin, uint8_t rx_pin, uint8_t io_mask);

/// @brief read mcp23s08 channel
/// @param instance 
/// @param channel 
/// @return 0 or 1
uint8_t synth_mcp23s08_read(spi_device_t* instance, uint8_t channel);

void synth_mcp23s08_write(spi_device_t* instance, uint8_t channel, uint8_t value);

#endif