cmake_minimum_required(VERSION 3.26)

set(VCPKG_ROOT /opt/vcpkg)
set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

project(lispkit_compiler)

set(CMAKE_CXX_STANDARD 20)

set(PARSER_DIR "${CMAKE_CURRENT_BINARY_DIR}")

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(argparse CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM_LIB REQUIRED IMPORTED_TARGET gtkmm-4.0)

set(SRC_FILES interpreter.cpp main_window.cpp
        AST.cpp)

set(LEXER_OUT "${PARSER_DIR}/lexer.cpp")
set(PARSER_OUT "${PARSER_DIR}/parser.cpp")

include_directories("${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")

bison_target(PARSER parser.y "${PARSER_OUT}")
flex_target(LEXER lexer.l "${LEXER_OUT}" DEFINES_FILE "${PARSER_DIR}/lexer.hpp")
ADD_FLEX_BISON_DEPENDENCY(LEXER PARSER)

add_executable(lispkit_compiler
        main.cc
        "${SRC_FILES}"
        "${LEXER_OUT}"
        "${PARSER_OUT}"
)

target_link_libraries(lispkit_compiler PRIVATE
        argparse::argparse
        PkgConfig::GTKMM_LIB
)
