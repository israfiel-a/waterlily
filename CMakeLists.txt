#########################################################################################
## This is the CMake build file for the Waterlily engine. It handles all build 
## configuration mechanisms, like selecting a windowing backend or turning on/off flags.
##
## WATERLILY_WAYLAND<ON> - Toggle Wayland (ON) or X11 (OFF) mode.
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
## Options that the user may flip on or off to customize the compilation.
#########################################################################################

option(WATERLILY_WAYLAND "Toggle Wayland (ON) or X11 (OFF) mode." ON)

#########################################################################################
## Flags set to make sure compilation goes the way we want it to.
#########################################################################################

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(BUILD_TYPE=0)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    add_compile_options(-Og -g3 -ggdb -fanalyzer -fsanitize=address -fsanitize=leak 
        -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
else()
    add_compile_definitions(BUILD_TYPE=1)

    add_compile_options(-march=native -mtune=native -Ofast -flto)
    add_link_options(-Ofast -flto)
endif()

if(WATERLILY_WAYLAND)
    add_compile_definitions(WAYLAND)
else()
    add_compile_definitions(X11)
endif()

add_compile_definitions(WATERLILY_VERSION="${PROJECT_VERSION}")

#########################################################################################
## Source file wrangling and processing.
#########################################################################################

set(WATERLILY_SOURCES 
    "${CMAKE_SOURCE_DIR}/Source/Engine/Digest.c"
    "${CMAKE_SOURCE_DIR}/Source/Engine/Log.c"
    "${CMAKE_SOURCE_DIR}/Source/Engine/Setup.c"
    "${CMAKE_SOURCE_DIR}/Source/Vulkan/Create.c"
    "${CMAKE_SOURCE_DIR}/Source/Vulkan/Destroy.c"
    "${CMAKE_SOURCE_DIR}/Source/Vulkan/GPU/GetPhysical.c"
    "${CMAKE_SOURCE_DIR}/Source/Vulkan/GPU/CreateLogical.c"
    "${CMAKE_SOURCE_DIR}/Source/Vulkan/Surface/GetCapabilities.c"
    "${CMAKE_SOURCE_DIR}/Source/Vulkan/Surface/GetExtent.c"
    "${CMAKE_SOURCE_DIR}/Source/Vulkan/Surface/GetFormat.c"
    "${CMAKE_SOURCE_DIR}/Source/Vulkan/Surface/GetMode.c"
)

if(WATERLILY_WAYLAND)
    list(APPEND WATERLILY_SOURCES 
        "${CMAKE_SOURCE_DIR}/Source/Window/Wayland.c"
        "${CMAKE_SOURCE_DIR}/Source/Vulkan/Surface/Wayland.c"
    )
else()
    list(APPEND WATERLILY_SOURCES 
        "${CMAKE_SOURCE_DIR}/Source/Window/X11.c"
        "${CMAKE_SOURCE_DIR}/Source/Vulkan/Surface/X11.c"
    )
endif()

foreach(file ${WATERLILY_SOURCES})
    cmake_path(GET file FILENAME FILE_NAME)
    get_source_file_property(CURRENT_CDS ${file} COMPILE_DEFINITIONS)
    set_source_files_properties(${file} PROPERTIES COMPILE_DEFINITIONS 
        "${CURRENT_CDS};FILENAME=\"${FILE_NAME}\"")
endforeach()

#########################################################################################
## Dependency dowsing and linking.
#########################################################################################

find_package(Vulkan REQUIRED COMPONENTS glslang)
link_libraries(${Vulkan_LIBRARIES})
include_directories(${Vulkan_INCLUDE_DIRS})

if(WATERLILY_WAYLAND)
    include(FindPackageHandleStandardArgs)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(Wayland_PKG_CONFIG QUIET "wayland-client")
    find_path(Wayland_INCLUDE_DIRS NAMES "wayland-client.h" HINTS 
        ${Wayland_PKG_CONFIG_INCLUDE_DIRS})
    find_library(Wayland_LIBRARIES NAMES "wayland-client" HINTS 
        ${Wayland_PKG_CONFIG_LIBRARY_DIRS})

    link_libraries(${Wayland_LIBRARIES})
    include_directories(${Wayland_INCLUDE_DIRS})
else()
    find_package(X11 REQUIRED)
    link_libraries(${X11_LIBRARIES})
    include_directories(${X11_INCLUDE_DIRS}) 
endif()

#########################################################################################
## Target creation and configuration.
#########################################################################################

add_library("Waterlily" ${WATERLILY_SOURCES})
set_target_properties("Waterlily" PROPERTIES 
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/Include/Waterlily.h"
)
target_include_directories("Waterlily" PUBLIC "${CMAKE_SOURCE_DIR}/Include")

