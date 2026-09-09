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

################################################################################
# Generated mock WebGPU API used by dawn_unittests
################################################################################

DawnJSONGenerator(
    TARGET "mock_api"
    PRINT_NAME "Dawn mock WebGPU API"
    OUTPUT_HEADERS DAWN_MOCK_WEBGPU_GEN_HEADERS
    OUTPUT_SOURCES DAWN_MOCK_WEBGPU_GEN_SOURCES
)

################################################################################
# dawn_test_infra - Code shared by all test kinds
################################################################################

add_library(dawn_test_infra OBJECT
    "AdapterTestConfig.cpp"
    "AdapterTestConfig.h"
    "DawnTest.cpp"
    "DawnTest.h"
    "MockCallback.h"
    "ParamGenerator.h"
    "PartitionAllocSupport.cpp"
    "PartitionAllocSupport.h"
    "StringViewMatchers.h"
    "ToggleParser.cpp"
    "ToggleParser.h"
    "mocks/platform/CachingInterfaceMock.cpp"
    "mocks/platform/CachingInterfaceMock.h"
)
if (DAWN_ENABLE_VULKAN AND UNIX AND NOT ANDROID AND NOT APPLE)
    target_sources(dawn_test_infra PRIVATE "GBMUtils.h")
endif()
common_compile_options(dawn_test_infra)
target_link_libraries(dawn_test_infra PRIVATE
    dawn_crash_handler
    absl::stacktrace
    absl::symbolize
    dawn_common
    dawn_internal_config
    dawn_native
    dawn_platform
    dawn_proc
    dawn_system_utils
    dawn_test_utils
    dawn_wgpu_utils
    dawncpp
    gmock
    partition_alloc
)

if (DAWN_SUPPORTS_GLFW_FOR_WINDOWING)
    target_link_libraries(dawn_test_infra PRIVATE
        glfw
        dawn::dawn_glfw
    )
endif()

if (DAWN_ENABLE_WEBGPU_ON_WEBGPU)
    target_link_libraries(dawn_test_infra PRIVATE replay)
endif()

################################################################################
# dawn_end2end_test_objects - Tests that only use the public Dawn API
################################################################################

set(end2end_conditional_private_platform_depends)
set(end2end_sources
    "end2end/AdapterCreationTests.cpp"
    "end2end/AdapterEnumerationTests.cpp"
    "end2end/AdapterFormatCapabilitiesVkTests.cpp"
    "end2end/AdapterPropertiesD3DTests.cpp"
    "end2end/AdapterPropertiesVkTests.cpp"
    "end2end/AdapterPropertiesDrmTests.cpp"
    "end2end/ArchTierLimitsExhaustive.cpp"
    "end2end/BasicTests.cpp"
    "end2end/BindGroupTests.cpp"
    "end2end/BindingArrayTests.cpp"
    "end2end/BufferHostMappedPointerTests.cpp"
    "end2end/BufferHostMappedPointerTests.h"
    "end2end/BufferRobustnessTests.cpp"
    "end2end/BufferTests.cpp"
    "end2end/BufferViewTests.cpp"
    "end2end/BufferZeroInitTests.cpp"
    "end2end/ClipDistancesTests.cpp"
    "end2end/ClipSpaceTests.cpp"
    "end2end/ColorStateTests.cpp"
    "end2end/CommandEncoderTests.cpp"
    "end2end/CompressedTextureFormatTests.cpp"
    "end2end/ComputeCopyStorageBufferTests.cpp"
    "end2end/ComputeDispatchTests.cpp"
    "end2end/ComputeFlowControlTests.cpp"
    "end2end/ComputeLayoutMemoryBufferTests.cpp"
    "end2end/ComputeSharedMemoryTests.cpp"
    "end2end/ComputeStorageBufferBarrierTests.cpp"
    "end2end/CopyExternalTextureForBrowserTests.cpp"
    "end2end/CopyTests.cpp"
    "end2end/CopyTextureForBrowserTests.cpp"
    "end2end/CreateDestroyTests.cpp"
    "end2end/CreatePipelineAsyncTests.cpp"
    "end2end/CullingTests.cpp"
    "end2end/DawnTestInfraTests.cpp"
    "end2end/DebugMarkerTests.cpp"
    "end2end/DepthBiasTests.cpp"
    "end2end/DepthStencilCopyTests.cpp"
    "end2end/DepthStencilLoadOpTests.cpp"
    "end2end/DepthStencilSamplingTests.cpp"
    "end2end/DepthStencilStateTests.cpp"
    "end2end/DestroyTests.cpp"
    "end2end/DeviceInitializationTests.cpp"
    "end2end/DeviceLifetimeTests.cpp"
    "end2end/DeviceLostTests.cpp"
    "end2end/DiscardBasicTests.cpp"
    "end2end/DrawIndexedIndirectTests.cpp"
    "end2end/DrawIndexedTests.cpp"
    "end2end/DrawIndirectTests.cpp"
    "end2end/DrawTests.cpp"
    "end2end/DualSourceBlendTests.cpp"
    "end2end/DynamicBufferOffsetTests.cpp"
    "end2end/EntryPointTests.cpp"
    "end2end/EventTests.cpp"
    "end2end/ExternalTextureTests.cpp"
    "end2end/FirstIndexOffsetTests.cpp"
    "end2end/FragDepthTests.cpp"
    "end2end/FramebufferFetchTests.cpp"
    "end2end/GpuMemorySynchronizationTests.cpp"
    "end2end/HistogramTests.cpp"
    "end2end/ImmediateDataBufferLengthTests.cpp"
    "end2end/ImmediateDataTests.cpp"
    "end2end/IndexFormatTests.cpp"
    "end2end/InfiniteLoopTests.cpp"
    "end2end/MaxLimitTests.cpp"
    "end2end/MemoryAllocationStressTests.cpp"
    "end2end/MemoryHeapPropertiesTests.cpp"
    "end2end/MultiDrawIndexedIndirectTests.cpp"
    "end2end/MultiDrawIndirectTests.cpp"
    "end2end/MultisampledInterpolationTests.cpp"
    "end2end/MultisampledRenderingTests.cpp"
    "end2end/MultisampledSamplingTests.cpp"
    "end2end/MultithreadTests.cpp"
    "end2end/NonzeroBufferCreationTests.cpp"
    "end2end/NonzeroTextureCreationTests.cpp"
    "end2end/OpArrayLengthTests.cpp"
    "end2end/PackUnpack4x8NormTests.cpp"
    "end2end/Packed4x8IntegerDotProductTests.cpp"
    "end2end/PipelineCachingTests.cpp"
    "end2end/PipelineLayoutTests.cpp"
    "end2end/PixelLocalStorageTests.cpp"
    "end2end/PolyfillBuiltinSimpleTests.cpp"
    "end2end/PrimitiveIndexTests.cpp"
    "end2end/PrimitiveStateTests.cpp"
    "end2end/PrimitiveTopologyTests.cpp"
    "end2end/QueryTests.cpp"
    "end2end/QueueTests.cpp"
    "end2end/QueueTimelineTests.cpp"
    "end2end/ReadOnlyDepthStencilAttachmentTests.cpp"
    "end2end/RenderAttachmentTests.cpp"
    "end2end/RenderBundleTests.cpp"
    "end2end/RenderPassLoadOpTests.cpp"
    "end2end/RenderPassTests.cpp"
    "end2end/RequiredBufferSizeInCopyTests.cpp"
    "end2end/ResourceTableTests.cpp"
    "end2end/SamplerFilterAnisotropicTests.cpp"
    "end2end/SamplerTests.cpp"
    "end2end/ScissorTests.cpp"
    "end2end/SelectNonShortCircuiting.cpp"
    "end2end/ShaderAtomicTests.cpp"
    "end2end/ShaderBuiltinPartialConstArgsErrorTests.cpp"
    "end2end/ShaderF16Tests.cpp"
    "end2end/ShaderIOPolyfillTests.cpp"
    "end2end/ShaderModuleCachingTests.cpp"
    "end2end/ShaderPrintTests.cpp"
    "end2end/ShaderTests.cpp"
    "end2end/ShaderValidationTests.cpp"
    "end2end/StorageTextureTests.cpp"
    "end2end/SubgroupMatrixTests.cpp"
    "end2end/SubgroupsTests.cpp"
    "end2end/SubresourceRenderAttachmentTests.cpp"
    "end2end/SupportedFeatureArch.cpp"
    "end2end/TexelBufferTests.cpp"
    "end2end/Texture3DTests.cpp"
    "end2end/TextureComponentSwizzleTests.cpp"
    "end2end/TextureCorruptionTests.cpp"
    "end2end/TextureFormatTests.cpp"
    "end2end/TextureFormatsTier1Tests.cpp"
    "end2end/TextureSampleCompare2DTests.cpp"
    "end2end/TextureShaderBuiltinTests.cpp"
    "end2end/TextureSubresourceTests.cpp"
    "end2end/TextureViewTests.cpp"
    "end2end/TextureZeroInitTests.cpp"
    "end2end/VertexFormatTests.cpp"
    "end2end/VertexOnlyRenderPipelineTests.cpp"
    "end2end/VertexStateTests.cpp"
    "end2end/ViewportOrientationTests.cpp"
    "end2end/ViewportTests.cpp"
)

if (NOT IS_DEBUG_BUILD)
    list(APPEND end2end_sources
        "end2end/RuntimeTimeoutCompute.cpp"
    )
endif()

if (DAWN_ENABLE_D3D11 OR DAWN_ENABLE_D3D12)
    list(APPEND end2end_conditional_private_platform_depends
        d3d11.lib
        d3d12.lib
        dxgi.lib
    )
    list(APPEND end2end_sources
        "end2end/VideoViewsTests_win.cpp"
    )
endif()

if (DAWN_ENABLE_D3D12)
    list(APPEND end2end_sources
        "end2end/D3D12CachingTests.cpp"
    )
endif()

if (APPLE)
    if (DAWN_ENABLE_METAL)
        list(APPEND end2end_sources
            "end2end/VideoViewsTests_mac.cpp"
        )
    endif()
    list(APPEND end2end_sources
        "end2end/BufferHostMappedPointerTests_apple.mm"
    )

    list(APPEND conditional_private_platform_depends
        "-framework IOSurface"
    )
endif()

if (UNIX)
    list(APPEND end2end_sources
        "end2end/BufferHostMappedPointerTests_posix.cpp"
    )
endif()

if (WIN32)
    list(APPEND end2end_sources
        "end2end/BufferHostMappedPointerTests_win.cpp"
    )
endif()

if (DAWN_SUPPORTS_GLFW_FOR_WINDOWING)
    list(APPEND end2end_conditional_private_platform_depends
        glfw
        dawn::dawn_glfw
    )
    if (DAWN_USE_X11)
        find_package(X11 REQUIRED)
        list(APPEND end2end_conditional_private_platform_depends
            ${X11_LIBRARIES}
        )
    endif()

    list(APPEND end2end_sources
        "end2end/SurfaceConfigurationValidationTests.cpp"
        "end2end/SurfaceTests.cpp"
        "end2end/WindowSurfaceTests.cpp"
    )
endif()

if (DAWN_ENABLE_VULKAN)
    list(APPEND end2end_sources
        "end2end/AllocatorMemoryInstrumentationTests.cpp"
    )
endif()

if (DAWN_ENABLE_WEBGPU_ON_WEBGPU)
    list(APPEND end2end_sources
        "end2end/CaptureAndReplayTests.cpp"
    )
endif()

if (DAWN_ENABLE_D3D11 OR DAWN_ENABLE_D3D12 OR DAWN_ENABLE_METAL OR (DAWN_ENABLE_VULKAN AND NOT ANDROID))
    list(APPEND end2end_sources
        "end2end/VideoViewsTests.cpp"
        "end2end/VideoViewsTests.h"
    )
endif()

if (DAWN_ENABLE_VULKAN AND (UNIX AND NOT ANDROID AND NOT APPLE))
    list(APPEND end2end_conditional_private_platform_depends
        "gbm"
    )
    list(APPEND end2end_sources
        "end2end/VideoViewsTests_gbm.cpp"
    )
endif()

if (DAWN_ENABLE_OPENGLES)
    list(APPEND end2end_sources
        "end2end/GLExplicitExtensionsTests.cpp"
    )
endif()

add_library(dawn_end2end_test_objects OBJECT ${end2end_sources})
common_compile_options(dawn_end2end_test_objects)
target_link_libraries(dawn_end2end_test_objects PRIVATE
    absl_strings
    dawn_common
    dawn_internal_config
    dawn_native
    dawn_platform
    dawn_proc
    dawn_shared_utils_chromium_test_compat
    dawn_system_utils
    dawn_test_infra
    dawn_test_utils
    dawn_wgpu_utils
    dawncpp
    gmock

    ${end2end_conditional_private_platform_depends}
)

################################################################################
# dawn_whitebox_test_objects - Tests that can use dawn::native internals
################################################################################

set(whitebox_conditional_private_platform_depends)
set(whitebox_sources
    "white_box/ImmediateOffsetTests.cpp"
    "white_box/ShaderModuleTests.cpp"
    "white_box/SharedBufferMemoryTests.cpp"
    "white_box/SharedBufferMemoryTests.h"
    "white_box/SharedTextureMemoryTests.cpp"
    "white_box/SharedTextureMemoryTests.h"
    "white_box/WaitListEventTests.cpp"
)

if (APPLE)
    list(APPEND whitebox_sources
        "white_box/SharedTextureMemoryTests_apple.mm"
    )
endif()

if (DAWN_ENABLE_D3D11)
    list(APPEND whitebox_sources
        "white_box/D3D11BufferTests.cpp"
        "white_box/D3D11MultithreadTests.cpp"
        "white_box/D3D11ShaderCachingTests.cpp"
        "white_box/SharedTextureMemoryTests_win.cpp"
    )
endif()

if (DAWN_ENABLE_D3D12)
    list(APPEND whitebox_sources
        "white_box/D3D12DescriptorHeapTests.cpp"
        "white_box/D3D12ResidencyTests.cpp"
        "white_box/D3D12ResourceHeapTests.cpp"
        "white_box/GPUTimestampCalibrationTests.cpp"
        "white_box/GPUTimestampCalibrationTests.h"
        "white_box/GPUTimestampCalibrationTests_D3D12.cpp"
        "white_box/SharedBufferMemoryTests_win.cpp"
        "white_box/SharedTextureMemoryTests_win_d3d12.cpp"
    )
endif()

if (DAWN_ENABLE_METAL)
    list(APPEND whitebox_sources
      "white_box/GPUTimestampCalibrationTests.cpp"
      "white_box/GPUTimestampCalibrationTests.h"
      "white_box/GPUTimestampCalibrationTests_Metal.mm"
      "white_box/MetalAutoreleasePoolTests.mm"
      "white_box/MetalBackendTests.cpp"
    )
endif()

if (DAWN_ENABLE_OPENGLES)
    list(APPEND whitebox_conditional_private_platform_depends dawn_khronos_platform)
    list(APPEND whitebox_sources
        "white_box/EGLImageWrappingTests.cpp"
        "white_box/GLFramebufferCompletenessTests.cpp"
        "white_box/GLTextureWrappingTests.cpp"
    )
endif()

if (DAWN_ENABLE_VULKAN)
    list(APPEND whitebox_conditional_private_platform_depends
        Vulkan::Headers
    )
    list(APPEND whitebox_sources
        "white_box/BufferAllocatedSizeTests.cpp"
        "white_box/InternalResolveAttachmentSampleTypeTests.cpp"
        "white_box/InternalResourceUsageTests.cpp"
        "white_box/InternalStorageBufferBindingTests.cpp"
        "white_box/QueryInternalShaderTests.cpp"
    )

    if (UNIX AND NOT ANDROID AND NOT APPLE)
        list(APPEND whitebox_sources
            "white_box/SharedTextureMemoryTests_dmabuf.cpp"
            "white_box/SharedTextureMemoryTests_opaquefd.cpp"
            "white_box/VulkanImageWrappingTests.cpp"
            "white_box/VulkanImageWrappingTests.h"
            "white_box/VulkanImageWrappingTests_DmaBuf.cpp"
            "white_box/VulkanImageWrappingTests_DmaBuf.h"
            "white_box/VulkanImageWrappingTests_OpaqueFD.cpp"
            "white_box/VulkanImageWrappingTests_OpaqueFD.h"
        )
    endif()

    if (ANDROID)
        list(APPEND whitebox_sources
            "white_box/SharedTextureMemoryTests_android.cpp"
            "white_box/SharedTextureMemoryYCbCrAndroidTests.cpp"
        )
    endif()

    if (DAWN_ENABLE_ERROR_INJECTION)
        list(APPEND whitebox_sources
            "white_box/VulkanErrorInjectorTests.cpp"
        )
    endif()
endif()

add_library(dawn_whitebox_test_objects OBJECT ${whitebox_sources})
common_compile_options(dawn_whitebox_test_objects)
target_link_libraries(dawn_whitebox_test_objects PRIVATE
    dawn_common
    dawn_internal_config
    dawn_native
    dawn_native_objects
    dawn_platform
    dawn_proc
    dawn_shared_utils_chromium_test_compat
    dawn_system_utils
    dawn_test_infra
    dawn_test_utils
    dawn_wgpu_utils
    dawncpp
    gmock

    ${whitebox_conditional_private_platform_depends}
)

################################################################################
# dawn_end2end_tests
################################################################################

add_executable(dawn_end2end_tests "End2EndTestsMain.cpp")
common_compile_options(dawn_end2end_tests)
target_link_libraries(dawn_end2end_tests PRIVATE
    dawn_internal_config
    dawn_end2end_test_objects
    dawn_whitebox_test_objects
    dawn_test_infra
    gmock
    partition_alloc
)

################################################################################
# dawn_native_unittest_mocks - Native implementation mocks used by unit tests
################################################################################

set(native_unittest_mock_sources
    ${DAWN_MOCK_WEBGPU_GEN_HEADERS}
    ${DAWN_MOCK_WEBGPU_GEN_SOURCES}
    "unittests/native/mocks/BindGroupLayoutMock.cpp"
    "unittests/native/mocks/BindGroupLayoutMock.h"
    "unittests/native/mocks/BindGroupMock.cpp"
    "unittests/native/mocks/BindGroupMock.h"
    "unittests/native/mocks/BufferMock.cpp"
    "unittests/native/mocks/BufferMock.h"
    "unittests/native/mocks/CommandBufferMock.cpp"
    "unittests/native/mocks/CommandBufferMock.h"
    "unittests/native/mocks/ComputePipelineMock.cpp"
    "unittests/native/mocks/ComputePipelineMock.h"
    "unittests/native/mocks/DawnMockTest.cpp"
    "unittests/native/mocks/DawnMockTest.h"
    "unittests/native/mocks/DeviceMock.cpp"
    "unittests/native/mocks/DeviceMock.h"
    "unittests/native/mocks/ExternalTextureMock.cpp"
    "unittests/native/mocks/ExternalTextureMock.h"
    "unittests/native/mocks/PipelineLayoutMock.cpp"
    "unittests/native/mocks/PipelineLayoutMock.h"
    "unittests/native/mocks/QuerySetMock.cpp"
    "unittests/native/mocks/QuerySetMock.h"
    "unittests/native/mocks/QueueMock.cpp"
    "unittests/native/mocks/QueueMock.h"
    "unittests/native/mocks/RenderPipelineMock.cpp"
    "unittests/native/mocks/RenderPipelineMock.h"
    "unittests/native/mocks/SamplerMock.cpp"
    "unittests/native/mocks/SamplerMock.h"
    "unittests/native/mocks/ShaderModuleMock.cpp"
    "unittests/native/mocks/ShaderModuleMock.h"
    "unittests/native/mocks/TextureMock.cpp"
    "unittests/native/mocks/TextureMock.h"
)

add_library(dawn_native_unittest_mocks OBJECT ${native_unittest_mock_sources})
common_compile_options(dawn_native_unittest_mocks)
if (MSVC)
    # The generated mock implementation exceeds the default COFF section limit.
    target_compile_options(dawn_native_unittest_mocks PRIVATE "/bigobj")
endif()
target_include_directories(dawn_native_unittest_mocks PRIVATE "${DAWN_BUILD_GEN_DIR}")
target_link_libraries(dawn_native_unittest_mocks PRIVATE
    dawn_common
    dawn_internal_config
    dawn_native
    dawn_native_objects
    dawn_proc
    dawn_test_infra
    dawncpp
    gmock
)

################################################################################
# dawn_unittest_objects
################################################################################

set(unittest_sources
    "${Dawn_SOURCE_DIR}/src/dawn/wire/client/ClientMemoryTransferService_mock.cpp"
    "${Dawn_SOURCE_DIR}/src/dawn/wire/client/ClientMemoryTransferService_mock.h"
    "${Dawn_SOURCE_DIR}/src/dawn/wire/server/ServerMemoryTransferService_mock.cpp"
    "${Dawn_SOURCE_DIR}/src/dawn/wire/server/ServerMemoryTransferService_mock.h"
    "${Dawn_SOURCE_DIR}/src/utils/assert_test.cc"
    "${Dawn_SOURCE_DIR}/src/utils/heap_array_test.cc"
    "${Dawn_SOURCE_DIR}/src/utils/numeric_test.cc"
    "${Dawn_SOURCE_DIR}/src/utils/span_tests.cc"
    "${Dawn_SOURCE_DIR}/src/utils/typed_integer_tests.cc"
    "${Dawn_SOURCE_DIR}/src/utils/underlying_type_tests.cc"
    "DawnNativeTest.cpp"
    "DawnNativeTest.h"
    "unittests/AlgebraTests.cpp"
    "unittests/AsyncTaskTests.cpp"
    "unittests/BitSetRangeIteratorTests.cpp"
    "unittests/BuddyAllocatorTests.cpp"
    "unittests/BuddyMemoryAllocatorTests.cpp"
    "unittests/ChainUtilsTests.cpp"
    "unittests/CommandAllocatorTests.cpp"
    "unittests/CommandLineParserTests.cpp"
    "unittests/ContentLessObjectCacheTests.cpp"
    "unittests/CppAPITests.cpp"
    "unittests/DefaultTests.cpp"
    "unittests/EnumClassBitmasksTests.cpp"
    "unittests/EnumMaskIteratorTests.cpp"
    "unittests/EnumeratorTests.cpp"
    "unittests/ErrorTests.cpp"
    "unittests/FeatureTests.cpp"
    "unittests/GPUInfoTests.cpp"
    "unittests/GetProcAddressTests.cpp"
    "unittests/ITypArrayTests.cpp"
    "unittests/ITypBitsetTests.cpp"
    "unittests/ITypStackVecTests.cpp"
    "unittests/ITypVectorTests.cpp"
    "unittests/LRUCacheTests.cpp"
    "unittests/LinkedListTests.cpp"
    "unittests/MathTests.cpp"
    "unittests/MemoryBlockAllocatorTests.cpp"
    "unittests/MutexProtectedTests.cpp"
    "unittests/MutexTests.cpp"
    "unittests/NumericTests.cpp"
    "unittests/ObjectBaseTests.cpp"
    "unittests/PerStageTests.cpp"
    "unittests/PerThreadProcTests.cpp"
    "unittests/PlacementAllocatedTests.cpp"
    "unittests/RangeTests.cpp"
    "unittests/RefBaseTests.cpp"
    "unittests/RefCountedTests.cpp"
    "unittests/ResultTests.cpp"
    "unittests/RingBufferAllocatorTests.cpp"
    "unittests/SerialMapTests.cpp"
    "unittests/SerialQueueTests.cpp"
    "unittests/Sha3Tests.cpp"
    "unittests/SlabAllocatorTests.cpp"
    "unittests/StringsTests.cpp"
    "unittests/SubresourceStorageTests.cpp"
    "unittests/SystemUtilsTests.cpp"
    "unittests/ToBackendTests.cpp"
    "unittests/ToggleTests.cpp"
    "unittests/UnicodeTests.cpp"
    "unittests/WeakRefTests.cpp"
    "unittests/WorkerTaskPoolTests.cpp"
    "unittests/native/AllowedErrorTests.cpp"
    "unittests/native/BlobCacheTests.cpp"
    "unittests/native/BlobTests.cpp"
    "unittests/native/BufferBaseTests.cpp"
    "unittests/native/CacheRequestTests.cpp"
    "unittests/native/CommandBufferEncodingTests.cpp"
    "unittests/native/CreatePipelineAsyncEventTests.cpp"
    "unittests/native/DestroyObjectTests.cpp"
    "unittests/native/DeviceAsyncTaskTests.cpp"
    "unittests/native/DeviceCreationTests.cpp"
    "unittests/native/DeviceMutexTests.cpp"
    "unittests/native/ErrorMonadTests.cpp"
    "unittests/native/ImmediatesTrackerTests.cpp"
    "unittests/native/LimitsTests.cpp"
    "unittests/native/MemoryInstrumentationTests.cpp"
    "unittests/native/ObjectContentHasherTests.cpp"
    "unittests/native/ShaderModuleTests.cpp"
    "unittests/native/SkipValidationTests.cpp"
    "unittests/native/StreamTests.cpp"
    "unittests/validation/BindGroupValidationTests.cpp"
    "unittests/validation/BufferValidationTests.cpp"
    "unittests/validation/CommandBufferValidationTests.cpp"
    "unittests/validation/CompatValidationTests.cpp"
    "unittests/validation/ComputeIndirectValidationTests.cpp"
    "unittests/validation/ComputeValidationTests.cpp"
    "unittests/validation/CopyCommandsValidationTests.cpp"
    "unittests/validation/CopyTextureForBrowserTests.cpp"
    "unittests/validation/DebugMarkerValidationTests.cpp"
    "unittests/validation/DeviceValidationTests.cpp"
    "unittests/validation/DrawIndirectValidationTests.cpp"
    "unittests/validation/DrawVertexAndIndexBufferOOBValidationTests.cpp"
    "unittests/validation/DynamicStateCommandValidationTests.cpp"
    "unittests/validation/ErrorScopeValidationTests.cpp"
    "unittests/validation/ExternalTextureTests.cpp"
    "unittests/validation/GetBindGroupLayoutValidationTests.cpp"
    "unittests/validation/ImmediateDataTests.cpp"
    "unittests/validation/IndexBufferValidationTests.cpp"
    "unittests/validation/InternalUsageValidationTests.cpp"
    "unittests/validation/LabelTests.cpp"
    "unittests/validation/MinimumBufferSizeValidationTests.cpp"
    "unittests/validation/MultiDrawIndirectValidationTests.cpp"
    "unittests/validation/MultipleDeviceTests.cpp"
    "unittests/validation/ObjectCachingTests.cpp"
    "unittests/validation/OverridableConstantsValidationTests.cpp"
    "unittests/validation/PipelineAndPassCompatibilityTests.cpp"
    "unittests/validation/PixelLocalStorageTests.cpp"
    "unittests/validation/QueryValidationTests.cpp"
    "unittests/validation/QueueOnSubmittedWorkDoneValidationTests.cpp"
    "unittests/validation/QueueSubmitValidationTests.cpp"
    "unittests/validation/QueueWriteBufferValidationTests.cpp"
    "unittests/validation/QueueWriteTextureValidationTests.cpp"
    "unittests/validation/RenderBundleValidationTests.cpp"
    "unittests/validation/RenderPassDescriptorValidationTests.cpp"
    "unittests/validation/RenderPipelineValidationTests.cpp"
    "unittests/validation/ResourceTableValidationTests.cpp"
    "unittests/validation/ResourceUsageTrackingTests.cpp"
    "unittests/validation/SamplerValidationTests.cpp"
    "unittests/validation/ShaderModuleValidationTests.cpp"
    "unittests/validation/StorageTextureValidationTests.cpp"
    "unittests/validation/TexelBufferValidationTests.cpp"
    "unittests/validation/TextureSubresourceTests.cpp"
    "unittests/validation/TextureValidationTests.cpp"
    "unittests/validation/TextureViewValidationTests.cpp"
    "unittests/validation/ToggleValidationTests.cpp"
    "unittests/validation/UnsafeAPIValidationTests.cpp"
    "unittests/validation/ValidationTest.cpp"
    "unittests/validation/ValidationTest.h"
    "unittests/validation/VertexBufferValidationTests.cpp"
    "unittests/validation/VertexStateValidationTests.cpp"
    "unittests/validation/WGSLFeatureValidationTests.cpp"
    "unittests/validation/WritableBufferBindingAliasingValidationTests.cpp"
    "unittests/validation/WritableTextureBindingAliasingValidationTests.cpp"
    "unittests/validation/WriteBufferTests.cpp"
    "unittests/wire/WireAdapterTests.cpp"
    "unittests/wire/WireArgumentTests.cpp"
    "unittests/wire/WireBasicTests.cpp"
    "unittests/wire/WireBufferMappingTests.cpp"
    "unittests/wire/WireConfusionDeathTests.cpp"
    "unittests/wire/WireCreatePipelineAsyncTests.cpp"
    "unittests/wire/WireDeviceLifetimeTests.cpp"
    "unittests/wire/WireDisconnectTests.cpp"
    "unittests/wire/WireErrorCallbackTests.cpp"
    "unittests/wire/WireExtensionTests.cpp"
    "unittests/wire/WireFutureTest.cpp"
    "unittests/wire/WireFutureTest.h"
    "unittests/wire/WireInjectBufferTests.cpp"
    "unittests/wire/WireInjectInstanceTests.cpp"
    "unittests/wire/WireInjectSurfaceTests.cpp"
    "unittests/wire/WireInjectTextureTests.cpp"
    "unittests/wire/WireInstanceTests.cpp"
    "unittests/wire/WireMemoryTransferServiceTests.cpp"
    "unittests/wire/WireOptionalTests.cpp"
    "unittests/wire/WireQueueTests.cpp"
    "unittests/wire/WireShaderModuleTests.cpp"
    "unittests/wire/WireSpecificCommandTests.cpp"
    "unittests/wire/WireTest.cpp"
    "unittests/wire/WireTest.h"
)

set(unittest_conditional_private_depends)

# RawPtrTests.cpp is intentionally omitted. GN enables it only when the real
# PartitionAlloc dangling-pointer checks are configured. CMake currently
# provides only Dawn's header-only no-op raw_ptr compatibility target.

if (WIN32)
    list(APPEND unittest_sources
        "unittests/WindowsUtilsTests.cpp"
        "unittests/wire/WireInlineSharedMemoryManagerTests.cpp"
    )
endif()

if (DAWN_ENABLE_D3D12)
    list(APPEND unittest_sources
        "unittests/d3d12/CopySplitTests.cpp"
    )
endif()

if (DAWN_ENABLE_WEBGPU_ON_WEBGPU)
    list(APPEND unittest_sources
        "unittests/replay/CaptureWalkerTests.cpp"
    )
    list(APPEND unittest_conditional_private_depends replay)
endif()

if (TINT_BUILD_SPV_READER)
    list(APPEND unittest_conditional_private_depends SPIRV-Tools-opt)
endif()

if (DAWN_ENABLE_VULKAN)
    # Both tests require Vulkan headers. MemoryTypeSelectorTests also depends on
    # implementation code compiled only when the Vulkan backend is enabled.
    list(APPEND unittest_sources
        "unittests/validation/YCbCrValidationTests.cpp"
        "unittests/vulkan/MemoryTypeSelectorTests.cpp"
    )
    list(APPEND unittest_conditional_private_depends Vulkan::Headers)

    if (UNIX AND NOT ANDROID AND NOT APPLE)
        list(APPEND unittest_sources
            "unittests/DRMUtilsTests.cpp"
        )
    endif()
endif()

add_library(dawn_unittest_objects OBJECT ${unittest_sources})
common_compile_options(dawn_unittest_objects)
# Wire unit tests include the generated mock WebGPU header.
add_dependencies(dawn_unittest_objects dawn_native_unittest_mocks)
if (MSVC)
    # Some unit test translation units exceed the default COFF section limit.
    target_compile_options(dawn_unittest_objects PRIVATE "/bigobj")
endif()
target_include_directories(dawn_unittest_objects PRIVATE "${DAWN_BUILD_GEN_DIR}")
target_link_libraries(dawn_unittest_objects PRIVATE
    absl::flat_hash_map
    absl::str_format
    absl_strings
    dawn_common
    dawn_crash_handler
    dawn_internal_config
    dawn_native
    dawn_native_objects
    dawn_platform
    dawn_proc
    dawn_shared_utils
    dawn_shared_utils_chromium_test_compat
    dawn_system_utils
    dawn_test_infra
    dawn_test_utils
    dawn_wgpu_utils
    dawn_wire
    dawncpp
    gmock
    libtint
    partition_alloc

    ${unittest_conditional_private_depends}
)

################################################################################
# dawn_unittests
################################################################################

add_executable(dawn_unittests "UnittestsMain.cpp")
common_compile_options(dawn_unittests)
target_link_libraries(dawn_unittests PRIVATE
    absl::flat_hash_map
    dawn_internal_config
    dawn_native_unittest_mocks
    dawn_unittest_objects
    dawn_test_infra
    gmock
    partition_alloc
)
