# Copyright 2020 The Dawn & Tint Authors
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

set(private_headers
    "BinarySemaphore.h"
    "ComboRenderBundleEncoderDescriptor.h"
    "ComboRenderPipelineDescriptor.h"
    "PlatformDebugLogger.h"
    "SystemUtils.h"
    "TerribleCommandBuffer.h"
    "TestUtils.h"
    "TextureUtils.h"
    "Timer.h"
    "WGPUHelpers.h"
    "WireHelper.h"
)
set(sources
    "BinarySemaphore.cpp"
    "ComboRenderBundleEncoderDescriptor.cpp"
    "ComboRenderPipelineDescriptor.cpp"
    "SystemUtils.cpp"
    "TerribleCommandBuffer.cpp"
    "TestUtils.cpp"
    "TextureUtils.cpp"
    "WGPUHelpers.cpp"
    "WireHelper.cpp"
)
set(conditional_private_depends)
if(WIN32 AND NOT WINDOWS_STORE)
  list(APPEND sources "WindowsDebugLogger.cpp")
else()
  list(APPEND sources "EmptyDebugLogger.cpp")
endif()

if(WIN32)
  list(APPEND sources "WindowsTimer.cpp")
elseif(APPLE)
    list(APPEND private_headers
        "ObjCUtils.h"
    )
    list(APPEND sources
        "ObjCUtils.mm"
        "OSXTimer.cpp"
    )
    list(APPEND conditional_private_depends "-framework QuartzCore")
elseif(UNIX)
    list(APPEND sources "PosixTimer.cpp")
endif()

if (TINT_BUILD_SPV_READER)
    list(APPEND conditional_private_depends SPIRV-Tools-opt)
endif ()
dawn_add_library(
  dawn_utils
  UTILITY_TARGET dawn_internal_config
  PRIVATE_HEADERS
    ${private_headers}
  SOURCES
    ${sources}
  DEPENDS
    dawn::dawn_wire
    dawn::dawncpp_headers
    dawn::partition_alloc
  PRIVATE_DEPENDS
    ${conditional_private_depends}
)
# Needed by WGPUHelpers
target_compile_definitions(dawn_utils
  PUBLIC
    "-DTINT_BUILD_SPV_READER=$<BOOL:${TINT_BUILD_SPV_READER}>")
