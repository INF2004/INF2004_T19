add_executable(ultrasonic ultrasonic.c)

# Pull in common dependencies
target_link_libraries(ultrasonic pico_stdlib hardware_gpio hardware_timer)

# Create map/bin/hex file, etc.
pico_add_extra_outputs(ultrasonic)

# Enable USB output for printf
pico_add_extra_outputs(ultrasonic)

pico_enable_stdio_usb(ultrasonic 1)
pico_enable_stdio_uart(ultrasonic 0)
