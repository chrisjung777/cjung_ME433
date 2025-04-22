cmake_minimum_required(VERSION 3.13)

project(HW4 C CXX ASM)
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
pico_sdk_init()

add_executable(HW4 HW4.c)
set_target_properties(HW4 PROPERTIES OUTPUT_NAME HW4.elf)

pico_set_program_name(HW4 "HW4")
pico_set_program_version(HW4 "0.1")

pico_enable_stdio_usb(HW4 1)
pico_enable_stdio_uart(HW4 0)

target_link_libraries(HW4
    pico_stdlib
    hardware_spi
    pico_stdio_usb
)

target_include_directories(HW4 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(HW4)
