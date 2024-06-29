#ifndef SYNTH_MCP3008_
#define SYNTH_MCP3008_

#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"

typedef struct Mcp3008 {
    spi_inst_t* spi;
    uint8_t chip_select_pin;
    uint8_t clock_pin;
    uint8_t tx_pin;
    uint8_t rx_pin;
    uint actual_baud_rate;

} mcp3008_t;

/// @brief initialise a
/// @param spi 
/// @param baudRate 
/// @param chip_select_pin 
/// @param clock_pin 
/// @param tx_pin 
/// @param rx_pin 
/// @return 
mcp3008_t synth_mcp3008_init(spi_inst_t* spi, size_t baudRate, uint8_t chip_select_pin,
                       uint8_t clock_pin, uint8_t tx_pin, uint8_t rx_pin);

/// @brief read mcp3008 channel
/// @param instance 
/// @param channel 
/// @return a value between 0 and 1023
uint16_t synth_mcp3008_read(mcp3008_t* instance, uint8_t channel);


#endif