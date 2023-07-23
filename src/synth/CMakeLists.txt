set(projectName "synth")

add_executable(${projectName} main.c 
    lib/waveform.c 
    lib/envelope.c)

# enable usb output, disable uart output
pico_enable_stdio_usb(${projectName} 0)
pico_enable_stdio_uart(${projectName} 1)

# pull in common dependencies
target_link_libraries(${projectName} pico_stdlib hardware_pwm hardware_dma)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${projectName})
