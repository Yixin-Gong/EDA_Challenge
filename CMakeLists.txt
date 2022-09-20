set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.19)

project(EDA_Challenge CXX)
set(CMAKE_CXX_STANDARD 14)

add_compile_options(-O2 -g)

include_directories(system)
file(GLOB_RECURSE SOURCES "main.cc" "system/*.cc")

add_custom_target(version_h BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/../system/version.h"
        COMMAND ${CMAKE_COMMAND} -D SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}"
        -P ${CMAKE_CURRENT_SOURCE_DIR}/gitversion.cmake
        COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/system/system.cc)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} m)
add_dependencies(${PROJECT_NAME} version_h)
