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

# Paths to generated files
set(INTEROP_GEN_DIR    "${DAWN_NODE_GEN_DIR}/src/dawn/node/interop")
set(INTEROP_WEBGPU_H   "${INTEROP_GEN_DIR}/WebGPU.h")
set(INTEROP_WEBGPU_CPP "${INTEROP_GEN_DIR}/WebGPU.cpp")

idlgen(
    TEMPLATE
        "${CMAKE_CURRENT_SOURCE_DIR}/WebGPU.h.tmpl"
    IDLS
        "${CMAKE_CURRENT_SOURCE_DIR}/Browser.idl"
        "${WEBGPU_IDL_PATH}"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/WebGPUCommon.tmpl"
    OUTPUT
        "${INTEROP_WEBGPU_H}"
)

idlgen(
    TEMPLATE
        "${CMAKE_CURRENT_SOURCE_DIR}/WebGPU.cpp.tmpl"
    IDLS
        "${CMAKE_CURRENT_SOURCE_DIR}/Browser.idl"
        "${WEBGPU_IDL_PATH}"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/WebGPUCommon.tmpl"
    OUTPUT
        "${INTEROP_WEBGPU_CPP}"
)

add_library(dawn_node_interop STATIC
    "Core.cpp"
    "Core.h"
    "${INTEROP_WEBGPU_H}"
    "${INTEROP_WEBGPU_CPP}"
)
common_compile_options(dawn_node_interop)

target_include_directories(dawn_node_interop
    PRIVATE
        "${PROJECT_SOURCE_DIR}"
        "${DAWN_THIRD_PARTY_DIR}"
        "${NODE_API_HEADERS_DIR}/include"
        "${DAWN_NODE_GEN_DIR}"
)

target_link_libraries(dawn_node_interop
    PRIVATE
        dawncpp
)
