# Copyright 2024 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_policy(SET CMP0048 NEW)
project(webgpu VERSION 1.1.0 LANGUAGES CXX)

cmake_minimum_required(VERSION 3.22.1)

set(DAWN_BUILD_ANDROID_SAMPLES OFF)
set(DAWN_BUILD_SAMPLES OFF)
set(TINT_BUILD_TESTS OFF)
set(TINT_BUILD_CMD_TOOLS OFF)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)

add_subdirectory(../../../../../.. dawn)

set(C_BINDINGS_PATH ../../../../../../out/Debug/gen/cpp)

add_library(native_dawn ${C_BINDINGS_PATH}/methods.cpp ${C_BINDINGS_PATH}/structures.cpp Util.cpp ${CMAKE_SOURCE_DIR}/gen/include/dawn/webgpu.h)

target_link_libraries(native_dawn webgpu_dawn)

list(INSERT CMAKE_MODULE_PATH 0 ../../../../../../src/cmake)
include(BundleLibraries)
bundle_libraries(webgpu_c_bundled native_dawn)

find_library(log-lib log)

target_link_libraries(webgpu_c_bundled ${log-lib} android)

add_custom_command(DEPENDS webgpu_dawn POST_BUILD
                   OUTPUT ${CMAKE_SOURCE_DIR}/gen/include/dawn/webgpu.h
                   COMMAND rm -r "${CMAKE_SOURCE_DIR}/gen" || echo
                   COMMAND ${CMAKE_COMMAND} -E copy
                           ${CMAKE_CURRENT_BINARY_DIR}/dawn/gen/include/dawn/webgpu.h
                           ${CMAKE_SOURCE_DIR}/gen/include/dawn/webgpu.h)
