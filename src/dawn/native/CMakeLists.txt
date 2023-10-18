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

DawnJSONGenerator(
    TARGET "native_utils"
    PRINT_NAME "Dawn native utilities"
    RESULT_VARIABLE "DAWN_NATIVE_UTILS_GEN_SOURCES"
)

add_library(dawn_native ${DAWN_PLACEHOLDER_FILE})
common_compile_options(dawn_native)

target_compile_definitions(dawn_native PRIVATE "DAWN_NATIVE_IMPLEMENTATION")
if(BUILD_SHARED_LIBS)
    target_compile_definitions(dawn_native PRIVATE "DAWN_NATIVE_SHARED_LIBRARY")
endif()

target_sources(dawn_native
  INTERFACE
    "${DAWN_INCLUDE_DIR}/dawn/native/DawnNative.h"
    "${DAWN_INCLUDE_DIR}/dawn/native/dawn_native_export.h"
  PRIVATE
    ${DAWN_NATIVE_UTILS_GEN_SOURCES}
    "Adapter.h"
    "Adapter.cpp"
    "ApplyClearColorValueWithDrawHelper.cpp"
    "ApplyClearColorValueWithDrawHelper.h"
    "AsyncTask.cpp"
    "AsyncTask.h"
    "AttachmentState.cpp"
    "AttachmentState.h"
    "BackendConnection.cpp"
    "BackendConnection.h"
    "BindGroup.cpp"
    "BindGroup.h"
    "BindGroupLayout.cpp"
    "BindGroupLayout.h"
    "BindGroupLayoutInternal.cpp"
    "BindGroupLayoutInternal.h"
    "BindGroupTracker.h"
    "BindingInfo.cpp"
    "BindingInfo.h"
    "BlitBufferToDepthStencil.cpp"
    "BlitBufferToDepthStencil.h"
    "BlitColorToColorWithDraw.cpp"
    "BlitColorToColorWithDraw.h"
    "BlitDepthToDepth.cpp"
    "BlitDepthToDepth.h"
    "BlitTextureToBuffer.cpp"
    "BlitTextureToBuffer.h"
    "Blob.cpp"
    "Blob.h"
    "BlobCache.cpp"
    "BlobCache.h"
    "BuddyAllocator.cpp"
    "BuddyAllocator.h"
    "BuddyMemoryAllocator.cpp"
    "BuddyMemoryAllocator.h"
    "Buffer.cpp"
    "Buffer.h"
    "CachedObject.cpp"
    "CachedObject.h"
    "CacheKey.cpp"
    "CacheKey.h"
    "CacheRequest.cpp"
    "CacheRequest.h"
    "CacheResult.h"
    "CallbackTaskManager.cpp"
    "CallbackTaskManager.h"
    "ChainUtils.h"
    "ChainUtilsImpl.inl"
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
    "CompilationMessages.cpp"
    "CompilationMessages.h"
    "ComputePassEncoder.cpp"
    "ComputePassEncoder.h"
    "ComputePipeline.cpp"
    "ComputePipeline.h"
    "CopyTextureForBrowserHelper.cpp"
    "CopyTextureForBrowserHelper.h"
    "CreatePipelineAsyncTask.cpp"
    "CreatePipelineAsyncTask.h"
    "Device.cpp"
    "Device.h"
    "DynamicUploader.cpp"
    "DynamicUploader.h"
    "EncodingContext.cpp"
    "EncodingContext.h"
    "EnumClassBitmasks.h"
    "EnumMaskIterator.h"
    "Error.cpp"
    "Error.h"
    "ErrorData.cpp"
    "ErrorData.h"
    "ErrorInjector.cpp"
    "ErrorInjector.h"
    "ErrorScope.cpp"
    "ErrorScope.h"
    "EventManager.cpp"
    "EventManager.h"
    "Features.cpp"
    "Features.h"
    "ExternalTexture.cpp"
    "ExternalTexture.h"
    "ExecutionQueue.cpp"
    "ExecutionQueue.h"
    "IndirectDrawMetadata.cpp"
    "IndirectDrawMetadata.h"
    "IndirectDrawValidationEncoder.cpp"
    "IndirectDrawValidationEncoder.h"
    "ObjectContentHasher.cpp"
    "ObjectContentHasher.h"
    "Format.cpp"
    "Format.h"
    "Forward.h"
    "Instance.cpp"
    "Instance.h"
    "InternalPipelineStore.cpp"
    "InternalPipelineStore.h"
    "IntegerTypes.h"
    "Limits.cpp"
    "Limits.h"
    "SystemEvent.cpp"
    "SystemEvent.h"
    "ObjectBase.cpp"
    "ObjectBase.h"
    "PassResourceUsage.cpp"
    "PassResourceUsage.h"
    "PassResourceUsageTracker.cpp"
    "PassResourceUsageTracker.h"
    "PerStage.cpp"
    "PerStage.h"
    "PhysicalDevice.cpp"
    "PhysicalDevice.h"
    "Pipeline.cpp"
    "Pipeline.h"
    "PipelineCache.cpp"
    "PipelineCache.h"
    "PipelineLayout.cpp"
    "PipelineLayout.h"
    "PooledResourceMemoryAllocator.cpp"
    "PooledResourceMemoryAllocator.h"
    "ProgrammableEncoder.cpp"
    "ProgrammableEncoder.h"
    "QueryHelper.cpp"
    "QueryHelper.h"
    "QuerySet.cpp"
    "QuerySet.h"
    "Queue.cpp"
    "Queue.h"
    "RefCountedWithExternalCount.cpp"
    "RefCountedWithExternalCount.h"
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
    "ScratchBuffer.cpp"
    "ScratchBuffer.h"
    "Serializable.h"
    "ShaderModule.cpp"
    "ShaderModule.h"
    "SharedFence.cpp"
    "SharedFence.h"
    "SharedTextureMemory.cpp"
    "SharedTextureMemory.h"
    "Subresource.cpp"
    "Subresource.h"
    "SubresourceStorage.h"
    "Surface.cpp"
    "Surface.h"
    "SwapChain.cpp"
    "SwapChain.h"
    "Texture.cpp"
    "Texture.h"
    "TintUtils.cpp"
    "TintUtils.h"
    "ToBackend.h"
    "Toggles.cpp"
    "Toggles.h"
    "UsageValidationMode.h"
    "VisitableMembers.h"
    "dawn_platform.h"
    "webgpu_absl_format.cpp"
    "webgpu_absl_format.h"
    "stream/BlobSource.cpp"
    "stream/BlobSource.h"
    "stream/ByteVectorSink.cpp"
    "stream/ByteVectorSink.h"
    "stream/Sink.h"
    "stream/Source.h"
    "stream/Stream.cpp"
    "stream/Stream.h"
    "utils/WGPUHelpers.cpp"
    "utils/WGPUHelpers.h"
)
target_link_libraries(dawn_native
    PUBLIC dawncpp_headers
    PRIVATE dawn_common
            dawn_platform
            dawn_internal_config
            libtint
            SPIRV-Tools-opt
            absl_strings
            absl_str_format_internal
)

target_include_directories(dawn_native PRIVATE ${DAWN_ABSEIL_DIR})

if (DAWN_USE_X11)
    target_include_directories(dawn_native PRIVATE ${X11_INCLUDE_DIR})
    target_sources(dawn_native PRIVATE
        "X11Functions.cpp"
        "X11Functions.h"
    )
endif()

# Only win32 app needs to link with user32.lib
# In UWP, all availiable APIs are defined in WindowsApp.lib
# and is automatically linked when WINDOWS_STORE set
if (WIN32 AND NOT WINDOWS_STORE)
    target_link_libraries(dawn_native PRIVATE user32.lib)
endif()

# DXGIGetDebugInterface1 is defined in dxgi.lib
# But this API is tagged as a development-only capability
# which implies that linking to this function will cause
# the application to fail Windows store certification
# So we only link to it in debug build when compiling for UWP.
# In win32 we load dxgi.dll using LoadLibrary
# so no need for static linking.
if (WINDOWS_STORE)
    target_link_libraries(dawn_native PRIVATE debug dxgi.lib)
endif()

if (DAWN_ENABLE_D3D11 OR DAWN_ENABLE_D3D12)
    target_sources(dawn_native
      INTERFACE
        "${DAWN_INCLUDE_DIR}/dawn/native/D3DBackend.h"
      PRIVATE
        "d3d/BackendD3D.cpp"
        "d3d/BackendD3D.h"
        "d3d/BlobD3D.cpp"
        "d3d/BlobD3D.h"
        "d3d/D3DCompilationRequest.h"
        "d3d/D3DError.cpp"
        "d3d/D3DError.h"
        "d3d/DeviceD3D.cpp"
        "d3d/DeviceD3D.h"
        "d3d/ExternalImageDXGIImpl.cpp"
        "d3d/ExternalImageDXGIImpl.h"
        "d3d/Fence.cpp"
        "d3d/Fence.h"
        "d3d/Forward.h"
        "d3d/PhysicalDeviceD3D.cpp"
        "d3d/PhysicalDeviceD3D.h"
        "d3d/PlatformFunctions.cpp"
        "d3d/PlatformFunctions.h"
        "d3d/ShaderUtils.cpp"
        "d3d/ShaderUtils.h"
        "d3d/SharedFenceD3D.cpp"
        "d3d/SharedFenceD3D.h"
        "d3d/SharedTextureMemoryD3D.cpp"
        "d3d/SharedTextureMemoryD3D.h"
        "d3d/SwapChainD3D.cpp"
        "d3d/SwapChainD3D.h"
        "d3d/TextureD3D.cpp"
        "d3d/TextureD3D.h"
        "d3d/UtilsD3D.cpp"
        "d3d/UtilsD3D.h"
        "d3d/d3d_platform.h"
    )
endif()

if (DAWN_ENABLE_D3D11)
    target_sources(dawn_native
      INTERFACE
        "${DAWN_INCLUDE_DIR}/dawn/native/D3D11Backend.h"
      PRIVATE
        "d3d11/BackendD3D11.cpp"
        "d3d11/BackendD3D11.h"
        "d3d11/BindGroupD3D11.cpp"
        "d3d11/BindGroupD3D11.h"
        "d3d11/BindGroupLayoutD3D11.cpp"
        "d3d11/BindGroupLayoutD3D11.h"
        "d3d11/BindGroupTrackerD3D11.cpp"
        "d3d11/BindGroupTrackerD3D11.h"
        "d3d11/BufferD3D11.cpp"
        "d3d11/BufferD3D11.h"
        "d3d11/CommandBufferD3D11.cpp"
        "d3d11/CommandBufferD3D11.h"
        "d3d11/CommandRecordingContextD3D11.cpp"
        "d3d11/CommandRecordingContextD3D11.h"
        "d3d11/ComputePipelineD3D11.cpp"
        "d3d11/ComputePipelineD3D11.h"
        "d3d11/DeviceD3D11.cpp"
        "d3d11/DeviceD3D11.h"
        "d3d11/DeviceInfoD3D11.cpp"
        "d3d11/DeviceInfoD3D11.h"
        "d3d11/FenceD3D11.cpp"
        "d3d11/FenceD3D11.h"
        "d3d11/Forward.h"
        "d3d11/PhysicalDeviceD3D11.cpp"
        "d3d11/PhysicalDeviceD3D11.h"
        "d3d11/PipelineLayoutD3D11.cpp"
        "d3d11/PipelineLayoutD3D11.h"
        "d3d11/PlatformFunctionsD3D11.cpp"
        "d3d11/PlatformFunctionsD3D11.h"
        "d3d11/QuerySetD3D11.cpp"
        "d3d11/QuerySetD3D11.h"
        "d3d11/QueueD3D11.cpp"
        "d3d11/QueueD3D11.h"
        "d3d11/RenderPipelineD3D11.cpp"
        "d3d11/RenderPipelineD3D11.h"
        "d3d11/SamplerD3D11.cpp"
        "d3d11/SamplerD3D11.h"
        "d3d11/SharedFenceD3D11.cpp"
        "d3d11/SharedFenceD3D11.h"
        "d3d11/SharedTextureMemoryD3D11.cpp"
        "d3d11/SharedTextureMemoryD3D11.h"
        "d3d11/ShaderModuleD3D11.cpp"
        "d3d11/ShaderModuleD3D11.h"
        "d3d11/SwapChainD3D11.cpp"
        "d3d11/SwapChainD3D11.h"
        "d3d11/TextureD3D11.cpp"
        "d3d11/TextureD3D11.h"
        "d3d11/UtilsD3D11.cpp"
        "d3d11/UtilsD3D11.h"
    )
endif()

if (DAWN_ENABLE_D3D12)
    target_sources(dawn_native
      INTERFACE
        "${DAWN_INCLUDE_DIR}/dawn/native/D3D12Backend.h"
      PRIVATE
        "d3d12/BackendD3D12.cpp"
        "d3d12/BackendD3D12.h"
        "d3d12/BindGroupD3D12.cpp"
        "d3d12/BindGroupD3D12.h"
        "d3d12/BindGroupLayoutD3D12.cpp"
        "d3d12/BindGroupLayoutD3D12.h"
        "d3d12/BufferD3D12.cpp"
        "d3d12/BufferD3D12.h"
        "d3d12/CPUDescriptorHeapAllocationD3D12.cpp"
        "d3d12/CPUDescriptorHeapAllocationD3D12.h"
        "d3d12/CommandAllocatorManager.cpp"
        "d3d12/CommandAllocatorManager.h"
        "d3d12/CommandBufferD3D12.cpp"
        "d3d12/CommandBufferD3D12.h"
        "d3d12/CommandRecordingContext.cpp"
        "d3d12/CommandRecordingContext.h"
        "d3d12/ComputePipelineD3D12.cpp"
        "d3d12/ComputePipelineD3D12.h"
        "d3d12/D3D12Info.cpp"
        "d3d12/D3D12Info.h"
        "d3d12/DeviceD3D12.cpp"
        "d3d12/DeviceD3D12.h"
        "d3d12/FenceD3D12.cpp"
        "d3d12/FenceD3D12.h"
        "d3d12/Forward.h"
        "d3d12/GPUDescriptorHeapAllocationD3D12.cpp"
        "d3d12/GPUDescriptorHeapAllocationD3D12.h"
        "d3d12/HeapAllocatorD3D12.cpp"
        "d3d12/HeapAllocatorD3D12.h"
        "d3d12/HeapD3D12.cpp"
        "d3d12/HeapD3D12.h"
        "d3d12/IntegerTypes.h"
        "d3d12/PageableD3D12.cpp"
        "d3d12/PageableD3D12.h"
        "d3d12/PhysicalDeviceD3D12.cpp"
        "d3d12/PhysicalDeviceD3D12.h"
        "d3d12/PipelineLayoutD3D12.cpp"
        "d3d12/PipelineLayoutD3D12.h"
        "d3d12/PlatformFunctionsD3D12.cpp"
        "d3d12/PlatformFunctionsD3D12.h"
        "d3d12/QuerySetD3D12.cpp"
        "d3d12/QuerySetD3D12.h"
        "d3d12/QueueD3D12.cpp"
        "d3d12/QueueD3D12.h"
        "d3d12/RenderPassBuilderD3D12.cpp"
        "d3d12/RenderPassBuilderD3D12.h"
        "d3d12/RenderPipelineD3D12.cpp"
        "d3d12/RenderPipelineD3D12.h"
        "d3d12/ResidencyManagerD3D12.cpp"
        "d3d12/ResidencyManagerD3D12.h"
        "d3d12/ResourceAllocatorManagerD3D12.cpp"
        "d3d12/ResourceAllocatorManagerD3D12.h"
        "d3d12/ResourceHeapAllocationD3D12.cpp"
        "d3d12/ResourceHeapAllocationD3D12.h"
        "d3d12/SamplerD3D12.cpp"
        "d3d12/SamplerD3D12.h"
        "d3d12/SamplerHeapCacheD3D12.cpp"
        "d3d12/SamplerHeapCacheD3D12.h"
        "d3d12/ShaderModuleD3D12.cpp"
        "d3d12/ShaderModuleD3D12.h"
        "d3d12/ShaderVisibleDescriptorAllocatorD3D12.cpp"
        "d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"
        "d3d12/StagingDescriptorAllocatorD3D12.cpp"
        "d3d12/StagingDescriptorAllocatorD3D12.h"
        "d3d12/SharedFenceD3D12.cpp"
        "d3d12/SharedFenceD3D12.h"
        "d3d12/SharedTextureMemoryD3D12.cpp"
        "d3d12/SharedTextureMemoryD3D12.h"
        "d3d12/StreamImplD3D12.cpp"
        "d3d12/SwapChainD3D12.cpp"
        "d3d12/SwapChainD3D12.h"
        "d3d12/TextureCopySplitter.cpp"
        "d3d12/TextureCopySplitter.h"
        "d3d12/TextureD3D12.cpp"
        "d3d12/TextureD3D12.h"
        "d3d12/UtilsD3D12.cpp"
        "d3d12/UtilsD3D12.h"
        "d3d12/d3d12_platform.h"
    )
    target_link_libraries(dawn_native PRIVATE dxguid.lib)
endif()

if (DAWN_ENABLE_METAL)
    target_sources(dawn_native
      INTERFACE
        "${DAWN_INCLUDE_DIR}/dawn/native/MetalBackend.h"
      PRIVATE
        "Surface_metal.mm"
        "metal/BackendMTL.h"
        "metal/BackendMTL.mm"
        "metal/BindGroupLayoutMTL.h"
        "metal/BindGroupLayoutMTL.mm"
        "metal/BindGroupMTL.h"
        "metal/BindGroupMTL.mm"
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
        "metal/QuerySetMTL.h"
        "metal/QuerySetMTL.mm"
        "metal/RenderPipelineMTL.h"
        "metal/RenderPipelineMTL.mm"
        "metal/SamplerMTL.h"
        "metal/SamplerMTL.mm"
        "metal/ShaderModuleMTL.h"
        "metal/ShaderModuleMTL.mm"
        "metal/SharedFenceMTL.h"
        "metal/SharedFenceMTL.mm"
        "metal/SharedTextureMemoryMTL.h"
        "metal/SharedTextureMemoryMTL.mm"
        "metal/SwapChainMTL.h"
        "metal/SwapChainMTL.mm"
        "metal/TextureMTL.h"
        "metal/TextureMTL.mm"
        "metal/UtilsMetal.h"
        "metal/UtilsMetal.mm"
    )
    if (DAWN_TARGET_MACOS)
        target_link_libraries(dawn_native PRIVATE "-framework Cocoa")
    endif()
    target_link_libraries(dawn_native PRIVATE
        "-framework IOKit"
        "-framework IOSurface"
        "-framework QuartzCore"
        "-framework Metal"
    )
endif()

if (DAWN_ENABLE_NULL)
    target_sources(dawn_native
      INTERFACE
        "${DAWN_INCLUDE_DIR}/dawn/native/NullBackend.h"
      PRIVATE
        "null/DeviceNull.cpp"
        "null/DeviceNull.h"
    )
endif()

if (DAWN_ENABLE_OPENGL OR DAWN_ENABLE_VULKAN)
    target_compile_definitions(dawn_native PRIVATE "DAWN_ENABLE_SPIRV_VALIDATION")
    target_sources(dawn_native PRIVATE
        "SpirvValidation.cpp"
        "SpirvValidation.h"
    )
endif()

if (DAWN_ENABLE_OPENGL)
    DawnGenerator(
        SCRIPT "${Dawn_SOURCE_DIR}/generator/opengl_loader_generator.py"
        PRINT_NAME "OpenGL function loader"
        ARGS "--gl-xml"
             "${Dawn_SOURCE_DIR}/third_party/khronos/OpenGL-Registry/xml/gl.xml"
             "--supported-extensions"
             "${Dawn_SOURCE_DIR}/src/dawn/native/opengl/supported_extensions.json"
        RESULT_VARIABLE "DAWN_NATIVE_OPENGL_AUTOGEN_SOURCES"
    )

    target_sources(dawn_native
      INTERFACE
        "${DAWN_INCLUDE_DIR}/dawn/native/OpenGLBackend.h"
      PRIVATE
        ${DAWN_NATIVE_OPENGL_AUTOGEN_SOURCES}
        "opengl/BackendGL.cpp"
        "opengl/BackendGL.h"
        "opengl/BindGroupGL.cpp"
        "opengl/BindGroupGL.h"
        "opengl/BindGroupLayoutGL.cpp"
        "opengl/BindGroupLayoutGL.h"
        "opengl/BufferGL.cpp"
        "opengl/BufferGL.h"
        "opengl/CommandBufferGL.cpp"
        "opengl/CommandBufferGL.h"
        "opengl/ComputePipelineGL.cpp"
        "opengl/ComputePipelineGL.h"
        "opengl/ContextEGL.cpp"
        "opengl/ContextEGL.h"
        "opengl/DeviceGL.cpp"
        "opengl/DeviceGL.h"
        "opengl/EGLFunctions.cpp"
        "opengl/EGLFunctions.h"
        "opengl/Forward.h"
        "opengl/GLFormat.cpp"
        "opengl/GLFormat.h"
        "opengl/OpenGLFunctions.cpp"
        "opengl/OpenGLFunctions.h"
        "opengl/OpenGLVersion.cpp"
        "opengl/OpenGLVersion.h"
        "opengl/PersistentPipelineStateGL.cpp"
        "opengl/PersistentPipelineStateGL.h"
        "opengl/PhysicalDeviceGL.cpp"
        "opengl/PhysicalDeviceGL.h"
        "opengl/PipelineGL.cpp"
        "opengl/PipelineGL.h"
        "opengl/PipelineLayoutGL.cpp"
        "opengl/PipelineLayoutGL.h"
        "opengl/QuerySetGL.cpp"
        "opengl/QuerySetGL.h"
        "opengl/QueueGL.cpp"
        "opengl/QueueGL.h"
        "opengl/RenderPipelineGL.cpp"
        "opengl/RenderPipelineGL.h"
        "opengl/SamplerGL.cpp"
        "opengl/SamplerGL.h"
        "opengl/ShaderModuleGL.cpp"
        "opengl/ShaderModuleGL.h"
        "opengl/TextureGL.cpp"
        "opengl/TextureGL.h"
        "opengl/UtilsEGL.cpp"
        "opengl/UtilsEGL.h"
        "opengl/UtilsGL.cpp"
        "opengl/UtilsGL.h"
        "opengl/opengl_platform.h"
    )

    target_include_directories(dawn_native PRIVATE ${DAWN_KHRONOS_DIR})
    target_link_libraries(dawn_native PRIVATE dawn_khronos_platform)
endif()

if (DAWN_ENABLE_SWIFTSHADER)
    target_compile_definitions(dawn_native PRIVATE "DAWN_ENABLE_SWIFTSHADER")
endif()

if (DAWN_ENABLE_VULKAN)
    target_sources(dawn_native PRIVATE
       INTERFACE
        "${DAWN_INCLUDE_DIR}/dawn/native/VulkanBackend.h"
      PRIVATE
        "vulkan/BackendVk.cpp"
        "vulkan/BackendVk.h"
        "vulkan/BindGroupLayoutVk.cpp"
        "vulkan/BindGroupLayoutVk.h"
        "vulkan/BindGroupVk.cpp"
        "vulkan/BindGroupVk.h"
        "vulkan/BufferVk.cpp"
        "vulkan/BufferVk.h"
        "vulkan/CommandBufferVk.cpp"
        "vulkan/CommandBufferVk.h"
        "vulkan/CommandRecordingContext.h"
        "vulkan/ComputePipelineVk.cpp"
        "vulkan/ComputePipelineVk.h"
        "vulkan/DescriptorSetAllocation.h"
        "vulkan/DescriptorSetAllocator.cpp"
        "vulkan/DescriptorSetAllocator.h"
        "vulkan/DeviceVk.cpp"
        "vulkan/DeviceVk.h"
        "vulkan/ExternalHandle.h"
        "vulkan/FencedDeleter.cpp"
        "vulkan/FencedDeleter.h"
        "vulkan/Forward.h"
        "vulkan/PhysicalDeviceVk.cpp"
        "vulkan/PhysicalDeviceVk.h"
        "vulkan/PipelineCacheVk.cpp"
        "vulkan/PipelineCacheVk.h"
        "vulkan/PipelineLayoutVk.cpp"
        "vulkan/PipelineLayoutVk.h"
        "vulkan/QuerySetVk.cpp"
        "vulkan/QuerySetVk.h"
        "vulkan/QueueVk.cpp"
        "vulkan/QueueVk.h"
        "vulkan/RenderPassCache.cpp"
        "vulkan/RenderPassCache.h"
        "vulkan/RenderPipelineVk.cpp"
        "vulkan/RenderPipelineVk.h"
        "vulkan/ResourceHeapVk.cpp"
        "vulkan/ResourceHeapVk.h"
        "vulkan/ResourceMemoryAllocatorVk.cpp"
        "vulkan/ResourceMemoryAllocatorVk.h"
        "vulkan/SamplerVk.cpp"
        "vulkan/SamplerVk.h"
        "vulkan/ShaderModuleVk.cpp"
        "vulkan/ShaderModuleVk.h"
        "vulkan/StreamImplVk.cpp"
        "vulkan/SwapChainVk.cpp"
        "vulkan/SwapChainVk.h"
        "vulkan/TextureVk.cpp"
        "vulkan/TextureVk.h"
        "vulkan/UtilsVulkan.cpp"
        "vulkan/UtilsVulkan.h"
        "vulkan/VulkanError.cpp"
        "vulkan/VulkanError.h"
        "vulkan/VulkanExtensions.cpp"
        "vulkan/VulkanExtensions.h"
        "vulkan/VulkanFunctions.cpp"
        "vulkan/VulkanFunctions.h"
        "vulkan/VulkanInfo.cpp"
        "vulkan/VulkanInfo.h"
        "vulkan/external_memory/MemoryImportParams.h"
        "vulkan/external_memory/MemoryService.cpp"
        "vulkan/external_memory/MemoryService.h"
        "vulkan/external_memory/MemoryServiceImplementation.cpp"
        "vulkan/external_memory/MemoryServiceImplementation.h"
        "vulkan/external_semaphore/SemaphoreService.cpp"
        "vulkan/external_semaphore/SemaphoreService.h"
        "vulkan/external_semaphore/SemaphoreServiceImplementation.cpp"
        "vulkan/external_semaphore/SemaphoreServiceImplementation.h"
    )

    target_link_libraries(dawn_native PUBLIC Vulkan-Headers)
    target_include_directories(dawn_native PRIVATE ${DAWN_VULKAN_UTILITY_LIBRARIES_DIR}/include)

    if (ANDROID)
        target_sources(dawn_native PRIVATE
            "vulkan/external_memory/MemoryServiceImplementationAHardwareBuffer.cpp"
            "vulkan/external_memory/MemoryServiceImplementationAHardwareBuffer.h"
            "vulkan/external_semaphore/SemaphoreServiceImplementationFD.cpp"
            "vulkan/external_semaphore/SemaphoreServiceImplementationFD.h"
        )
    elseif (UNIX AND NOT APPLE)
        target_sources(dawn_native PRIVATE
            "vulkan/external_memory/MemoryServiceImplementationDmaBuf.cpp"
            "vulkan/external_memory/MemoryServiceImplementationDmaBuf.h"
            "vulkan/external_memory/MemoryServiceImplementationOpaqueFD.cpp"
            "vulkan/external_memory/MemoryServiceImplementationOpaqueFD.h"
            "vulkan/external_semaphore/SemaphoreServiceImplementationFD.cpp"
            "vulkan/external_semaphore/SemaphoreServiceImplementationFD.h"
        )
    endif()
endif()

# TODO how to do the component build in CMake?
target_sources(dawn_native PRIVATE "DawnNative.cpp")
if (DAWN_ENABLE_D3D11 OR DAWN_ENABLE_D3D12)
    target_sources(dawn_native PRIVATE "d3d/D3DBackend.cpp")
endif()
if (DAWN_ENABLE_D3D11)
    target_sources(dawn_native PRIVATE "d3d11/D3D11Backend.cpp")
endif()
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

DawnJSONGenerator(
    TARGET "webgpu_dawn_native_proc"
    PRINT_NAME "Dawn native WebGPU procs"
    RESULT_VARIABLE "WEBGPU_DAWN_NATIVE_PROC_GEN"
)

add_library(webgpu_dawn ${DAWN_PLACEHOLDER_FILE})
common_compile_options(webgpu_dawn)
target_link_libraries(webgpu_dawn PRIVATE dawn_native)
target_link_libraries(webgpu_dawn PUBLIC dawn_headers)
target_compile_definitions(webgpu_dawn PRIVATE "WGPU_IMPLEMENTATION")
if(BUILD_SHARED_LIBS)
    target_compile_definitions(webgpu_dawn PRIVATE "WGPU_SHARED_LIBRARY")
endif()
target_sources(webgpu_dawn PRIVATE ${WEBGPU_DAWN_NATIVE_PROC_GEN})

install_if_enabled(dawn_native)
install_if_enabled(webgpu_dawn)
