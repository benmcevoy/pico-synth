set(projectName "synth")

add_executable(${projectName} main.c)

# enable usb output, disable uart output
pico_enable_stdio_usb(${projectName} 1)
pico_enable_stdio_uart(${projectName} 0)

# pull in common dependencies
target_link_libraries(${projectName} pico_stdlib hardware_pwm pico_multicore)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${projectName})


