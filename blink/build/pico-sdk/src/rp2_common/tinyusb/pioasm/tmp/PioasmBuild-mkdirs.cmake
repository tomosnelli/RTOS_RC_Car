# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/tom/RTOS_RC_Car/pico-sdk/tools/pioasm"
  "/home/tom/RTOS_RC_Car/blink/build/pioasm"
  "/home/tom/RTOS_RC_Car/blink/build/pico-sdk/src/rp2_common/tinyusb/pioasm"
  "/home/tom/RTOS_RC_Car/blink/build/pico-sdk/src/rp2_common/tinyusb/pioasm/tmp"
  "/home/tom/RTOS_RC_Car/blink/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
  "/home/tom/RTOS_RC_Car/blink/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src"
  "/home/tom/RTOS_RC_Car/blink/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/tom/RTOS_RC_Car/blink/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/tom/RTOS_RC_Car/blink/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
