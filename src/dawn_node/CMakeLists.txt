# Copyright 2021 The Dawn Authors
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

set(GEN_DIR         "${CMAKE_CURRENT_BINARY_DIR}/gen")
set(IDLGEN_TOOL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tools/src/cmd/idlgen")

# idlgen() is a function that uses the tools/cmd/idlgen/main.go tool to generate
# code from an IDL file and template.
# idlgen() accepts the following named arguments:
#   TEMPLATE <path> - (required) the path to the root .tmpl file. If the
#                     template imports other templates, then these should be
#                     added to the DEPENDS argument list.
#   OUTPUT <path>   - (required) the output file path.
#   IDLS <paths>    - (at least one required) the list of input WebIDL files.
#   DEPENDS <paths> - an optional list of additional file dependencies used.
function(idlgen)
    cmake_parse_arguments(IDLGEN
        ""                # options
        "TEMPLATE;OUTPUT" # one_value_keywords
        "IDLS;DEPENDS"    # multi_value_keywords
        ${ARGN})

    if(NOT IDLGEN_TEMPLATE)
        message(FATAL_ERROR "idlgen() missing TEMPLATE argument")
    endif()
    if(NOT IDLGEN_OUTPUT)
        message(FATAL_ERROR "idlgen() missing OUTPUT argument")
    endif()
    if(NOT IDLGEN_IDLS)
        message(FATAL_ERROR "idlgen() missing IDLS argument(s)")
    endif()
    add_custom_command(
        COMMAND "go" "run" "main.go"
                "--template" "${IDLGEN_TEMPLATE}"
                "--output"   "${IDLGEN_OUTPUT}"
                ${IDLGEN_IDLS}
        DEPENDS "${IDLGEN_TOOL_DIR}/main.go"
                ${IDLGEN_TEMPLATE}
                ${IDLGEN_DEPENDS}
                ${IDLGEN_IDLS}
        OUTPUT  ${IDLGEN_OUTPUT}
        WORKING_DIRECTORY ${IDLGEN_TOOL_DIR}
        COMMENT "Generating ${IDLGEN_OUTPUT}"
    )
endfunction()

add_subdirectory(binding)
add_subdirectory(interop)

add_library(dawn_node SHARED "Module.cpp")
set_target_properties(dawn_node PROPERTIES
    PREFIX ""
    OUTPUT_NAME "dawn"
    SUFFIX ".node"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
)
target_link_libraries(dawn_node dawn_node_binding dawn_node_interop dawn_native dawncpp dawn_proc)
target_include_directories(dawn_node PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${NODE_API_HEADERS_DIR}
    ${NODE_ADDON_API_DIR}
    ${GEN_DIR}
)

# dawn_node targets require C++17
set_property(
    TARGET dawn_node
    PROPERTY CXX_STANDARD 17
)
