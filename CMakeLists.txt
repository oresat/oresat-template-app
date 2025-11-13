# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app_template LANGUAGES C)

execute_process(COMMAND bash -c "oresat-configs canopennode ${CMAKE_CURRENT_SOURCE_DIR}/od.yaml ${CMAKE_CURRENT_SOURCE_DIR}/../../common/od.yaml -d gen")

target_sources(app PRIVATE src/main.c gen/OD.c)

zephyr_include_directories(gen)

