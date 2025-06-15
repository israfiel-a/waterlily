cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project("Example" LANGUAGES C VERSION 0.0.0.1)

include(Helpers)
run_checks()
create_target(ON)

add_dependencies(Example RPGtk)
target_link_libraries(Example RPGtk)