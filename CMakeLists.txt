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

FIND_PACKAGE(Git)

IF(GIT_FOUND)
    SET(USE_GIT 1)
ELSE(GIT_FOUND)
    SET(USE_GIT 0)
ENDIF(GIT_FOUND)

FILE(WRITE ${CMAKE_BINARY_DIR}/Version.cmake
    "
    IF(${USE_GIT} EQUAL 1)
        EXECUTE_PROCESS(
            COMMAND ${GIT_EXECUTABLE} describe --tags --dirty
            WORKING_DIRECTORY \${WORKING_DIRECTORY}
            OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
            RESULT_VARIABLE GIT_DESCRIBE_RESULT
            ERROR_VARIABLE GIT_DESCRIBE_ERROR
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    ELSE(${USE_GIT} EQUAL 1)
        SET(GIT_DESCRIBE_RESULT -1)
    ENDIF(${USE_GIT} EQUAL 1)

    IF(GIT_DESCRIBE_RESULT EQUAL 0)
        FILE(WRITE ${CMAKE_BINARY_DIR}/source/VERSION \"\${GIT_DESCRIBE_VERSION}\")
    ELSE(GIT_DESCRIBE_RESULT EQUAL 0)
        FILE(READ  ${CMAKE_SOURCE_DIR}/VERSION GIT_DESCRIBE_VERSION)
    ENDIF(GIT_DESCRIBE_RESULT EQUAL 0)

    STRING(REGEX REPLACE \"v([0-9]*)\\\\.([0-9]*)\\\\.(.*)\"
        \"\\\\1.\\\\2.\\\\3\" VERSION \"\${GIT_DESCRIBE_VERSION}\" )
    STRING(REGEX REPLACE \"v([0-9]*)\\\\.([0-9]*)\\\\.(.*)\"
        \"\\\\1\" VERSION_MAJOR \"\${GIT_DESCRIBE_VERSION}\")
    STRING(REGEX REPLACE \"v([0-9]*)\\\\.([0-9]*)\\\\.(.*)\"
        \"\\\\2\" VERSION_MINOR \"\${GIT_DESCRIBE_VERSION}\")
    STRING(REGEX REPLACE \"v([0-9]*)\\\\.([0-9]*)\\\\.(.*)\"
        \"\\\\3\" VERSION_PATCH \"\${GIT_DESCRIBE_VERSION}\")
    "
)

FILE(WRITE ${CMAKE_BINARY_DIR}/version.h.in
    "
    \#define VERSION \"@VERSION@\"\n
    "
)

FILE(WRITE ${CMAKE_BINARY_DIR}/FillVersion.cmake
    "
    INCLUDE(Version.cmake)
    CONFIGURE_FILE(\${SRC} \${DST} @ONLY)
    "
)

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
ADD_CUSTOM_TARGET(
    version
    ${CMAKE_COMMAND} -D SRC=${CMAKE_BINARY_DIR}/version.h.in
                     -D DST=${CMAKE_BINARY_DIR}/version.h
                     -D WORKING_DIRECTORY=${CMAKE_SOURCE_DIR}
                     -P ${CMAKE_BINARY_DIR}/FillVersion.cmake
)

## Configure and include CPack
SET(WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
INCLUDE(${CMAKE_BINARY_DIR}/Version.cmake)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY  "${PROJECT_NAME} project")
SET(CPACK_PACKAGE_VERSION "${VERSION}")
SET(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "VERSIONHEADER-${VERSIONHEADER_VERSION_FULL}")
set(CPACK_GENERATOR "TBZ2")
set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_PACKAGE_RELOCATABLE "true")
set(CPACK_SOURCE_INSTALLED_DIRECTORIES
    "${CMAKE_SOURCE_DIR}/src;./src;"
    "${CMAKE_SOURCE_DIR}/include;./include;"
    "${CMAKE_BINARY_DIR}/source;./source;"
)
set(CPACK_SOURCE_IGNORE_FILES
    "~$"
    ".sw[a-z]$"
    ".DS_Store"
    "tags"
    "cscope.*"
    ".vscode/"
    "build/"
    "^${CMAKE_SOURCE_DIR}/.git.*"
    "${CPACK_SOURCE_IGNORE_FILES}"
)

INCLUDE(CPack)

IF (UNIX)
    FILE(WRITE ${CMAKE_BINARY_DIR}/Dist.cmake
        "
        MESSAGE(STATUS \"Package Dist running...\")
        INCLUDE(CPackSourceConfig.cmake)
        EXECUTE_PROCESS(COMMAND cpack -G TGZ --config CPackSourceConfig.cmake
        TIMEOUT 3600
        WORKING_DIRECTORY \${CMAKE_BINARY_DIR})
        "
    )

    ## Add custom target
    ADD_CUSTOM_TARGET(dist
        COMMAND ${CMAKE_COMMAND} .
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/Dist.cmake
    )
ENDIF(UNIX)

ADD_DEPENDENCIES(dist version)

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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB LIB_LIMPID_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.c")

ADD_LIBRARY(${LIB_LIMPID_NAME} STATIC ${LIB_LIMPID_SRC})

## Install binary and other related assets
set(CMAKE_INSTALL_DIR $ENV{INSTALL_PREFIX}/${VERSIONHEADER_VERSION_FULL})
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
