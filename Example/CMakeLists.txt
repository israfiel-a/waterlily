cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project("Example" LANGUAGES C VERSION 0.0.0.6)

include(Helpers)
run_checks(ON)
create_target(ON RAW_SOURCES "Example")

add_dependencies(Example Waterlily)
target_link_libraries(Example Waterlily)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/Resources" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")