################################################################################
## The configuration files for the Waterlily project. This requires CMake 
## version 3.10 or higher. The following options effect this file's output:
##
## WATERLILY_UNDERLYING<ON> - Toggle Wayland (ON) or X11 (OFF) mode.
##
## Copyright (c) 2025 - the Waterlily Team
## This source code is under the GPLv3. For information on what that entails,
## please see <https://www.gnu.org/licenses/gpl-3.0.txt>.
################################################################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project("Waterlily" LANGUAGES C VERSION 0.0.0.24)

option(WATERLILY_UNDERLYING "Toggle Wayland (ON) or X11 (OFF) mode." ON)
option(WATERLILY_BUILD_EXAMPLE "Toggle the building of the example application." ON)
option(WATERLILY_UPDATE_MODULES "Update/fetch submodules." OFF)
if(${WATERLILY_UPDATE_MODULES})
    message(AUTHOR_WARNING "Updating submodules.")
    find_package(Git REQUIRED)
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(BUILD_TYPE=0)
else()
    add_compile_definitions(BUILD_TYPE=1)
endif()

# We need the CMake helpers in this directory.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Utilities")

include(Helpers)
run_checks()

if(WATERLILY_UNDERLYING)
    set(WATERLILY_UNDERLYING_SYSTEM "Wayland")
else()
    set(WATERLILY_UNDERLYING_SYSTEM "X11")
endif() 
string(TOUPPER ${WATERLILY_UNDERLYING_SYSTEM} WATERLILY_UNDERLYING_SYSTEM_UC)

create_target(OFF RAW_SOURCES ${PROJECT_NAME})

add_subdirectory("${CMAKE_SOURCE_DIR}/Logging")
add_subdirectory("${CMAKE_SOURCE_DIR}/Geranium")
add_subdirectory("${CMAKE_SOURCE_DIR}/Hyacinth")
add_dependencies(${PROJECT_NAME} Geranium Hyacinth)
target_link_libraries(${PROJECT_NAME} Geranium Hyacinth)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(${PROJECT_NAME} WLLogging)
endif()

if(${WATERLILY_BUILD_EXAMPLE})
    add_subdirectory("${CMAKE_SOURCE_DIR}/Example")
endif()
