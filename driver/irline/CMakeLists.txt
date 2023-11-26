# CMakeLists.txt in the motor directory

# Define the target for the motor component
add_library(ir ir.c)

target_compile_definitions(ir PRIVATE
)

target_include_directories(ir PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../ # for our FreeRTOSConfig.h
    ${CMAKE_CURRENT_LIST_DIR}/../../../.. # for our common lwipopts
)

# Set any additional properties or dependencies for the motor component
target_link_libraries(ir
  PRIVATE
    hardware_pwm
    pico_stdlib
    FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
)