#ifndef SYNTH_MCP3008_
#define SYNTH_MCP3008_

#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "controller.h"

/// @brief initialise a
/// @param spi 
/// @param baudRate 
/// @param chip_select_pin 
/// @param clock_pin 
/// @param tx_pin 
/// @param rx_pin 
/// @return 
spi_device_t synth_mcp3008_init(spi_inst_t* spi, size_t baud_rate, uint8_t chip_select_pin,
                       uint8_t clock_pin, uint8_t tx_pin, uint8_t rx_pin);

/// @brief read mcp3008 channel
/// @param instance 
/// @param channel 
/// @return a value between 0 and 1023
uint16_t synth_mcp3008_read(spi_device_t* instance, uint8_t channel);


#endif