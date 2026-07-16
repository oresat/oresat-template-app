# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(BOARD "frdm_mcxn947/mcxn947/cpu0")

set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(PATCHLEVEL "0")
set(VERSION_TWEAK "0")
set(EXTRAVERSION "00000000")

find_package(Git QUIET REQUIRED)

execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RESULT_VARIABLE res
    OUTPUT_VARIABLE EXTRAVERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

set_property(GLOBAL APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/.git/index")

#
# utilize settings to autogenerate a header file to adjust the build inside the C code
#
configure_file (
   "${CMAKE_SOURCE_DIR}/src/VERSION.in"
   "${CMAKE_SOURCE_DIR}/VERSION"
   )

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(oresat_template LANGUAGES C)

include_directories("${PROJECT_SOURCE_DIR}") # needed so c code can find this header

# (*) zephyr_include_directories(od)
zephyr_include_directories(objdict)

# (*)=disabled until oresat-configs supports code generation for Zephyr's canopennode
# (*) execute_process(COMMAND bash -c "oresat-configs canopennode od.yaml ../../common/od.yaml -d gen")

target_sources(app PRIVATE
  src/main.c
)

# Compile in various demos depending on Kconfig values.
target_sources_ifdef(CONFIG_BLINK_DEMO      app PRIVATE src/blink.c)
target_sources_ifdef(CONFIG_I2C_SENSOR_DEMO app PRIVATE src/i2c_sensor.c)
target_sources_ifdef(CONFIG_DAC_DEMO        app PRIVATE src/dac.c)
target_sources_ifdef(CONFIG_ADC_DEMO        app PRIVATE src/adc.c)
target_sources_ifdef(CONFIG_CAN_DEMO        app PRIVATE src/can.c)
target_sources_ifdef(CONFIG_GPIO_DEMO       app PRIVATE src/gpio.c)
target_sources_ifdef(CONFIG_CAN_DEMO        app PRIVATE objdict/CO_OD.c)
# (*) target_sources_ifdef(CONFIG_CAN_DEMO       app PRIVATE od/CO_OD.c)

