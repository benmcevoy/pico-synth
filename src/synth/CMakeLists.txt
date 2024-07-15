set(projectName "synth")
set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")

add_executable(${projectName} main.c 
    usb_midi_host_app_driver.c
    usb_midi_host.c
    usb_descriptors.c
    lib/delay.c
    lib/controller.c
    lib/envelope.c
    lib/filter.c
    lib/mcp3008.c
    lib/midi.c
    lib/waveform.c)

#pico_set_binary_type(${projectName} no_flash)

# allow stable overclock - set SPI flash clock divider - necessary over 300MHz 
pico_define_boot_stage2(pico-clock-div4 ${PICO_DEFAULT_BOOT_STAGE2_FILE})
target_compile_definitions(pico-clock-div4 PRIVATE PICO_FLASH_SPI_CLKDIV=4)
pico_set_boot_stage2(${projectName} pico-clock-div4)

# configure UART
pico_enable_stdio_usb(${projectName} 0)
pico_enable_stdio_uart(${projectName} 1)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(${projectName} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

# pull in common dependencies
target_link_libraries(${projectName} PUBLIC 
    pico_stdlib 
    hardware_pwm 
    hardware_dma 
    hardware_spi
    tinyusb_board
    tinyusb_host
    tinyusb_device
    tinyusb_pico_pio_usb
    pico_bootsel_via_double_reset
    pico_multicore
    pico_unique_id
    )


# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${projectName})