cmake_minimum_required(VERSION 3.16)
project(HttpServer VERSION 0.0.1 LANGUAGES C CXX)

include(cmake/build_option.cmake)
include(cmake/compile_setting.cmake)

add_subdirectory(http-server)
add_subdirectory(test)

include(cmake/install.cmake)
include(cmake/cpack.cmake)
