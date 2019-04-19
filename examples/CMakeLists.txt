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

find_package(Threads REQUIRED)
find_package(LibDl REQUIRED)
find_package(LibReadline REQUIRED)

set(CMAKE_BUILD_TYPE Release)
add_c_compiler_flag(-Wall)
add_c_compiler_flag(-s) # strip target

include_directories($ENV{INSTALL_PREFIX}/include)
link_directories($ENV{INSTALL_PREFIX}/lib)

## Example CLI Target
add_executable(cli_app ${CMAKE_CURRENT_SOURCE_DIR}/cli/sample_app.c)
#target_link_libraries(cli_app PRIVATE -ldl)
target_link_libraries(cli_app PRIVATE -lreadline)
target_link_libraries(cli_app PUBLIC limpid)
target_link_libraries(cli_app PUBLIC ${CMAKE_THREAD_LIBS_INIT})

add_executable(cli_prompt ${CMAKE_CURRENT_SOURCE_DIR}/cli/client.c)
#target_link_libraries(cli_prompt PRIVATE -ldl)
target_link_libraries(cli_prompt PRIVATE -lreadline)
target_link_libraries(cli_prompt PUBLIC limpid)
target_link_libraries(cli_prompt PUBLIC ${CMAKE_THREAD_LIBS_INIT})

## Example JSON Target
add_executable(json_app ${CMAKE_CURRENT_SOURCE_DIR}/json/sample_app.c)
target_link_libraries(json_app PRIVATE -lreadline)
target_link_libraries(json_app PUBLIC limpid)
target_link_libraries(json_app PUBLIC ${CMAKE_THREAD_LIBS_INIT})

add_executable(json_prompt ${CMAKE_CURRENT_SOURCE_DIR}/json/client.c)
target_link_libraries(json_prompt PRIVATE -lreadline)
target_link_libraries(json_prompt PUBLIC limpid)
target_link_libraries(json_prompt PUBLIC ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(cli_app cli_prompt json_app json_prompt
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
