# CMakeLists.txt in the magnetometer directory

# Define the target for the magnetometer component
add_library(magnetometer magnetometer.c magnetometer.h)

target_compile_definitions(magnetometer PRIVATE
)


target_include_directories(magnetometer PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../ # for our FreeRTOSConfig.h
    ${CMAKE_CURRENT_LIST_DIR}/../../../.. # for our common lwipopts
)

# Set any additional properties or dependencies for the magnetometer component
target_link_libraries(magnetometer
  PRIVATE
    hardware_adc
    pico_stdlib
    FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
)