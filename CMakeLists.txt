set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.19)

project(EDA_Challenge CXX)
set(CMAKE_CXX_STANDARD 14)

add_compile_options(-Ofast)

if (NOT COMPILE_ON_SERVER)
    include_directories(system gtest/include gui . parser)
    file(GLOB_RECURSE SOURCES "main.cc" "system/*.cc" "gui/*.cc" "parser/*.cc")
    file(GLOB_RECURSE TEST_SOURCES "test/*.cc")

    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

    add_custom_target(version_h BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/../system/version.h"
            COMMAND ${CMAKE_COMMAND} -D SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}"
            -P ${CMAKE_CURRENT_SOURCE_DIR}/gitversion.cmake
            COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/gui && glib-compile-resources gresource.xml --target=resources.cc --generate-source
            COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/system/system.cc)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
    pkg_check_modules(GTKMM_PLPLOT REQUIRED gtkmm-plplot-2.0)

    add_compile_definitions(GTK_LIBRARY_VERSION="${GTK3_VERSION}" GTKMM_LIBRARY_VERSION="${GTKMM3_VERSION}")
    add_compile_definitions(CXX_COMPILER_VERSION="${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

    include_directories(${GTK3_INCLUDE_DIRS} ${GTKMM3_INCLUDE_DIRS} ${GTKMM_PLPLOT_INCLUDE_DIRS})
    link_directories(${GTK3_LIBRARY_DIRS} ${GTKMM3_LIBRARY_DIRS} ${GTKMM_PLPLOT_LIBRARY_DIRS})

    add_executable(${PROJECT_NAME} ${SOURCES})
    add_executable(Google_Test ${SOURCES} ${TEST_SOURCES})

    find_library(GTEST gtest ${CMAKE_SOURCE_DIR}/gtest)
    find_library(GTEST_MAIN gtest_main ${CMAKE_SOURCE_DIR}/gtest)

    target_link_libraries(Google_Test ${GTEST} ${GTEST_MAIN} ${GTK3_LIBRARIES} ${GTKMM3_LIBRARIES} ${GTKMM_PLPLOT_LIBRARIES} m pthread)
    target_link_libraries(${PROJECT_NAME} ${GTK3_LIBRARIES} ${GTKMM3_LIBRARIES} ${GTKMM_PLPLOT_LIBRARIES} m pthread)
    add_dependencies(${PROJECT_NAME} version_h)

    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "IS_NOT_RUNNING_GOOGLE_TEST")
    set_target_properties(version_h PROPERTIES COMPILE_DEFINITIONS "IS_NOT_RUNNING_GOOGLE_TEST")
else ()
    include_directories(system . parser)
    file(GLOB_RECURSE SOURCES "main.cc" "system/*.cc" "parser/*.cc")

    add_compile_definitions(GTK_LIBRARY_VERSION="No gtk library" GTKMM_LIBRARY_VERSION="No gtkmm library")
    add_compile_definitions(CXX_COMPILER_VERSION="${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

    add_custom_target(version_h BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/../system/version.h"
            COMMAND ${CMAKE_COMMAND} -D SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}"
            -P ${CMAKE_CURRENT_SOURCE_DIR}/gitversion.cmake
            COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/system/system.cc)

    add_executable(${PROJECT_NAME} ${SOURCES})
    target_link_libraries(${PROJECT_NAME} m pthread)

    add_dependencies(${PROJECT_NAME} version_h)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "IS_NOT_RUNNING_GUI")
endif ()
