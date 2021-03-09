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
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest EXCLUDE_FROM_ALL)
endif()

if(${TINT_BUILD_SPV_READER} OR ${TINT_BUILD_SPV_WRITER})
  if (NOT IS_DIRECTORY "${SPIRV-Headers_SOURCE_DIR}")
    set(SPIRV-Headers_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/spirv-headers CACHE STRING "")
  endif()

  if (NOT TARGET SPIRV-Tools)
    set(SPIRV_SKIP_TESTS ON CACHE BOOL ON)
    set(SPIRV_WERROR OFF CACHE BOOL OFF)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/spirv-tools EXCLUDE_FROM_ALL)
  endif()
endif()
