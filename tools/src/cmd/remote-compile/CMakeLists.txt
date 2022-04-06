# Copyright 2021 The Tint Authors.
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

set(SRC
    main.cc
    rwmutex.h
    socket.cc
    socket.h
)

find_package (Threads REQUIRED)

add_executable(tint-remote-compile ${SRC})

target_link_libraries (tint-remote-compile Threads::Threads)
target_include_directories(tint-remote-compile PRIVATE "${TINT_ROOT_SOURCE_DIR}")

# If we're building on mac / ios and we have CoreGraphics, then we can use the
# metal API to validate our shaders. This is roughly 4x faster than invoking
# the metal shader compiler executable.
if(APPLE)
  find_library(LIB_CORE_GRAPHICS CoreGraphics)
  if(LIB_CORE_GRAPHICS)
    target_sources(tint-remote-compile PRIVATE "msl_metal.mm")
    target_compile_definitions(tint-remote-compile PRIVATE "-DTINT_ENABLE_MSL_COMPILATION_USING_METAL_API=1")
    target_compile_options(tint-remote-compile PRIVATE "-fmodules" "-fcxx-modules")
    target_link_options(tint-remote-compile PRIVATE "-framework" "CoreGraphics")
  endif()
endif()
