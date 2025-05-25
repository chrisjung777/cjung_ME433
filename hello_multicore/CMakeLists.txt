cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico2 CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(hello_multicore C CXX ASM)

pico_sdk_init()


add_executable(hello_multicore
        multicore.c
        )

target_link_libraries(hello_multicore
        pico_stdlib
        pico_multicore
        hardware_adc
        hardware_gpio)

pico_add_extra_outputs(hello_multicore)

pico_enable_stdio_usb(hello_multicore 1)
pico_enable_stdio_uart(hello_multicore 0)
