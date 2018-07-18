# Copyright 2017 The Dawn Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

find_package(PythonInterp REQUIRED)
include(CMakeParseArguments)

# Check for Jinja2
message(STATUS "${PYTHON_EXECUTABLE}")
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "import jinja2"
    RESULT_VARIABLE RET
)
if (NOT RET EQUAL 0)
    message(FATAL_ERROR "Missing dependencies for code generation, please ensure you have python-jinja2 installed.")
endif()

function(Generate)
    set(oneValueArgs LIB_NAME LIB_TYPE PRINT_NAME EXECUTABLE FOLDER)
    set(multiValueArgs COMMAND_LINE_ARGS EXTRA_DEPS EXTRA_SOURCES)
    cmake_parse_arguments(G "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    execute_process(
        COMMAND ${G_COMMAND_LINE_ARGS} --print-dependencies
        OUTPUT_VARIABLE DEPENDENCIES
        RESULT_VARIABLE RET
    )
    if (NOT RET EQUAL 0)
        message(STATUS ${RET})
        message(FATAL_ERROR "Failed to get the dependencies for ${G_PRINT_NAME}.")
    endif()

    execute_process(
        COMMAND ${G_COMMAND_LINE_ARGS} --print-outputs
        OUTPUT_VARIABLE OUTPUTS
        RESULT_VARIABLE RET
    )
    if (NOT RET EQUAL 0)
        message(FATAL_ERROR "Failed to get the outputs for ${G_PRINT_NAME}.")
    endif()

    add_custom_command(
        COMMAND ${G_COMMAND_LINE_ARGS}
        DEPENDS ${DEPENDENCIES} ${G_EXTRA_DEPS}
        OUTPUT ${OUTPUTS}
        COMMENT "Generating files for ${G_PRINT_NAME}."
    )

    add_library(${G_LIB_NAME} ${G_LIB_TYPE}
        ${G_EXTRA_SOURCES}
        ${OUTPUTS}
    )

    NXTInternalTarget("${G_FOLDER}" ${G_LIB_NAME})
    set_property(TARGET ${G_LIB_NAME} APPEND PROPERTY COMPILE_OPTIONS ${NXT_GENERATED_FLAGS})
endfunction()

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

set(GENERATOR_COMMON_ARGS
    ${PYTHON_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.py
    ${PROJECT_SOURCE_DIR}/dawn.json
    -t ${CMAKE_CURRENT_SOURCE_DIR}/templates
    -o ${CMAKE_CURRENT_BINARY_DIR}
    PARENT_SCOPE
)
