# CMakeLists.txt in the ultrasonic directory

# Define the target for the ultrasonic component
add_library(ultra ultrasonic.c ultrasonic.h)

target_compile_definitions(ultra PRIVATE
)


target_include_directories(ultra PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../ # for our FreeRTOSConfig.h
    ${CMAKE_CURRENT_LIST_DIR}/../../../.. # for our common lwipopts
)

# Set any additional properties or dependencies for the ultrasonic component
target_link_libraries(ultra
  PRIVATE
    hardware_timer
    hardware_gpio
    pico_stdlib
    m # math lib
    FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
)