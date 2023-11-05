# CMakeLists.txt in the motor directory

# Define the target for the encoder component
add_library(encoder encoder.c encoder.h)

target_compile_definitions(encoder PRIVATE
)

target_include_directories(encoder PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../ # for our FreeRTOSConfig.h
    ${CMAKE_CURRENT_LIST_DIR}/../../../.. # for our common lwipopts
)

# Set any additional properties or dependencies for the encoder component
target_link_libraries(encoder
  PRIVATE
    pico_stdlib
    hardware_gpio
    pico_time
    FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
)