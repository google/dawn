# Copyright 2022 The Dawn Authors
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

if(DAWN_SUPPORTS_GLFW_FOR_WINDOWING)

add_library(dawn_glfw STATIC ${DAWN_PLACEHOLDER_FILE})
common_compile_options(dawn_glfw)
target_sources(dawn_glfw PRIVATE
    "utils.cpp"
)
target_link_libraries(dawn_glfw
  PUBLIC
    glfw
    dawncpp_headers
    dawn_common
  PRIVATE
    dawn_internal_config
)

target_compile_definitions(dawn_glfw PRIVATE "WGPU_IMPLEMENTATION")
if(BUILD_SHARED_LIBS)
    target_compile_definitions(dawn_glfw PRIVATE "WGPU_SHARED_LIBRARY")
endif()

if (DAWN_ENABLE_METAL)
    target_link_libraries(dawn_glfw PRIVATE "-framework Metal")
    target_sources(dawn_glfw PRIVATE
        "utils_metal.mm"
    )
endif()

add_library(webgpu_glfw ALIAS dawn_glfw)

endif()
