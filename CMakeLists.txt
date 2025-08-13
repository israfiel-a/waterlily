#########################################################################################
## This is the CMake build file for the Waterlily engine. It handles all build 
## configuration mechanisms, like selecting a windowing backend or turning on/off flags.
##
## Copyright (C) 2025 - Israfil Argos
##
## This program is free software: you can redistribute it and/or modify it under the 
## terms of the GNU General Public License as published by the Free Software 
## Foundation, either version 3 of the License, or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful, but WITHOUT ANY 
## WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
## PARTICULAR PURPOSE.  See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along with this 
## program.  If not, see <https://www.gnu.org/licenses/>.
#########################################################################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project("Waterlily" LANGUAGES C VERSION 0.0.1 DESCRIPTION 
    "A comprehensive library for creating RPG games in C23 on Linux.")

#########################################################################################
## Some quick checks to make sure the operating environment is good.
#########################################################################################

if(NOT LINUX)
    message(FATAL_ERROR "This operating system is not supported.")
endif()

if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "This compiler is not supported.")
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE)
    message(WARNING "No build type given, assuming debug.")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

#########################################################################################
## Source file wrangling and processing.
#########################################################################################

set(WATERLILY_SOURCES 
    "${Waterlily_SOURCE_DIR}/Source/Engine.c"
    "${Waterlily_SOURCE_DIR}/Source/Files.c"
    "${Waterlily_SOURCE_DIR}/Source/Input.c"
    "${Waterlily_SOURCE_DIR}/Source/Public.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan.c"
    "${Waterlily_SOURCE_DIR}/Source/Window.c"
)

foreach(file ${WATERLILY_SOURCES})
    cmake_path(GET file FILENAME FILE_NAME)
    get_source_file_property(CURRENT_CDS ${file} COMPILE_DEFINITIONS)
    set_source_files_properties(${file} PROPERTIES COMPILE_DEFINITIONS 
        "${CURRENT_CDS};FILENAME=\"${FILE_NAME}\"")
endforeach()

#########################################################################################
## Target creation and configuration.
#########################################################################################

add_library("Waterlily" ${WATERLILY_SOURCES})
set_target_properties("Waterlily" PROPERTIES 
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${Waterlily_SOURCE_DIR}/Include/Waterlily.h"
    C_STANDARD 23
    CMAKE_C_STANDARD_REQUIRED ON
    EXPORT_COMPILE_COMMANDS ON
)
target_include_directories("Waterlily" PUBLIC "${Waterlily_SOURCE_DIR}/Include")

#########################################################################################
## Flags set to make sure compilation goes the way we want it to.
#########################################################################################

target_compile_options("Waterlily" PUBLIC -Wall -Wextra -Wpedantic -Werror)
target_link_options("Waterlily" INTERFACE -nostartfiles)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions("Waterlily" PRIVATE BUILD_TYPE=0)

    target_compile_options("Waterlily" PUBLIC -Og -g3 -ggdb -fanalyzer -fsanitize=leak 
        -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract 
        -fsanitize=undefined)
    target_link_options("Waterlily" PUBLIC -fsanitize=address -fsanitize=undefined)
else()
    target_compile_definitions("Waterlily" PRIVATE BUILD_TYPE=1)

    target_compile_options("Waterlily" PUBLIC -march=native -mtune=native -Ofast -flto)
    target_link_options("Waterlily" PUBLIC -Ofast -flto)
endif()

target_compile_definitions("Waterlily" PRIVATE WATERLILY_VERSION="${PROJECT_VERSION}")

#########################################################################################
## Dependency dowsing and linking.
#########################################################################################

find_package(Vulkan REQUIRED COMPONENTS glslang)
target_link_libraries("Waterlily" ${Vulkan_LIBRARIES})
target_include_directories("Waterlily" PUBLIC ${Vulkan_INCLUDE_DIRS})

include(FindPackageHandleStandardArgs)
find_package(PkgConfig REQUIRED)

pkg_check_modules(Wayland_PKG_CONFIG REQUIRED "wayland-client")
find_path(Wayland_INCLUDE_DIRS NAMES "wayland-client.h" HINTS 
    ${Wayland_PKG_CONFIG_INCLUDE_DIRS})
find_library(Wayland_LIBRARIES NAMES "wayland-client" HINTS 
    ${Wayland_PKG_CONFIG_LIBRARY_DIRS})

pkg_check_modules(xkbcommon_PKG_CONFIG REQUIRED "xkbcommon")
find_path(xkbcommon_INCLUDE_PATH NAMES "xkbcommon" HINTS 
    ${xkbcommon_PKG_CONFIG_INCLUDE_DIRS})
find_library(xkbcommon_LIBRARY NAMES "xkbcommon" HINTS 
    ${xkbcommon_PKG_CONFIG_LIBRARY_DIRS})

target_link_libraries("Waterlily" ${Wayland_LIBRARIES} ${xkbcommon_LIBRARY})
target_include_directories("Waterlily" PUBLIC ${Wayland_INCLUDE_DIRS} 
    ${xkbcommon_INCLUDE_PATH})

