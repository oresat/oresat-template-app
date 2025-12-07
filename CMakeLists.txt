# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(oresat_zephyr_template LANGUAGES C)

execute_process(COMMAND bash -c "oresat-configs canopennode ${CMAKE_CURRENT_SOURCE_DIR}/od.yaml ${CMAKE_CURRENT_SOURCE_DIR}/../../common/od.yaml -d gen")

# Compile in various demos depending on Kconfig values.
target_sources_ifdef(CONFIG_BLINK_DEMO      app PRIVATE src/blink.c)
target_sources_ifdef(CONFIG_I2C_SENSOR_DEMO app PRIVATE src/i2c_sensor.c)
target_sources_ifdef(CONFIG_DAC_DEMO        app PRIVATE src/dac.c)
target_sources_ifdef(CONFIG_ADC_DEMO        app PRIVATE src/adc.c)

target_sources(app PRIVATE src/main.c gen/OD.c)
zephyr_include_directories(gen)
