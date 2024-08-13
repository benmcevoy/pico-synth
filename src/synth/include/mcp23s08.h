#ifndef SYNTH_MCP23S08_
#define SYNTH_MCP23S08_

#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"

typedef struct Mcp23s08 {
    spi_inst_t* spi;
    uint8_t chip_select_pin;
    uint8_t clock_pin;
    uint8_t tx_pin;
    uint8_t rx_pin;
    uint actual_baud_rate;

} mcp23s08_t;

/// @brief initialise a mcp23008
/// @param spi 
/// @param baudRate 
/// @param chip_select_pin 
/// @param clock_pin 
/// @param tx_pin 
/// @param rx_pin 
/// @return 
mcp23s08_t synth_mcp23s08_init(spi_inst_t* spi, size_t baudRate, uint8_t chip_select_pin,
                       uint8_t clock_pin, uint8_t tx_pin, uint8_t rx_pin);

/// @brief read mcp23008 channel
/// @param instance 
/// @param channel 
/// @return a value between 0 and 255
uint8_t synth_mcp23s08_read(mcp23s08_t* instance, uint8_t channel);

void synth_mcp23s08_write(mcp23s08_t* instance, uint8_t gpio_mask);

#endif