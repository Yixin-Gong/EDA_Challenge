set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.19)

project(EDA_Challenge CXX)
set(CMAKE_CXX_STANDARD 14)

add_compile_options(-O2 -g)

include_directories(system gtest/include gui .)
file(GLOB_RECURSE SOURCES "main.cc" "system/*.cc" "gui/*.cc")
file(GLOB_RECURSE TEST_SOURCES "testbench/*.cc")

add_custom_target(version_h BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/../system/version.h"
        COMMAND ${CMAKE_COMMAND} -D SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}"
        -P ${CMAKE_CURRENT_SOURCE_DIR}/gitversion.cmake
        COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/gui && glib-compile-resources gresource.xml --target=resources.cc --generate-source
        COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/system/system.cc)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

include_directories(${GTK4_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS})

add_executable(${PROJECT_NAME} ${SOURCES})
add_executable(Google_Test ${SOURCES} ${TEST_SOURCES})

find_library(GTEST gtest ${CMAKE_SOURCE_DIR}/gtest)
find_library(GTEST_MAIN gtest_main ${CMAKE_SOURCE_DIR}/gtest)

target_link_libraries(Google_Test ${GTEST} ${GTEST_MAIN} ${GTK4_LIBRARIES} m)
target_link_libraries(${PROJECT_NAME} ${GTK4_LIBRARIES} m)
add_dependencies(${PROJECT_NAME} version_h)

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "IS_NOT_RUNNING_GOOGLE_TEST")
set_target_properties(version_h PROPERTIES COMPILE_DEFINITIONS "IS_NOT_RUNNING_GOOGLE_TEST")
