# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
#set(BOARD "mcxn947_protocard/mcxn947/cpu0")
set(BOARD "frdm_mcxn947/mcxn947/cpu0")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(oresat_zephyr_template LANGUAGES C)

# execute_process(COMMAND bash -c "oresat-configs canopennode od.yaml ../../common/od.yaml -d gen")

target_sources(app PRIVATE
  src/main.c
)

#zephyr_include_directories(od)
zephyr_include_directories(objdict)

# Compile in various demos depending on Kconfig values.
target_sources_ifdef(CONFIG_BLINK_DEMO      app PRIVATE src/blink.c)
target_sources_ifdef(CONFIG_I2C_SENSOR_DEMO app PRIVATE src/i2c_sensor.c)
target_sources_ifdef(CONFIG_DAC_DEMO        app PRIVATE src/dac.c)
target_sources_ifdef(CONFIG_ADC_DEMO        app PRIVATE src/adc.c)
target_sources_ifdef(CONFIG_CAN_DEMO        app PRIVATE src/can.c)
target_sources_ifdef(CONFIG_CAN_DEMO        app PRIVATE objdict/CO_OD.c)
#target_sources_ifdef(CONFIG_CAN_DEMO        app PRIVATE od/CO_OD.c)
target_sources_ifdef(CONFIG_GPIO_DEMO       app PRIVATE src/gpio.c)
