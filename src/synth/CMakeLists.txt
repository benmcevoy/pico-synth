set(projectName "synth")
set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")

add_executable(${projectName} main.c 
    usb_descriptors.c
    lib/envelope.c
    lib/filter.c
    lib/midi.c
    lib/pitchtable.c
    lib/waveform.c)

# configure UART
pico_enable_stdio_usb(${projectName} 0)
pico_enable_stdio_uart(${projectName} 1)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(${projectName} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

# pull in common dependencies
target_link_libraries(${projectName} PUBLIC pico_stdlib hardware_pwm hardware_dma tinyusb_device tinyusb_board pico_unique_id)

# allow stable overclock - set SPI flash clock divider - necessary over 300MHz 
#pico_define_boot_stage2(${projectName} ${PICO_DEFAULT_BOOT_STAGE2_FILE})
#target_compile_definitions(${projectName} PRIVATE PICO_FLASH_SPI_CLKDIV=4)
#pico_set_boot_stage2(${projectName})

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${projectName})
