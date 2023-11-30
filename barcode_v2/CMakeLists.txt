add_executable(barcode_v2 barcode_v2.c)

# pull in common dependencies
target_link_libraries(barcode_v2 hardware_adc hardware_pwm hardware_timer hardware_gpio pico_stdlib)

# enable usb output, enable uart output
pico_enable_stdio_usb(barcode_v2 1)
pico_enable_stdio_uart(barcode_v2 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(barcode_v2)

# add url via pico_set_program_url
example_auto_set_url(barcode_v2)