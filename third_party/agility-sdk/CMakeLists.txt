# Copyright 2026 The Dawn & Tint Authors
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

add_library(agility-sdk INTERFACE)
add_library(dawn::agility-sdk ALIAS agility-sdk)

target_include_directories(agility-sdk INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/build/native/include>"
)

target_compile_definitions(agility-sdk INTERFACE
    DAWN_USE_AGILITY_SDK
)

# Determine the architecture-specific DLL source directory
if (CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|arm64|aarch64")
    set(_agility_sdk_bin_dir "${CMAKE_CURRENT_SOURCE_DIR}/src/build/native/bin/arm64")
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(_agility_sdk_bin_dir "${CMAKE_CURRENT_SOURCE_DIR}/src/build/native/bin/win32")
else()
    set(_agility_sdk_bin_dir "${CMAKE_CURRENT_SOURCE_DIR}/src/build/native/bin/x64")
endif()

get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (isMultiConfig)
    set(_agility_sdk_out_dir "${CMAKE_BINARY_DIR}/$<CONFIG>/D3D12")
else()
    set(_agility_sdk_out_dir "${CMAKE_BINARY_DIR}/D3D12")
endif()

add_custom_target(copy_agility_sdk_dlls
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_agility_sdk_out_dir}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${_agility_sdk_bin_dir}/D3D12Core.dll"
        "${_agility_sdk_bin_dir}/d3d12SDKLayers.dll"
        "${_agility_sdk_out_dir}"
    COMMENT "Copying Agility SDK DLLs to ${_agility_sdk_out_dir}")

add_dependencies(agility-sdk copy_agility_sdk_dlls)
