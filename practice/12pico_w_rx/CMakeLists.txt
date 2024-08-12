# practice 11 freeRTOS + BLE

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico_w CACHE STRING "Board type")

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(pico_w_freertos_rf C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(pico_w_freertos_rf pico_w_freertos_rf.c )

pico_set_program_name(pico_w_freertos_rf "pico_w_freertos_rf")
pico_set_program_version(pico_w_freertos_rf "0.1")

pico_enable_stdio_uart(pico_w_freertos_rf 1)
pico_enable_stdio_usb(pico_w_freertos_rf 0)

# Add the standard library to the build
# target_link_libraries(pico_w_freertos_rf
#        pico_stdlib)

# Add the standard include files to the build
target_include_directories(pico_w_freertos_rf PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(
        pico_w_freertos_rf
        pico_stdlib
        hardware_pwm
        pico_cyw43_arch_none
        FreeRTOS-Kernel-Heap4
        )

pico_add_extra_outputs(pico_w_freertos_rf)
