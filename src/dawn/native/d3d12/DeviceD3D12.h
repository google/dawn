// Copyright 2017 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_DAWN_NATIVE_D3D12_DEVICED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_DEVICED3D12_H_

#include <memory>
#include <vector>

#include "dawn/common/SerialQueue.h"
#include "dawn/native/Device.h"
#include "dawn/native/d3d12/CommandRecordingContext.h"
#include "dawn/native/d3d12/D3D12Info.h"
#include "dawn/native/d3d12/Forward.h"
#include "dawn/native/d3d12/TextureD3D12.h"

namespace dawn::native::d3d12 {

class CommandAllocatorManager;
struct ExternalImageDescriptorDXGISharedHandle;
class ExternalImageDXGIImpl;
class PlatformFunctions;
class ResidencyManager;
class ResourceAllocatorManager;
class SamplerHeapCache;
class ShaderVisibleDescriptorAllocator;
class StagingDescriptorAllocator;

#define ASSERT_SUCCESS(hr)            \
    do {                              \
        HRESULT succeeded = hr;       \
        ASSERT(SUCCEEDED(succeeded)); \
    } while (0)

// Definition of backend types
class Device final : public DeviceBase {
  public:
    static ResultOrError<Ref<Device>> Create(Adapter* adapter,
                                             const DeviceDescriptor* descriptor,
                                             const TogglesState& deviceToggles);
    ~Device() override;

    MaybeError Initialize(const DeviceDescriptor* descriptor);

    ResultOrError<Ref<CommandBufferBase>> CreateCommandBuffer(
        CommandEncoder* encoder,
        const CommandBufferDescriptor* descriptor) override;

    MaybeError TickImpl() override;

    ID3D12Device* GetD3D12Device() const;
    ComPtr<ID3D12CommandQueue> GetCommandQueue() const;
    ID3D12SharingContract* GetSharingContract() const;
    HANDLE GetFenceHandle() const;

    ComPtr<ID3D12CommandSignature> GetDispatchIndirectSignature() const;
    ComPtr<ID3D12CommandSignature> GetDrawIndirectSignature() const;
    ComPtr<ID3D12CommandSignature> GetDrawIndexedIndirectSignature() const;

    CommandAllocatorManager* GetCommandAllocatorManager() const;
    ResidencyManager* GetResidencyManager() const;

    const PlatformFunctions* GetFunctions() const;
    ComPtr<IDXGIFactory4> GetFactory() const;
    ComPtr<IDxcLibrary> GetDxcLibrary() const;
    ComPtr<IDxcCompiler> GetDxcCompiler() const;
    ComPtr<IDxcValidator> GetDxcValidator() const;

    ResultOrError<CommandRecordingContext*> GetPendingCommandContext(
        Device::SubmitMode submitMode = Device::SubmitMode::Normal);

    MaybeError ClearBufferToZero(CommandRecordingContext* commandContext,
                                 BufferBase* destination,
                                 uint64_t destinationOffset,
                                 uint64_t size);

    const D3D12DeviceInfo& GetDeviceInfo() const;

    MaybeError NextSerial();
    MaybeError WaitForSerial(ExecutionSerial serial);

    void ReferenceUntilUnused(ComPtr<IUnknown> object);

    void ForceEventualFlushOfCommands() override;

    MaybeError ExecutePendingCommandContext();

    MaybeError CopyFromStagingToBufferImpl(BufferBase* source,
                                           uint64_t sourceOffset,
                                           BufferBase* destination,
                                           uint64_t destinationOffset,
                                           uint64_t size) override;

    void CopyFromStagingToBufferHelper(CommandRecordingContext* commandContext,
                                       BufferBase* source,
                                       uint64_t sourceOffset,
                                       BufferBase* destination,
                                       uint64_t destinationOffset,
                                       uint64_t size);

    MaybeError CopyFromStagingToTextureImpl(const BufferBase* source,
                                            const TextureDataLayout& src,
                                            const TextureCopy& dst,
                                            const Extent3D& copySizePixels) override;

    ResultOrError<ResourceHeapAllocation> AllocateMemory(
        D3D12_HEAP_TYPE heapType,
        const D3D12_RESOURCE_DESC& resourceDescriptor,
        D3D12_RESOURCE_STATES initialUsage,
        uint32_t formatBytesPerBlock,
        bool forceAllocateAsCommittedResource = false);

    void DeallocateMemory(ResourceHeapAllocation& allocation);

    ShaderVisibleDescriptorAllocator* GetViewShaderVisibleDescriptorAllocator() const;
    ShaderVisibleDescriptorAllocator* GetSamplerShaderVisibleDescriptorAllocator() const;

    // Returns nullptr when descriptor count is zero.
    StagingDescriptorAllocator* GetViewStagingDescriptorAllocator(uint32_t descriptorCount) const;

    StagingDescriptorAllocator* GetSamplerStagingDescriptorAllocator(
        uint32_t descriptorCount) const;

    SamplerHeapCache* GetSamplerHeapCache();

    StagingDescriptorAllocator* GetRenderTargetViewAllocator() const;

    StagingDescriptorAllocator* GetDepthStencilViewAllocator() const;

    std::unique_ptr<ExternalImageDXGIImpl> CreateExternalImageDXGIImpl(
        const ExternalImageDescriptorDXGISharedHandle* descriptor);

    Ref<TextureBase> CreateD3D12ExternalTexture(const TextureDescriptor* descriptor,
                                                ComPtr<ID3D12Resource> d3d12Texture,
                                                std::vector<Ref<Fence>> waitFences,
                                                Ref<D3D11on12ResourceCacheEntry> d3d11on12Resource,
                                                bool isSwapChainTexture,
                                                bool isInitialized);

    ComPtr<ID3D11On12Device> GetOrCreateD3D11on12Device();

    uint32_t GetOptimalBytesPerRowAlignment() const override;
    uint64_t GetOptimalBufferToTextureCopyOffsetAlignment() const override;

    float GetTimestampPeriodInNS() const override;

    bool ShouldDuplicateNumWorkgroupsForDispatchIndirect(
        ComputePipelineBase* computePipeline) const override;

    bool MayRequireDuplicationOfIndirectParameters() const override;

    bool ShouldDuplicateParametersForDrawIndirect(
        const RenderPipelineBase* renderPipelineBase) const override;

    uint64_t GetBufferCopyOffsetAlignmentForDepthStencil() const override;

    // Dawn APIs
    void SetLabelImpl() override;

  private:
    using DeviceBase::DeviceBase;

    ResultOrError<Ref<BindGroupBase>> CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) override;
    ResultOrError<Ref<BindGroupLayoutBase>> CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor,
        PipelineCompatibilityToken pipelineCompatibilityToken) override;
    ResultOrError<Ref<BufferBase>> CreateBufferImpl(const BufferDescriptor* descriptor) override;
    ResultOrError<Ref<PipelineLayoutBase>> CreatePipelineLayoutImpl(
        const PipelineLayoutDescriptor* descriptor) override;
    ResultOrError<Ref<QuerySetBase>> CreateQuerySetImpl(
        const QuerySetDescriptor* descriptor) override;
    ResultOrError<Ref<SamplerBase>> CreateSamplerImpl(const SamplerDescriptor* descriptor) override;
    ResultOrError<Ref<ShaderModuleBase>> CreateShaderModuleImpl(
        const ShaderModuleDescriptor* descriptor,
        ShaderModuleParseResult* parseResult,
        OwnedCompilationMessages* compilationMessages) override;
    ResultOrError<Ref<SwapChainBase>> CreateSwapChainImpl(
        const SwapChainDescriptor* descriptor) override;
    ResultOrError<Ref<NewSwapChainBase>> CreateSwapChainImpl(
        Surface* surface,
        NewSwapChainBase* previousSwapChain,
        const SwapChainDescriptor* descriptor) override;
    ResultOrError<Ref<TextureBase>> CreateTextureImpl(const TextureDescriptor* descriptor) override;
    ResultOrError<Ref<TextureViewBase>> CreateTextureViewImpl(
        TextureBase* texture,
        const TextureViewDescriptor* descriptor) override;
    Ref<ComputePipelineBase> CreateUninitializedComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) override;
    Ref<RenderPipelineBase> CreateUninitializedRenderPipelineImpl(
        const RenderPipelineDescriptor* descriptor) override;
    void InitializeComputePipelineAsyncImpl(Ref<ComputePipelineBase> computePipeline,
                                            WGPUCreateComputePipelineAsyncCallback callback,
                                            void* userdata) override;
    void InitializeRenderPipelineAsyncImpl(Ref<RenderPipelineBase> renderPipeline,
                                           WGPUCreateRenderPipelineAsyncCallback callback,
                                           void* userdata) override;

    void DestroyImpl() override;
    MaybeError WaitForIdleForDestruction() override;
    bool HasPendingCommands() const override;

    MaybeError CheckDebugLayerAndGenerateErrors();
    void AppendDebugLayerMessages(ErrorData* error) override;

    MaybeError EnsureDXCIfRequired();

    MaybeError CreateZeroBuffer();

    ComPtr<ID3D12Fence> mFence;
    HANDLE mFenceEvent = nullptr;
    HANDLE mFenceHandle = nullptr;
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override;

    ComPtr<ID3D12Device> mD3d12Device;  // Device is owned by adapter and will not be outlived.
    ComPtr<ID3D12CommandQueue> mCommandQueue;
    ComPtr<ID3D12SharingContract> mD3d12SharingContract;

    // 11on12 device corresponding to mCommandQueue
    ComPtr<ID3D11On12Device> mD3d11On12Device;

    ComPtr<ID3D12CommandSignature> mDispatchIndirectSignature;
    ComPtr<ID3D12CommandSignature> mDrawIndirectSignature;
    ComPtr<ID3D12CommandSignature> mDrawIndexedIndirectSignature;

    CommandRecordingContext mPendingCommands;

    SerialQueue<ExecutionSerial, ComPtr<IUnknown>> mUsedComObjectRefs;

    std::unique_ptr<CommandAllocatorManager> mCommandAllocatorManager;
    std::unique_ptr<ResourceAllocatorManager> mResourceAllocatorManager;
    std::unique_ptr<ResidencyManager> mResidencyManager;

    static constexpr uint32_t kMaxSamplerDescriptorsPerBindGroup = 3 * kMaxSamplersPerShaderStage;
    static constexpr uint32_t kMaxViewDescriptorsPerBindGroup =
        kMaxBindingsPerPipelineLayout - kMaxSamplerDescriptorsPerBindGroup;

    static constexpr uint32_t kNumSamplerDescriptorAllocators =
        ConstexprLog2Ceil(kMaxSamplerDescriptorsPerBindGroup) + 1;
    static constexpr uint32_t kNumViewDescriptorAllocators =
        ConstexprLog2Ceil(kMaxViewDescriptorsPerBindGroup) + 1;

    // Index corresponds to Log2Ceil(descriptorCount) where descriptorCount is in
    // the range [0, kMaxSamplerDescriptorsPerBindGroup].
    std::array<std::unique_ptr<StagingDescriptorAllocator>, kNumViewDescriptorAllocators + 1>
        mViewAllocators;

    // Index corresponds to Log2Ceil(descriptorCount) where descriptorCount is in
    // the range [0, kMaxViewDescriptorsPerBindGroup].
    std::array<std::unique_ptr<StagingDescriptorAllocator>, kNumSamplerDescriptorAllocators + 1>
        mSamplerAllocators;

    std::unique_ptr<StagingDescriptorAllocator> mRenderTargetViewAllocator;

    std::unique_ptr<StagingDescriptorAllocator> mDepthStencilViewAllocator;

    std::unique_ptr<ShaderVisibleDescriptorAllocator> mViewShaderVisibleDescriptorAllocator;

    std::unique_ptr<ShaderVisibleDescriptorAllocator> mSamplerShaderVisibleDescriptorAllocator;

    // Sampler cache needs to be destroyed before the CPU sampler allocator to ensure the final
    // release is called.
    std::unique_ptr<SamplerHeapCache> mSamplerHeapCache;

    // A buffer filled with zeros that is used to copy into other buffers when they need to be
    // cleared.
    Ref<Buffer> mZeroBuffer;

    // The number of nanoseconds required for a timestamp query to be incremented by 1
    float mTimestampPeriod = 1.0f;

    // List of external image resources opened using this device.
    LinkedList<ExternalImageDXGIImpl> mExternalImageList;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_DEVICED3D12_H_
