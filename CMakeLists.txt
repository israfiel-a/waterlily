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
project("RPGtk" LANGUAGES C VERSION 0.0.0.2)

# We need the CMake helpers in this directory.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Components/tk-utilities")

option(TK_UNDERLYING "Toggle Wayland (ON) or X11 (OFF) mode." ON)

include(Helpers)
run_checks()
create_target()

add_subdirectory("${CMAKE_SOURCE_DIR}/Components/tk-vulkan")
add_subdirectory("${CMAKE_SOURCE_DIR}/Components/tk-window")