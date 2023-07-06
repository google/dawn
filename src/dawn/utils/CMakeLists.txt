# Copyright 2020 The Dawn Authors
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
