# CMakeLists.txt in the motor directory

# Define the target for the motor component
add_library(motor motor.c motor.h)

target_compile_definitions(motor PRIVATE
)

target_include_directories(motor PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../ # for our FreeRTOSConfig.h
    ${CMAKE_CURRENT_LIST_DIR}/../../../.. # for our common lwipopts
)

# Set any additional properties or dependencies for the motor component
target_link_libraries(motor
  PRIVATE
    hardware_pwm
    pico_stdlib
    FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
)