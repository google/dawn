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

DawnGenerator(
    SCRIPT "${Dawn_SOURCE_DIR}/generator/dawn_version_generator.py"
    PRINT_NAME "Dawn version based utilities"
    ARGS "--dawn-dir"
         "${Dawn_SOURCE_DIR}"
    RESULT_VARIABLE "DAWN_VERSION_AUTOGEN_SOURCES"
)

DawnGenerator(
    SCRIPT "${Dawn_SOURCE_DIR}/generator/dawn_gpu_info_generator.py"
    PRINT_NAME "Dawn GPU info utilities"
    ARGS "--gpu-info-json"
         "${Dawn_SOURCE_DIR}/src/dawn/gpu_info.json"
    RESULT_VARIABLE "DAWN_GPU_INFO_AUTOGEN_SOURCES"
)

add_library(dawn_common STATIC ${DAWN_PLACEHOLDER_FILE})
common_compile_options(dawn_common)
target_sources(dawn_common PRIVATE
    ${DAWN_VERSION_AUTOGEN_SOURCES}
    ${DAWN_GPU_INFO_AUTOGEN_SOURCES}
    "Alloc.h"
    "Assert.cpp"
    "Assert.h"
    "BitSetIterator.h"
    "Compiler.h"
    "ConcurrentCache.h"
    "Constants.h"
    "CoreFoundationRef.h"
    "DynamicLib.cpp"
    "DynamicLib.h"
    "GPUInfo.cpp"
    "GPUInfo.h"
    "HashUtils.h"
    "IOKitRef.h"
    "LinkedList.h"
    "Log.cpp"
    "Log.h"
    "Math.cpp"
    "Math.h"
    "NSRef.h"
    "NonCopyable.h"
    "Numeric.h"
    "PlacementAllocated.h"
    "Platform.h"
    "Preprocessor.h"
    "RefBase.h"
    "RefCounted.cpp"
    "RefCounted.h"
    "Result.cpp"
    "Result.h"
    "SerialMap.h"
    "SerialQueue.h"
    "SerialStorage.h"
    "SlabAllocator.cpp"
    "SlabAllocator.h"
    "StackContainer.h"
    "SwapChainUtils.h"
    "SystemUtils.cpp"
    "SystemUtils.h"
    "TypeTraits.h"
    "TypedInteger.h"
    "UnderlyingType.h"
    "ityp_array.h"
    "ityp_bitset.h"
    "ityp_span.h"
    "ityp_stack_vec.h"
    "ityp_vector.h"
    "vulkan_platform.h"
    "xlib_with_undefs.h"
)

if (WIN32)
    target_sources(dawn_common PRIVATE
        "WindowsUtils.cpp"
        "WindowsUtils.h"
        "windows_with_undefs.h"
    )
elseif(APPLE)
    target_sources(dawn_common PRIVATE
        "SystemUtils_mac.mm"
    )
endif()

target_link_libraries(dawn_common PUBLIC dawncpp_headers PRIVATE dawn_internal_config)

# TODO Android Log support
# TODO Vulkan headers support
