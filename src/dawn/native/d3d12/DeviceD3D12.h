// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_D3D12_DEVICED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_DEVICED3D12_H_

#include <memory>
#include <vector>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/SerialQueue.h"
#include "dawn/native/d3d/DeviceD3D.h"
#include "dawn/native/d3d12/CommandRecordingContext.h"
#include "dawn/native/d3d12/D3D12Info.h"
#include "dawn/native/d3d12/Forward.h"
#include "dawn/native/d3d12/TextureD3D12.h"

namespace dawn::native::d3d {
class ExternalImageDXGIImpl;
}  // namespace dawn::native::d3d

namespace dawn::native::d3d12 {

class CommandAllocatorManager;
class PlatformFunctions;
class ResidencyManager;
class ResourceAllocatorManager;
class SamplerHeapCache;
class ShaderVisibleDescriptorAllocator;
class StagingDescriptorAllocator;

#define ASSERT_SUCCESS(hr)                 \
    do {                                   \
        HRESULT succeeded = hr;            \
        DAWN_ASSERT(SUCCEEDED(succeeded)); \
    } while (0)

// Definition of backend types
class Device final : public d3d::Device {
  public:
    static ResultOrError<Ref<Device>> Create(AdapterBase* adapter,
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

    ComPtr<ID3D12CommandSignature> GetDispatchIndirectSignature() const;
    ComPtr<ID3D12CommandSignature> GetDrawIndirectSignature() const;
    ComPtr<ID3D12CommandSignature> GetDrawIndexedIndirectSignature() const;

    CommandAllocatorManager* GetCommandAllocatorManager() const;
    MutexProtected<ResidencyManager>& GetResidencyManager() const;

    const PlatformFunctions* GetFunctions() const;

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

    MutexProtected<ShaderVisibleDescriptorAllocator>& GetViewShaderVisibleDescriptorAllocator()
        const;
    MutexProtected<ShaderVisibleDescriptorAllocator>& GetSamplerShaderVisibleDescriptorAllocator()
        const;

    // Returns nullptr when descriptor count is zero.
    MutexProtected<StagingDescriptorAllocator>* GetViewStagingDescriptorAllocator(
        uint32_t descriptorCount) const;

    MutexProtected<StagingDescriptorAllocator>* GetSamplerStagingDescriptorAllocator(
        uint32_t descriptorCount) const;

    SamplerHeapCache* GetSamplerHeapCache();

    MutexProtected<StagingDescriptorAllocator>& GetRenderTargetViewAllocator() const;

    MutexProtected<StagingDescriptorAllocator>& GetDepthStencilViewAllocator() const;

    ResultOrError<Ref<d3d::Fence>> CreateFence(
        const d3d::ExternalImageDXGIFenceDescriptor* descriptor) override;
    ResultOrError<std::unique_ptr<d3d::ExternalImageDXGIImpl>> CreateExternalImageDXGIImplImpl(
        const ExternalImageDescriptor* descriptor) override;

    Ref<TextureBase> CreateD3DExternalTexture(const TextureDescriptor* descriptor,
                                              ComPtr<IUnknown> d3dTexture,
                                              std::vector<Ref<d3d::Fence>> waitFences,
                                              bool isSwapChainTexture,
                                              bool isInitialized) override;

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

    // TODO(dawn:1413) move these methods to the d3d12::Queue.
    void ForceEventualFlushOfCommands();
    bool HasPendingCommands() const;
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials();
    MaybeError WaitForIdleForDestruction();

    // Those DXC methods are needed by d3d12::ShaderModule
    ComPtr<IDxcLibrary> GetDxcLibrary() const;
    ComPtr<IDxcCompiler3> GetDxcCompiler() const;

  private:
    using Base = d3d::Device;

    Device(AdapterBase* adapter,
           const DeviceDescriptor* descriptor,
           const TogglesState& deviceToggles);

    ResultOrError<Ref<BindGroupBase>> CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) override;
    ResultOrError<Ref<BindGroupLayoutInternalBase>> CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) override;
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
        Surface* surface,
        SwapChainBase* previousSwapChain,
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

    ResultOrError<Ref<SharedTextureMemoryBase>> ImportSharedTextureMemoryImpl(
        const SharedTextureMemoryDescriptor* descriptor) override;
    ResultOrError<Ref<SharedFenceBase>> ImportSharedFenceImpl(
        const SharedFenceDescriptor* descriptor) override;

    void DestroyImpl() override;

    MaybeError CheckDebugLayerAndGenerateErrors();
    void AppendDebugLayerMessages(ErrorData* error) override;
    void AppendDeviceLostMessage(ErrorData* error) override;

    MaybeError EnsureDXCIfRequired();

    MaybeError CreateZeroBuffer();

    ComPtr<ID3D12Fence> mFence;
    HANDLE mFenceEvent = nullptr;

    ComPtr<ID3D12Device> mD3d12Device;  // Device is owned by adapter and will not be outlived.
    ComPtr<ID3D12CommandQueue> mCommandQueue;
    ComPtr<ID3D12SharingContract> mD3d12SharingContract;

    ComPtr<ID3D12CommandSignature> mDispatchIndirectSignature;
    ComPtr<ID3D12CommandSignature> mDrawIndirectSignature;
    ComPtr<ID3D12CommandSignature> mDrawIndexedIndirectSignature;

    CommandRecordingContext mPendingCommands;

    MutexProtected<SerialQueue<ExecutionSerial, ComPtr<IUnknown>>> mUsedComObjectRefs;

    std::unique_ptr<CommandAllocatorManager> mCommandAllocatorManager;
    std::unique_ptr<MutexProtected<ResourceAllocatorManager>> mResourceAllocatorManager;
    std::unique_ptr<MutexProtected<ResidencyManager>> mResidencyManager;

    static constexpr uint32_t kMaxSamplerDescriptorsPerBindGroup = 3 * kMaxSamplersPerShaderStage;
    static constexpr uint32_t kMaxViewDescriptorsPerBindGroup =
        kMaxBindingsPerPipelineLayout - kMaxSamplerDescriptorsPerBindGroup;

    static constexpr uint32_t kNumSamplerDescriptorAllocators =
        ConstexprLog2Ceil(kMaxSamplerDescriptorsPerBindGroup) + 1;
    static constexpr uint32_t kNumViewDescriptorAllocators =
        ConstexprLog2Ceil(kMaxViewDescriptorsPerBindGroup) + 1;

    // Index corresponds to Log2Ceil(descriptorCount) where descriptorCount is in
    // the range [0, kMaxSamplerDescriptorsPerBindGroup].
    std::array<std::unique_ptr<MutexProtected<StagingDescriptorAllocator>>,
               kNumViewDescriptorAllocators + 1>
        mViewAllocators;

    // Index corresponds to Log2Ceil(descriptorCount) where descriptorCount is in
    // the range [0, kMaxViewDescriptorsPerBindGroup].
    std::array<std::unique_ptr<MutexProtected<StagingDescriptorAllocator>>,
               kNumSamplerDescriptorAllocators + 1>
        mSamplerAllocators;

    std::unique_ptr<MutexProtected<StagingDescriptorAllocator>> mRenderTargetViewAllocator;

    std::unique_ptr<MutexProtected<StagingDescriptorAllocator>> mDepthStencilViewAllocator;

    std::unique_ptr<MutexProtected<ShaderVisibleDescriptorAllocator>>
        mViewShaderVisibleDescriptorAllocator;

    std::unique_ptr<MutexProtected<ShaderVisibleDescriptorAllocator>>
        mSamplerShaderVisibleDescriptorAllocator;

    // Sampler cache needs to be destroyed before the CPU sampler allocator to ensure the final
    // release is called.
    std::unique_ptr<SamplerHeapCache> mSamplerHeapCache;

    // A buffer filled with zeros that is used to copy into other buffers when they need to be
    // cleared.
    Ref<Buffer> mZeroBuffer;

    // The number of nanoseconds required for a timestamp query to be incremented by 1
    float mTimestampPeriod = 1.0f;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_DEVICED3D12_H_
