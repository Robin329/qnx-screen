
cmake_minimum_required(VERSION 3.10)

project(qnx-stream-api-demo)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

find_package(screen REQUIRED)

add_executable(cons consumer/main.cpp)
target_link_libraries(cons screen::screen EGL GLESv2)
set_target_properties(cons PROPERTIES CXX_STANDARD 17)

add_executable(prod producer/main.cpp)
target_link_libraries(prod screen::screen EGL GLESv2)
set_target_properties(prod PROPERTIES CXX_STANDARD 17)

