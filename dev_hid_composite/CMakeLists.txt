cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD pico_w) 

include(pico_sdk_import.cmake)

project(hid_mouse_modes C CXX ASM)
pico_sdk_init()

add_executable(hid_mouse_modes
    main.c
    usb_descriptors.c
)

target_include_directories(hid_mouse_modes PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(hid_mouse_modes
    pico_stdlib
    pico_unique_id
    hardware_gpio
    tinyusb_device
    tinyusb_board
)

pico_enable_stdio_usb(hid_mouse_modes 1)
pico_enable_stdio_uart(hid_mouse_modes 0)

pico_add_extra_outputs(hid_mouse_modes)
