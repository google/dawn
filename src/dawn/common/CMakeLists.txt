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
    "ContentLessObjectCache.h"
    "ContentLessObjectCacheable.h"
    "CoreFoundationRef.h"
    "DynamicLib.cpp"
    "DynamicLib.h"
    "FutureUtils.h"
    "GPUInfo.cpp"
    "GPUInfo.h"
    "HashUtils.h"
    "IOKitRef.h"
    "LinkedList.h"
    "Log.cpp"
    "Log.h"
    "Math.cpp"
    "Math.h"
    "Mutex.cpp"
    "Mutex.h"
    "MutexProtected.h"
    "NSRef.h"
    "NonCopyable.h"
    "Numeric.h"
    "PlacementAllocated.h"
    "Platform.h"
    "Preprocessor.h"
    "Ref.h"
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
    "SystemUtils.cpp"
    "SystemUtils.h"
    "TypeTraits.h"
    "TypedInteger.h"
    "UnderlyingType.h"
    "WeakRef.h"
    "WeakRefSupport.cpp"
    "WeakRefSupport.h"
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
        "IOSurfaceUtils.cpp"
        "IOSurfaceUtils.h"
        "SystemUtils_mac.mm"
    )
endif()

target_link_libraries(dawn_common PUBLIC dawncpp_headers PRIVATE dawn_internal_config)

# TODO Android Log support
# TODO Vulkan headers support
