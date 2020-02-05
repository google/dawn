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

DawnJSONGenerator(
    TARGET "dawn_native_utils"
    PRINT_NAME "Dawn native utilities"
    RESULT_VARIABLE "DAWN_NATIVE_UTILS_GEN_SOURCES"
)

add_library(dawn_native STATIC ${DAWN_DUMMY_FILE})
target_sources(dawn_native PRIVATE
    "${DAWN_INCLUDE_DIR}/dawn_native/DawnNative.h"
    "${DAWN_INCLUDE_DIR}/dawn_native/dawn_native_export.h"
    ${DAWN_NATIVE_UTILS_GEN_SOURCES}
    "Adapter.cpp"
    "Adapter.h"
    "AttachmentState.cpp"
    "AttachmentState.h"
    "BackendConnection.cpp"
    "BackendConnection.h"
    "BindGroup.cpp"
    "BindGroup.h"
    "BindGroupLayout.cpp"
    "BindGroupLayout.h"
    "BindGroupTracker.h"
    "BuddyAllocator.cpp"
    "BuddyAllocator.h"
    "BuddyMemoryAllocator.cpp"
    "BuddyMemoryAllocator.h"
    "Buffer.cpp"
    "Buffer.h"
    "CachedObject.cpp"
    "CachedObject.h"
    "CommandAllocator.cpp"
    "CommandAllocator.h"
    "CommandBuffer.cpp"
    "CommandBuffer.h"
    "CommandBufferStateTracker.cpp"
    "CommandBufferStateTracker.h"
    "CommandEncoder.cpp"
    "CommandEncoder.h"
    "CommandValidation.cpp"
    "CommandValidation.h"
    "Commands.cpp"
    "Commands.h"
    "ComputePassEncoder.cpp"
    "ComputePassEncoder.h"
    "ComputePipeline.cpp"
    "ComputePipeline.h"
    "Device.cpp"
    "Device.h"
    "DynamicUploader.cpp"
    "DynamicUploader.h"
    "EncodingContext.cpp"
    "EncodingContext.h"
    "Error.cpp"
    "Error.h"
    "ErrorData.cpp"
    "ErrorData.h"
    "ErrorInjector.cpp"
    "ErrorInjector.h"
    "ErrorScope.cpp"
    "ErrorScope.h"
    "ErrorScopeTracker.cpp"
    "ErrorScopeTracker.h"
    "Extensions.cpp"
    "Extensions.h"
    "Fence.cpp"
    "Fence.h"
    "FenceSignalTracker.cpp"
    "FenceSignalTracker.h"
    "Format.cpp"
    "Format.h"
    "Forward.h"
    "Instance.cpp"
    "Instance.h"
    "ObjectBase.cpp"
    "ObjectBase.h"
    "PassResourceUsage.h"
    "PassResourceUsageTracker.cpp"
    "PassResourceUsageTracker.h"
    "PerStage.cpp"
    "PerStage.h"
    "Pipeline.cpp"
    "Pipeline.h"
    "PipelineLayout.cpp"
    "PipelineLayout.h"
    "ProgrammablePassEncoder.cpp"
    "ProgrammablePassEncoder.h"
    "Queue.cpp"
    "Queue.h"
    "RefCounted.cpp"
    "RefCounted.h"
    "RenderBundle.cpp"
    "RenderBundle.h"
    "RenderBundleEncoder.cpp"
    "RenderBundleEncoder.h"
    "RenderEncoderBase.cpp"
    "RenderEncoderBase.h"
    "RenderPassEncoder.cpp"
    "RenderPassEncoder.h"
    "RenderPipeline.cpp"
    "RenderPipeline.h"
    "ResourceHeap.h"
    "ResourceHeapAllocator.h"
    "ResourceMemoryAllocation.cpp"
    "ResourceMemoryAllocation.h"
    "RingBufferAllocator.cpp"
    "RingBufferAllocator.h"
    "Sampler.cpp"
    "Sampler.h"
    "ShaderModule.cpp"
    "ShaderModule.h"
    "StagingBuffer.cpp"
    "StagingBuffer.h"
    "Surface.cpp"
    "Surface.h"
    "SwapChain.cpp"
    "SwapChain.h"
    "Texture.cpp"
    "Texture.h"
    "ToBackend.h"
    "Toggles.cpp"
    "Toggles.h"
    "dawn_platform.h"
)
target_link_libraries(dawn_native
    PUBLIC dawncpp_headers
    PRIVATE dawn_common
            dawn_platform
            dawn_internal_config
            shaderc_spvc
            spirv-cross-core
)

if (DAWN_ENABLE_D3D12)
    #TODO
endif()

if (DAWN_ENABLE_METAL)
    target_sources(dawn_native PRIVATE
        "${DAWN_INCLUDE_DIR}/dawn_native/MetalBackend.h"
        "Surface_metal.mm"
        "metal/BackendMTL.h"
        "metal/BackendMTL.mm"
        "metal/BufferMTL.h"
        "metal/BufferMTL.mm"
        "metal/CommandBufferMTL.h"
        "metal/CommandBufferMTL.mm"
        "metal/CommandRecordingContext.h"
        "metal/CommandRecordingContext.mm"
        "metal/ComputePipelineMTL.h"
        "metal/ComputePipelineMTL.mm"
        "metal/DeviceMTL.h"
        "metal/DeviceMTL.mm"
        "metal/Forward.h"
        "metal/PipelineLayoutMTL.h"
        "metal/PipelineLayoutMTL.mm"
        "metal/QueueMTL.h"
        "metal/QueueMTL.mm"
        "metal/RenderPipelineMTL.h"
        "metal/RenderPipelineMTL.mm"
        "metal/SamplerMTL.h"
        "metal/SamplerMTL.mm"
        "metal/ShaderModuleMTL.h"
        "metal/ShaderModuleMTL.mm"
        "metal/StagingBufferMTL.h"
        "metal/StagingBufferMTL.mm"
        "metal/SwapChainMTL.h"
        "metal/SwapChainMTL.mm"
        "metal/TextureMTL.h"
        "metal/TextureMTL.mm"
        "metal/UtilsMetal.h"
        "metal/UtilsMetal.mm"
    )
    target_link_libraries(dawn_native PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework IOSurface"
        "-framework QuartzCore"
    )
endif()

if (DAWN_ENABLE_NULL)
    target_sources(dawn_native PRIVATE
        "${DAWN_INCLUDE_DIR}/dawn_native/NullBackend.h"
        "null/DeviceNull.cpp"
        "null/DeviceNull.h"
    )
endif()

if (DAWN_ENABLE_OPENGL)
    DawnGenerator(
        SCRIPT "${Dawn_SOURCE_DIR}/generator/opengl_loader_generator.py"
        PRINT_NAME "OpenGL function loader"
        ARGS "--gl-xml"
             "${Dawn_SOURCE_DIR}/third_party/khronos/gl.xml"
             "--supported-extensions"
             "${Dawn_SOURCE_DIR}/src/dawn_native/opengl/supported_extensions.json"
        RESULT_VARIABLE "DAWN_NATIVE_OPENGL_AUTOGEN_SOURCES"
    )

    # TODO
endif()

if (DAWN_ENABLE_VULKAN)
    # TODO
endif()

# TODO how to do the component build in CMake?
target_sources(dawn_native PRIVATE "DawnNative.cpp")
if (DAWN_ENABLE_D3D12)
    target_sources(dawn_native PRIVATE "d3d12/D3D12Backend.cpp")
endif()
if (DAWN_ENABLE_METAL)
    target_sources(dawn_native PRIVATE "metal/MetalBackend.mm")
endif()
if (DAWN_ENABLE_NULL)
    target_sources(dawn_native PRIVATE "null/NullBackend.cpp")
endif()
if (DAWN_ENABLE_OPENGL)
    target_sources(dawn_native PRIVATE "opengl/OpenGLBackend.cpp")
endif()
if (DAWN_ENABLE_VULKAN)
    target_sources(dawn_native PRIVATE "vulkan/VulkanBackend.cpp")
endif()
