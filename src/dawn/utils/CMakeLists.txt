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

add_library(dawn_utils STATIC ${DAWN_PLACEHOLDER_FILE})
common_compile_options(dawn_utils)
target_sources(dawn_utils PRIVATE
    "BinarySemaphore.cpp"
    "BinarySemaphore.h"
    "ComboRenderBundleEncoderDescriptor.cpp"
    "ComboRenderBundleEncoderDescriptor.h"
    "ComboRenderPipelineDescriptor.cpp"
    "ComboRenderPipelineDescriptor.h"
    "PlatformDebugLogger.h"
    "SystemUtils.cpp"
    "SystemUtils.h"
    "TerribleCommandBuffer.cpp"
    "TerribleCommandBuffer.h"
    "TestUtils.cpp"
    "TestUtils.h"
    "TextureUtils.cpp"
    "TextureUtils.h"
    "Timer.h"
    "WGPUHelpers.cpp"
    "WGPUHelpers.h"
    "WireHelper.cpp"
    "WireHelper.h"
)
target_link_libraries(dawn_utils
    PUBLIC dawncpp_headers
    PRIVATE dawn_internal_config
            dawn_common
            dawn_native
            dawn_proc
            dawn_wire
            SPIRV-Tools-opt
)

if(WIN32 AND NOT WINDOWS_STORE)
    target_sources(dawn_utils PRIVATE "WindowsDebugLogger.cpp")
else()
    target_sources(dawn_utils PRIVATE "EmptyDebugLogger.cpp")
endif()

if(WIN32)
    target_sources(dawn_utils PRIVATE "WindowsTimer.cpp")
elseif(APPLE)
    target_sources(dawn_utils PRIVATE
        "OSXTimer.cpp"
        "ObjCUtils.h"
        "ObjCUtils.mm"
    )
    target_link_libraries(dawn_utils PRIVATE "-framework QuartzCore")
elseif(UNIX)
    target_sources(dawn_utils PRIVATE "PosixTimer.cpp")
endif()

if (DAWN_ENABLE_METAL)
    target_link_libraries(dawn_utils PRIVATE "-framework Metal")
endif()
