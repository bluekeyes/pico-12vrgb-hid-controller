cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico CACHE STRING "Board type")

include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(rgb_controller C CXX ASM)

pico_sdk_init()

add_executable(rgb_controller)

pico_set_program_name(rgb_controller "rgb_controller")
pico_set_program_version(rgb_controller "0.1")

target_compile_options(rgb_controller PRIVATE -Werror)

target_sources(rgb_controller PRIVATE
  src/color/color.c
  src/controller/animations/fade.c
  src/controller/controller.c
  src/controller/persist.c
  src/device/lamp.c
  src/device/temperature.c
  src/main.c
  src/usb_descriptors.c
  src/usb_hid.c
)

target_include_directories(rgb_controller PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/src/include
  ${CMAKE_CURRENT_LIST_DIR}/src
)

pico_enable_stdio_uart(rgb_controller 1)

# Add the standard library to the build
target_link_libraries(rgb_controller
  hardware_adc
  hardware_flash
  hardware_pwm
  hardware_watchdog
  pico_bootrom
  pico_stdlib
  pico_unique_id
  tinyusb_device
)

pico_add_extra_outputs(rgb_controller)
