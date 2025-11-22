# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(oresat_zephyr_template LANGUAGES C)

target_sources(app PRIVATE src/main.c)

# Compile in various demos depending on Kconfig values.
target_sources_ifdef(CONFIG_BLINK_DEMO      app PRIVATE src/blink.c)
target_sources_ifdef(CONFIG_I2C_SENSOR_DEMO app PRIVATE src/i2c_sensor.c)
target_sources_ifdef(CONFIG_DAC_DEMO        app PRIVATE src/dac.c)
target_sources_ifdef(CONFIG_ADC_DEMO        app PRIVATE src/adc.c)
