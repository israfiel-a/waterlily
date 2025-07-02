################################################################################
## The configuration files for the Waterlily project. This requires CMake 
## version 3.10 or higher. The following options effect this file's output:
##
## WATERLILY_WAYLAND<ON> - Toggle Wayland (ON) or X11 (OFF) mode.
##
## Copyright (c) 2025 - the Waterlily Team
## This source code is under the GPLv3. For information on what that entails,
## please see <https://www.gnu.org/licenses/gpl-3.0.txt>.
################################################################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project("Waterlily" LANGUAGES C VERSION 0.0.0.30)

if(NOT DEFINED CMAKE_BUILD_TYPE)
    message(WARNING "No build type given, assuming debug.")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(NOT UNIX OR APPLE)
    message(FATAL_ERROR "This operating system is not supported by " 
        "${PROJECT_NAME}.")
endif()

option(WATERLILY_WAYLAND "Toggle Wayland (ON) or X11 (OFF) mode." ON)
option(WATERLILY_EXAMPLE "Toggle the example application." ON)
option(WATERLILY_UPDATE_MODULES "Update/fetch submodules." OFF)

if(WATERLILY_WAYLAND)
    add_compile_definitions(WAYLAND)
else()
    add_compile_definitions(X11)
endif()

if(WATERLILY_EXAMPLE OR WATERLILY_UPDATE_MODULES)
    find_package(Git REQUIRED)
endif()

if(WATERLILY_UPDATE_MODULES)
    message(WARNING "Updating submodules.")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init 
        --recursive WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

set(WATERLILY_SOURCES "${CMAKE_SOURCE_DIR}/Waterlily.c")
include_directories("${CMAKE_SOURCE_DIR}")

set(WATERLILY_SUBPROJECTS "Ageratum" "Geranium" "Hyacinth" "Primrose")
foreach(subproject ${WATERLILY_SUBPROJECTS})
    set(PROJECT_DIRECTORY "${CMAKE_SOURCE_DIR}/${subproject}")
    include_directories(${PROJECT_DIRECTORY})

    if(IS_DIRECTORY "${PROJECT_DIRECTORY}/Source")
        file(GLOB PROJECT_SOURCES "${PROJECT_DIRECTORY}/Source/*.c")
        list(APPEND WATERLILY_SOURCES ${PROJECT_SOURCES})
    endif()

    if(IS_DIRECTORY "${PROJECT_DIRECTORY}/Targets")
        if(WATERLILY_WAYLAND)
            list(APPEND WATERLILY_SOURCES "${PROJECT_DIRECTORY}/Targets/Wayland.c")
        else()
            list(APPEND WATERLILY_SOURCES "${PROJECT_DIRECTORY}/Targets/X11.c")
        endif()
    endif()
endforeach()

foreach(file ${WATERLILY_SOURCES})
    cmake_path(GET file FILENAME FILE_NAME)
    get_source_file_property(CURRENT_CDS ${file} COMPILE_DEFINITIONS)
    set_source_files_properties(${file} PROPERTIES COMPILE_DEFINITIONS "${CURRENT_CDS};FILENAME=\"${FILE_NAME}\"")
endforeach()

if(WATERLILY_WAYLAND)
    include(FindPackageHandleStandardArgs)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(Wayland_PKG_CONFIG QUIET "wayland-client")
    find_path(Wayland_INCLUDE_DIRS NAMES "wayland-client.h" HINTS ${Wayland_PKG_CONFIG_INCLUDE_DIRS})
    find_library(Wayland_LIBRARIES NAMES "wayland-client" HINTS ${Wayland_PKG_CONFIG_LIBRARY_DIRS})

    link_libraries(${Wayland_LIBRARIES})
    include_directories(${Wayland_INCLUDE_DIRS})
endif()

find_package(Vulkan REQUIRED COMPONENTS glslang)
link_libraries(${Vulkan_LIBRARIES})
include_directories(${Vulkan_INCLUDE_DIRS})

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Werror -Wpedantic -Wextra -Wno-gnu-zero-variadic-macro-arguments)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(BUILD_TYPE=0)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    add_compile_options(-Og -g3 -ggdb -fsanitize=address -fsanitize=pointer-compare  -fsanitize=leak -fsanitize=pointer-subtract -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fanalyzer)
    endif()
else()
    add_compile_definitions(BUILD_TYPE=1)
    add_compile_options(-march=native -mtune=native -Ofast -flto)
    add_link_options(-Ofast -flto)
endif()

add_library(${PROJECT_NAME} ${WATERLILY_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES 
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.h"
)

if(WATERLILY_EXAMPLE)
    include(FetchContent)
    FetchContent_Declare(
        "Example"
        GIT_REPOSITORY "https://github.com/waterlily-team/example.git"
        GIT_TAG "origin/main"
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/Example"
    )
    FetchContent_MakeAvailable(Example)
endif()
