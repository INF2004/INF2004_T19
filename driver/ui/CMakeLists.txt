# CMakeLists.txt in the ui directory

# Define the target for the UI component
add_library(UIComponent ui.c ui.h)

target_compile_definitions(UIComponent PRIVATE
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
    LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)
    PING_USE_SOCKETS=1
)


target_include_directories(UIComponent PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../ # for our FreeRTOSConfig.h
    ${CMAKE_CURRENT_LIST_DIR}/../../../.. # for our common lwipopts
)

# Set any additional properties or dependencies for the UI component
target_link_libraries(UIComponent
  PRIVATE
    pico_cyw43_arch_lwip_sys_freertos
    pico_lwip_http
    hardware_adc
    pico_stdlib
    FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
)