#
#                 Copyright (c) 2019 Siddharth Chandrasekaran
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#   Author : Siddharth Chandrasekaran
#   Email  : siddharth@embedjournal.com
#   Date   : Wed Mar 27 15:57:22 IST 2019
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

project(limpid)
set(PROJECT_AUTHOR "Siddharth Chandrasekran")
set(PROJECT_YEAR 2017)

set(LIB_LIMPID_NAME "limpid")

## POSIX Thread Library
find_package(Threads REQUIRED)

## Make sure we can import our CMake functions
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

## Include helpers
include(AddCCompilerFlag)
include(AddCXXCompilerFlag)
include(DetermineTargetArchitecture)

set(CMAKE_BUILD_TYPE Release)
add_c_compiler_flag(-Wall)
add_c_compiler_flag(-s) # strip target

determine_target_architecture(ARCH)

include_directories($ENV{INSTALL_PREFIX}/include)
link_directories($ENV{INSTALL_PREFIX}/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB LIB_LIMPID_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.c")
file(GLOB LIB_LIMPID_HELPERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "helpers/*.c")

ADD_LIBRARY(${LIB_LIMPID_NAME} STATIC ${LIB_LIMPID_SRC} ${LIB_LIMPID_HELPERS})

## Install binary and other related assets
set(CMAKE_INSTALL_DIR $ENV{INSTALL_PREFIX})
install(TARGETS ${LIB_LIMPID_NAME} DESTINATION ${CMAKE_INSTALL_DIR}/lib)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/limpid DESTINATION ${CMAKE_INSTALL_DIR}/include)

## Example CLI Target
add_executable(cli_app ${CMAKE_CURRENT_SOURCE_DIR}/examples/cli/sample_app.c)
target_link_libraries(cli_app PUBLIC limpid)
target_link_libraries(cli_app PUBLIC ${CMAKE_THREAD_LIBS_INIT})

add_executable(cli_prompt ${CMAKE_CURRENT_SOURCE_DIR}/examples/cli/client.c)
target_link_libraries(cli_prompt PUBLIC limpid)
target_link_libraries(cli_prompt PUBLIC ${CMAKE_THREAD_LIBS_INIT})

## Example JSON Target
add_executable(json_app ${CMAKE_CURRENT_SOURCE_DIR}/examples/json/sample_app.c)
target_link_libraries(json_app PUBLIC limpid)
target_link_libraries(json_app PUBLIC ${CMAKE_THREAD_LIBS_INIT})

add_executable(json_prompt ${CMAKE_CURRENT_SOURCE_DIR}/examples/json/client.c)
target_link_libraries(json_prompt PUBLIC limpid)
target_link_libraries(json_prompt PUBLIC ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(cli_app cli_prompt json_app json_prompt
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
