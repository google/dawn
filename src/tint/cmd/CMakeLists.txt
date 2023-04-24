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

## Tint executable
add_executable(tint "")
target_sources(tint PRIVATE
  "generate_external_texture_bindings.cc"
  "generate_external_texture_bindings.h"
  "helper.cc"
  "helper.h"
  "main.cc"
)
tint_default_compile_options(tint)
target_link_libraries(tint libtint tint_val)

if (${TINT_BUILD_SPV_READER} OR ${TINT_BUILD_SPV_WRITER})
  target_link_libraries(tint SPIRV-Tools)
endif()

if (${TINT_BUILD_GLSL_WRITER})
  target_link_libraries(tint glslang)
  target_link_libraries(tint glslang-default-resource-limits)
  if (NOT MSVC)
    target_compile_options(tint PRIVATE
      -Wno-reserved-id-macro
      -Wno-shadow-field-in-constructor
      -Wno-shadow
      -Wno-weak-vtables
    )
  endif()
endif()

add_executable(tint_info "")
target_sources(tint_info PRIVATE
  "helper.cc"
  "helper.h"
  "info.cc"
)
tint_default_compile_options(tint_info)
target_link_libraries(tint_info libtint tint_val)

if (${TINT_BUILD_SPV_READER})
    target_link_libraries(tint_info SPIRV-Tools)
endif()

add_executable(tint-loopy "")
target_sources(tint-loopy PRIVATE
  "generate_external_texture_bindings.cc"
  "generate_external_texture_bindings.h"
  "helper.cc"
  "helper.h"
  "loopy.cc"
)
tint_default_compile_options(tint-loopy)
target_link_libraries(tint-loopy libtint tint_val)

