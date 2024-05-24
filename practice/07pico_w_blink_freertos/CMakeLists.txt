# Changing from pico to pico w board so running some blinky stuff

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# have to set this manually
# set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})

set(PICO_BOARD pico_w CACHE STRING "Board type")

# pull in pico sdk (must before project according to the pico documents)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(pico_w_blink C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(pico_w_blink pico_w_blink.c )

pico_set_program_name(pico_w_blink "pico_w_blink")
pico_set_program_version(pico_w_blink "0.1")

pico_enable_stdio_uart(pico_w_blink 1)
pico_enable_stdio_usb(pico_w_blink 0)

# Add the standard library to the build
target_link_libraries(pico_w_blink
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(pico_w_blink PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(pico_w_blink 
        pico_cyw43_arch_none
        )

pico_add_extra_outputs(pico_w_blink)
