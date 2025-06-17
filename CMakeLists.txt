################################################################################
## The configuration files for the RPGtk project. This requires CMake version 
## 3.10 or higher. The following options effect this file's output:
##
## TK_UNDERLYING<ON> - Toggle Wayland (ON) or X11 (OFF) mode.
##
## Copyright (c) 2025 - RPGtk Team
## This source code is under the GPLv3. For information on what that entails,
## please see <https://www.gnu.org/licenses/gpl-3.0.txt>.
################################################################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project("RPGtk" LANGUAGES C VERSION 0.0.0.12)

# We need the CMake helpers in this directory.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Components/tk-utilities")

include(Helpers)
run_checks()

option(TK_UNDERLYING "Toggle Wayland (ON) or X11 (OFF) mode." ON)
option(TK_BUILD_EXAMPLE "Toggle the building of the example application." ON)
option(TK_UPDATE_MODULES "Update/fetch submodules." OFF)

if(TK_UNDERLYING)
    set(TK_UNDERLYING_SYSTEM "Wayland")
else()
    set(TK_UNDERLYING_SYSTEM "X11")
endif() 
string(TOUPPER ${TK_UNDERLYING_SYSTEM} TK_UNDERLYING_SYSTEM_UC)

if(${TK_UPDATE_MODULES})
    message(AUTHOR_WARNING "Updating submodules.")
    find_package(Git REQUIRED)
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

create_target(OFF)

add_subdirectory("${CMAKE_SOURCE_DIR}/Components/tk-assets")
add_subdirectory("${CMAKE_SOURCE_DIR}/Components/tk-vulkan")
add_subdirectory("${CMAKE_SOURCE_DIR}/Components/tk-window")
add_dependencies(${PROJECT_NAME} TKVulkan TKWindow)
target_link_libraries(${PROJECT_NAME} TKVulkan TKWindow)

if(${TK_BUILD_EXAMPLE})
    add_subdirectory("${CMAKE_SOURCE_DIR}/Example")
endif()
