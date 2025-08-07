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
## Source file wrangling and processing.
#########################################################################################

set(WATERLILY_SOURCES 
    "${Waterlily_SOURCE_DIR}/Source/Engine/Digest.c"
    "${Waterlily_SOURCE_DIR}/Source/Engine/Log.c"
    "${Waterlily_SOURCE_DIR}/Source/Engine/Setup.c"
    "${Waterlily_SOURCE_DIR}/Source/Files/Execute.c"
    "${Waterlily_SOURCE_DIR}/Source/Files/Measure.c"
    "${Waterlily_SOURCE_DIR}/Source/Files/Open.c"
    "${Waterlily_SOURCE_DIR}/Source/Files/Read.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Create.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Destroy.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Render.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Sync.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Commands/BeginRenderpass.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Commands/CreateBuffers.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Commands/CreateSyncs.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Commands/RecordBuffer.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/GPU/GetPhysical.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/GPU/CreateLogical.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/GPU/Destroy.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Pipeline/Create.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Pipeline/CreateLayout.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Pipeline/CreateRenderpass.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Pipeline/FillInPipeline.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Pipeline/SetupShaders.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Surface/GetCapabilities.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Surface/GetExtent.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Surface/GetFormat.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Surface/GetMode.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Swapchain/Create.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Swapchain/CreateFramebuffers.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Swapchain/Destroy.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Swapchain/Partition.c"
    "${Waterlily_SOURCE_DIR}/Source/Vulkan/Swapchain/Recreate.c"
)

if(WATERLILY_WAYLAND)
    list(APPEND WATERLILY_SOURCES 
        "${Waterlily_SOURCE_DIR}/Source/Window/Wayland.c"
        "${Waterlily_SOURCE_DIR}/Source/Vulkan/Surface/Wayland.c"
    )
else()
    list(APPEND WATERLILY_SOURCES 
        "${Waterlily_SOURCE_DIR}/Source/Window/X11.c"
        "${Waterlily_SOURCE_DIR}/Source/Vulkan/Surface/X11.c"
    )
endif()

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

if(WATERLILY_WAYLAND)
    include(FindPackageHandleStandardArgs)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(Wayland_PKG_CONFIG QUIET "wayland-client")
    find_path(Wayland_INCLUDE_DIRS NAMES "wayland-client.h" HINTS 
        ${Wayland_PKG_CONFIG_INCLUDE_DIRS})
    find_library(Wayland_LIBRARIES NAMES "wayland-client" HINTS 
        ${Wayland_PKG_CONFIG_LIBRARY_DIRS})

    target_link_libraries("Waterlily" ${Wayland_LIBRARIES})
    target_include_directories("Waterlily" PUBLIC ${Wayland_INCLUDE_DIRS})
else()
    find_package(X11 REQUIRED)
    target_link_libraries("Waterlily" ${X11_LIBRARIES})
    target_include_directories("Waterlily" PUBLIC ${X11_INCLUDE_DIRS}) 
endif()

