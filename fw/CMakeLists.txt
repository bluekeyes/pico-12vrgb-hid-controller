cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico CACHE STRING "Board type")

include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.5.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.5.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(pico_12vrgb_controller C CXX ASM)

pico_sdk_init()

add_executable(pico_12vrgb_controller)

pico_set_program_name(pico_12vrgb_controller "pico_12vrgb_controller")
pico_set_program_version(pico_12vrgb_controller "0.1")

target_compile_options(pico_12vrgb_controller PRIVATE -Werror)

target_include_directories(pico_12vrgb_controller PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/src/include
  ${CMAKE_CURRENT_LIST_DIR}/src
)

target_sources(pico_12vrgb_controller PRIVATE
  src/color/color.c
  src/controller/animations/fade.c
  src/controller/controller.c
  src/controller/persist.c
  src/controller/sensor.c
  src/debug.c
  src/device/lamp.c
  src/device/temperature.c
  src/main.c
  src/usb_descriptors.c
  src/usb_hid.c
)

pico_enable_stdio_uart(pico_12vrgb_controller 1)

target_link_libraries(pico_12vrgb_controller
  hardware_adc
  hardware_flash
  hardware_pwm
  hardware_watchdog
  pico_bootrom
  pico_stdlib
  pico_unique_id
  tinyusb_device
)

pico_add_extra_outputs(pico_12vrgb_controller)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG_USBHID=1 DEBUG_ANIMATE=1)
endif()
