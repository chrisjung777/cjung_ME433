cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico2)

include(pico_sdk_import.cmake)
project(display3 C CXX ASM)
pico_sdk_init()

add_executable(display3
    display3.c
    ssd1306.c
)

target_link_libraries(display3
    pico_stdlib
    hardware_i2c
    hardware_adc
    hardware_timer
)

target_include_directories(display3 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

pico_enable_stdio_usb(display3 1)
pico_enable_stdio_uart(display3 0)
pico_add_extra_outputs(display3)
