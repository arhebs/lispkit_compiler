cmake_minimum_required(VERSION 3.26)
project(lispkit_compiler)

set(PARSER_DIR "${CMAKE_CURRENT_BINARY_DIR}")

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(LEXER_OUT "${PARSER_DIR}/lexer.c")
set(PARSER_OUT "${PARSER_DIR}/parser.c")

include_directories("${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")

bison_target(PARSER parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
flex_target(LEXER lexer.l "${LEXER_OUT}" DEFINES_FILE "${PARSER_DIR}/lexer.h")
ADD_FLEX_BISON_DEPENDENCY(LEXER PARSER)

add_executable(lispkit_compiler main.cc "${LEXER_OUT}" "${PARSER_OUT}")
