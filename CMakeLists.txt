#  it's a workaround but pls dont run python in cmake.
#  run makefsdata.py separately in the main directory instead.

add_subdirectory(driver)

add_executable(maze_sys
        maze.c
)
target_compile_definitions(maze_sys PRIVATE
)
target_include_directories(maze_sys PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(maze_sys
    PRIVATE
        pico_stdlib
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        UIComponent
        motor
        encoder
        ultrasonic
        magnetometer
)
pico_add_extra_outputs(maze_sys)
pico_enable_stdio_usb(maze_sys 1)