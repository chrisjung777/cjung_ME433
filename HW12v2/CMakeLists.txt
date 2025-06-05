cmake_minimum_required(VERSION 3.13)

project(camera1 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(pico_sdk_import.cmake)

pico_sdk_init()

add_executable(camera1
    camera1.c
)

set_target_properties(camera1 PROPERTIES SUFFIX ".elf")

target_link_libraries(camera1
    pico_stdlib
    hardware_i2c
    hardware_gpio
    hardware_pwm
)

pico_enable_stdio_usb(camera1 1)
pico_enable_stdio_uart(camera1 0)

pico_add_extra_outputs(camera1)
