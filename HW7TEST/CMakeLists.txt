cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(HW7TEST C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add your executable and source files
add_executable(HW7TEST
    HW7TEST.c
    ssd1306.c
)

# Set program name and version (optional)
pico_set_program_name(HW7TEST "HW7TEST")
pico_set_program_version(HW7TEST "0.1")

# Enable USB output for printf (disable UART)
pico_enable_stdio_usb(HW7TEST 1)
pico_enable_stdio_uart(HW7TEST 0)

# Link to Pico standard library and hardware libraries
target_link_libraries(HW7TEST
    pico_stdlib
    hardware_i2c
    hardware_adc
)

# Add the standard include files to the build
target_include_directories(HW7TEST PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Generate UF2, bin, and other output formats
pico_add_extra_outputs(HW7TEST)
