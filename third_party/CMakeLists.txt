# Copyright 2020 The Tint Authors.
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

if (${TINT_BUILD_TESTS} AND NOT TARGET gmock)
  set(gtest_force_shared_crt ON CACHE BOOL "Controls whether a shared run-time library should be used even when Google Test is built as static library" FORCE)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest EXCLUDE_FROM_ALL)
endif()

if ((${TINT_BUILD_SPIRV_TOOLS_FUZZER} OR ${TINT_BUILD_AST_FUZZER}) AND
    (NOT TARGET protobuf::libprotobuf OR NOT TARGET protobuf::protoc))
  set(protobuf_BUILD_TESTS OFF CACHE BOOL "Controls whether protobuf tests are built" FORCE)
  set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "Controls whether a protobuf static runtime is built" FORCE)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/protobuf/cmake)
endif()

if(${TINT_BUILD_SPV_READER} OR ${TINT_BUILD_SPV_WRITER})
  if (NOT IS_DIRECTORY "${SPIRV-Headers_SOURCE_DIR}")
    set(SPIRV-Headers_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/spirv-headers CACHE STRING "Source directory for SPIR-V headers")
  endif()

  if (NOT TARGET SPIRV-Tools)
    set(SPIRV_SKIP_TESTS ON CACHE BOOL "Controls whether SPIR-V tests are run" FORCE)
    set(SPIRV_WERROR OFF CACHE BOOL OFF FORCE)
    if (${TINT_BUILD_SPIRV_TOOLS_FUZZER})
      set(SPIRV_BUILD_FUZZER ON CACHE BOOL "Controls whether spirv-fuzz is built" FORCE)
    endif()
    set(SPIRV-Headers_SOURCE_DIR "${TINT_THIRD_PARTY_DIR}/spirv-headers")
    add_subdirectory("${TINT_THIRD_PARTY_DIR}/spirv-tools" "${CMAKE_BINARY_DIR}/third_party/spirv-tools" EXCLUDE_FROM_ALL)
  endif()
endif()

if(${TINT_BUILD_GLSL_WRITER})
  set(SPIRV-Headers_SOURCE_DIR "${TINT_THIRD_PARTY_DIR}/glslang")
  add_subdirectory("${TINT_THIRD_PARTY_DIR}/glslang" "${CMAKE_BINARY_DIR}/third_party/glslang" EXCLUDE_FROM_ALL)
endif()
