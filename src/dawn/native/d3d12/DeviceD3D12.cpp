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

#include "dawn/native/d3d12/DeviceD3D12.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <utility>

#include "dawn/common/GPUInfo.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/ExternalImageDXGIImpl.h"
#include "dawn/native/d3d12/BackendD3D12.h"
#include "dawn/native/d3d12/BindGroupD3D12.h"
#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn/native/d3d12/CommandAllocatorManager.h"
#include "dawn/native/d3d12/CommandBufferD3D12.h"
#include "dawn/native/d3d12/ComputePipelineD3D12.h"
#include "dawn/native/d3d12/FenceD3D12.h"
#include "dawn/native/d3d12/PhysicalDeviceD3D12.h"
#include "dawn/native/d3d12/PipelineLayoutD3D12.h"
#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"
#include "dawn/native/d3d12/QuerySetD3D12.h"
#include "dawn/native/d3d12/QueueD3D12.h"
#include "dawn/native/d3d12/RenderPipelineD3D12.h"
#include "dawn/native/d3d12/ResidencyManagerD3D12.h"
#include "dawn/native/d3d12/ResourceAllocatorManagerD3D12.h"
#include "dawn/native/d3d12/SamplerD3D12.h"
#include "dawn/native/d3d12/SamplerHeapCacheD3D12.h"
#include "dawn/native/d3d12/ShaderModuleD3D12.h"
#include "dawn/native/d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"
#include "dawn/native/d3d12/StagingDescriptorAllocatorD3D12.h"
#include "dawn/native/d3d12/SwapChainD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d12 {

// TODO(dawn:155): Figure out these values.
static constexpr uint16_t kShaderVisibleDescriptorHeapSize = 1024;
static constexpr uint8_t kAttachmentDescriptorHeapSize = 64;

// Value may change in the future to better accomodate large clears.
static constexpr uint64_t kZeroBufferSize = 1024 * 1024 * 4;  // 4 Mb

static constexpr uint64_t kMaxDebugMessagesToPrint = 5;

// static
ResultOrError<Ref<Device>> Device::Create(AdapterBase* adapter,
                                          const DeviceDescriptor* descriptor,
                                          const TogglesState& deviceToggles) {
    Ref<Device> device = AcquireRef(new Device(adapter, descriptor, deviceToggles));
    DAWN_TRY(device->Initialize(descriptor));
    return device;
}

MaybeError Device::Initialize(const DeviceDescriptor* descriptor) {
    mD3d12Device = ToBackend(GetPhysicalDevice())->GetDevice();

    ASSERT(mD3d12Device != nullptr);

    // Create device-global objects
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    DAWN_TRY(
        CheckHRESULT(mD3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)),
                     "D3D12 create command queue"));

    if ((HasFeature(Feature::TimestampQuery) || HasFeature(Feature::TimestampQueryInsidePasses)) &&
        !IsToggleEnabled(Toggle::DisableTimestampQueryConversion)) {
        // Get GPU timestamp counter frequency (in ticks/second). This fails if the specified
        // command queue doesn't support timestamps. D3D12_COMMAND_LIST_TYPE_DIRECT queues
        // always support timestamps except where there are bugs in Windows container and vGPU
        // implementations.
        uint64_t frequency;
        DAWN_TRY(CheckHRESULT(mCommandQueue->GetTimestampFrequency(&frequency),
                              "D3D12 get timestamp frequency"));
        // Calculate the period in nanoseconds by the frequency.
        mTimestampPeriod = static_cast<float>(1e9) / frequency;
    }

    // If PIX is not attached, the QueryInterface fails. Hence, no need to check the return
    // value.
    mCommandQueue.As(&mD3d12SharingContract);

    DAWN_TRY(CheckHRESULT(mD3d12Device->CreateFence(uint64_t(GetLastSubmittedCommandSerial()),
                                                    D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&mFence)),
                          "D3D12 create fence"));

    mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    ASSERT(mFenceEvent != nullptr);

    DAWN_TRY(CheckHRESULT(mD3d12Device->CreateSharedHandle(mFence.Get(), nullptr, GENERIC_ALL,
                                                           nullptr, &mFenceHandle),
                          "D3D12 create fence handle"));
    ASSERT(mFenceHandle != nullptr);

    // Initialize backend services
    mCommandAllocatorManager = std::make_unique<CommandAllocatorManager>(this);

    // Zero sized allocator is never requested and does not need to exist.
    for (uint32_t countIndex = 0; countIndex < kNumViewDescriptorAllocators; countIndex++) {
        mViewAllocators[countIndex + 1] = std::make_unique<StagingDescriptorAllocator>(
            this, 1u << countIndex, kShaderVisibleDescriptorHeapSize,
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    for (uint32_t countIndex = 0; countIndex < kNumSamplerDescriptorAllocators; countIndex++) {
        mSamplerAllocators[countIndex + 1] = std::make_unique<StagingDescriptorAllocator>(
            this, 1u << countIndex, kShaderVisibleDescriptorHeapSize,
            D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    }

    mRenderTargetViewAllocator = std::make_unique<StagingDescriptorAllocator>(
        this, 1, kAttachmentDescriptorHeapSize, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    mDepthStencilViewAllocator = std::make_unique<StagingDescriptorAllocator>(
        this, 1, kAttachmentDescriptorHeapSize, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    mSamplerHeapCache = std::make_unique<SamplerHeapCache>(this);

    mResidencyManager = std::make_unique<ResidencyManager>(this);
    mResourceAllocatorManager = std::make_unique<ResourceAllocatorManager>(this);

    // ShaderVisibleDescriptorAllocators use the ResidencyManager and must be initialized after.
    DAWN_TRY_ASSIGN(
        mSamplerShaderVisibleDescriptorAllocator,
        ShaderVisibleDescriptorAllocator::Create(this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));

    DAWN_TRY_ASSIGN(
        mViewShaderVisibleDescriptorAllocator,
        ShaderVisibleDescriptorAllocator::Create(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

    // Initialize indirect commands
    D3D12_INDIRECT_ARGUMENT_DESC argumentDesc = {};
    argumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

    D3D12_COMMAND_SIGNATURE_DESC programDesc = {};
    programDesc.ByteStride = 3 * sizeof(uint32_t);
    programDesc.NumArgumentDescs = 1;
    programDesc.pArgumentDescs = &argumentDesc;

    GetD3D12Device()->CreateCommandSignature(&programDesc, nullptr,
                                             IID_PPV_ARGS(&mDispatchIndirectSignature));

    argumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
    programDesc.ByteStride = 4 * sizeof(uint32_t);

    GetD3D12Device()->CreateCommandSignature(&programDesc, nullptr,
                                             IID_PPV_ARGS(&mDrawIndirectSignature));

    argumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
    programDesc.ByteStride = 5 * sizeof(uint32_t);

    GetD3D12Device()->CreateCommandSignature(&programDesc, nullptr,
                                             IID_PPV_ARGS(&mDrawIndexedIndirectSignature));

    DAWN_TRY(DeviceBase::Initialize(Queue::Create(this, &descriptor->defaultQueue)));
    // Device shouldn't be used until after DeviceBase::Initialize so we must wait until after
    // device initialization to call NextSerial
    DAWN_TRY(NextSerial());

    // Ensure DXC if use_dxc toggle is set.
    DAWN_TRY(EnsureDXCIfRequired());

    DAWN_TRY(CreateZeroBuffer());

    SetLabelImpl();

    return {};
}

Device::Device(AdapterBase* adapter,
               const DeviceDescriptor* descriptor,
               const TogglesState& deviceToggles)
    : Base(adapter, descriptor, deviceToggles) {}

Device::~Device() = default;

ID3D12Device* Device::GetD3D12Device() const {
    return mD3d12Device.Get();
}

ComPtr<ID3D12CommandQueue> Device::GetCommandQueue() const {
    return mCommandQueue;
}

ID3D12SharingContract* Device::GetSharingContract() const {
    return mD3d12SharingContract.Get();
}

ComPtr<ID3D12CommandSignature> Device::GetDispatchIndirectSignature() const {
    return mDispatchIndirectSignature;
}

ComPtr<ID3D12CommandSignature> Device::GetDrawIndirectSignature() const {
    return mDrawIndirectSignature;
}

ComPtr<ID3D12CommandSignature> Device::GetDrawIndexedIndirectSignature() const {
    return mDrawIndexedIndirectSignature;
}

// Ensure DXC if use_dxc toggles are set and validated.
MaybeError Device::EnsureDXCIfRequired() {
    if (IsToggleEnabled(Toggle::UseDXC)) {
        ASSERT(ToBackend(GetPhysicalDevice())->GetBackend()->IsDXCAvailable());
        DAWN_TRY(ToBackend(GetPhysicalDevice())->GetBackend()->EnsureDxcCompiler());
        DAWN_TRY(ToBackend(GetPhysicalDevice())->GetBackend()->EnsureDxcLibrary());
        DAWN_TRY(ToBackend(GetPhysicalDevice())->GetBackend()->EnsureDxcValidator());
    }

    return {};
}

const PlatformFunctions* Device::GetFunctions() const {
    return ToBackend(GetPhysicalDevice())->GetBackend()->GetFunctions();
}

CommandAllocatorManager* Device::GetCommandAllocatorManager() const {
    return mCommandAllocatorManager.get();
}

ResidencyManager* Device::GetResidencyManager() const {
    return mResidencyManager.get();
}

ResultOrError<CommandRecordingContext*> Device::GetPendingCommandContext(
    Device::SubmitMode submitMode) {
    // Callers of GetPendingCommandList do so to record commands. Only reserve a command
    // allocator when it is needed so we don't submit empty command lists
    if (!mPendingCommands.IsOpen()) {
        DAWN_TRY(mPendingCommands.Open(mD3d12Device.Get(), mCommandAllocatorManager.get()));
    }
    if (submitMode == Device::SubmitMode::Normal) {
        mPendingCommands.SetNeedsSubmit();
    }
    return &mPendingCommands;
}

MaybeError Device::CreateZeroBuffer() {
    BufferDescriptor zeroBufferDescriptor;
    zeroBufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    zeroBufferDescriptor.size = kZeroBufferSize;
    zeroBufferDescriptor.label = "ZeroBuffer_Internal";
    DAWN_TRY_ASSIGN(mZeroBuffer, Buffer::Create(this, &zeroBufferDescriptor));

    return {};
}

MaybeError Device::ClearBufferToZero(CommandRecordingContext* commandContext,
                                     BufferBase* destination,
                                     uint64_t offset,
                                     uint64_t size) {
    // TODO(crbug.com/dawn/852): It would be ideal to clear the buffer in CreateZeroBuffer, but
    // the allocation of the staging buffer causes various end2end tests that monitor heap usage
    // to fail if it's done during device creation. Perhaps ClearUnorderedAccessView*() can be
    // used to avoid that.
    if (!mZeroBuffer->IsDataInitialized()) {
        DynamicUploader* uploader = GetDynamicUploader();
        UploadHandle uploadHandle;
        DAWN_TRY_ASSIGN(uploadHandle, uploader->Allocate(kZeroBufferSize, GetPendingCommandSerial(),
                                                         kCopyBufferToBufferOffsetAlignment));

        memset(uploadHandle.mappedBuffer, 0u, kZeroBufferSize);

        CopyFromStagingToBufferHelper(commandContext, uploadHandle.stagingBuffer,
                                      uploadHandle.startOffset, mZeroBuffer.Get(), 0,
                                      kZeroBufferSize);

        mZeroBuffer->SetIsDataInitialized();
    }

    Buffer* dstBuffer = ToBackend(destination);

    // Necessary to ensure residency of the zero buffer.
    mZeroBuffer->TrackUsageAndTransitionNow(commandContext, wgpu::BufferUsage::CopySrc);
    dstBuffer->TrackUsageAndTransitionNow(commandContext, wgpu::BufferUsage::CopyDst);

    while (size > 0) {
        uint64_t copySize = std::min(kZeroBufferSize, size);
        commandContext->GetCommandList()->CopyBufferRegion(
            dstBuffer->GetD3D12Resource(), offset, mZeroBuffer->GetD3D12Resource(), 0, copySize);

        offset += copySize;
        size -= copySize;
    }

    return {};
}

MaybeError Device::TickImpl() {
    // Perform cleanup operations to free unused objects
    ExecutionSerial completedSerial = GetCompletedCommandSerial();

    mResourceAllocatorManager->Tick(completedSerial);
    DAWN_TRY(mCommandAllocatorManager->Tick(completedSerial));
    mViewShaderVisibleDescriptorAllocator->Tick(completedSerial);
    mSamplerShaderVisibleDescriptorAllocator->Tick(completedSerial);
    mRenderTargetViewAllocator->Tick(completedSerial);
    mDepthStencilViewAllocator->Tick(completedSerial);
    mUsedComObjectRefs.ClearUpTo(completedSerial);

    if (mPendingCommands.IsOpen() && mPendingCommands.NeedsSubmit()) {
        DAWN_TRY(ExecutePendingCommandContext());
        DAWN_TRY(NextSerial());
    }

    DAWN_TRY(CheckDebugLayerAndGenerateErrors());

    return {};
}

MaybeError Device::NextSerial() {
    IncrementLastSubmittedCommandSerial();

    TRACE_EVENT1(GetPlatform(), General, "D3D12Device::SignalFence", "serial",
                 uint64_t(GetLastSubmittedCommandSerial()));

    return CheckHRESULT(
        mCommandQueue->Signal(mFence.Get(), uint64_t(GetLastSubmittedCommandSerial())),
        "D3D12 command queue signal fence");
}

MaybeError Device::WaitForSerial(ExecutionSerial serial) {
    DAWN_TRY(CheckPassedSerials());
    if (GetCompletedCommandSerial() < serial) {
        DAWN_TRY(CheckHRESULT(mFence->SetEventOnCompletion(uint64_t(serial), mFenceEvent),
                              "D3D12 set event on completion"));
        WaitForSingleObject(mFenceEvent, INFINITE);
        DAWN_TRY(CheckPassedSerials());
    }
    return {};
}

ResultOrError<ExecutionSerial> Device::CheckAndUpdateCompletedSerials() {
    ExecutionSerial completedSerial = ExecutionSerial(mFence->GetCompletedValue());
    if (DAWN_UNLIKELY(completedSerial == ExecutionSerial(UINT64_MAX))) {
        // GetCompletedValue returns UINT64_MAX if the device was removed.
        // Try to query the failure reason.
        DAWN_TRY(CheckHRESULT(mD3d12Device->GetDeviceRemovedReason(),
                              "ID3D12Device::GetDeviceRemovedReason"));
        // Otherwise, return a generic device lost error.
        return DAWN_DEVICE_LOST_ERROR("Device lost");
    }

    if (completedSerial <= GetCompletedCommandSerial()) {
        return ExecutionSerial(0);
    }

    return completedSerial;
}

void Device::ReferenceUntilUnused(ComPtr<IUnknown> object) {
    mUsedComObjectRefs.Enqueue(object, GetPendingCommandSerial());
}

bool Device::HasPendingCommands() const {
    return mPendingCommands.NeedsSubmit();
}

void Device::ForceEventualFlushOfCommands() {
    if (mPendingCommands.IsOpen()) {
        mPendingCommands.SetNeedsSubmit();
    }
}

MaybeError Device::ExecutePendingCommandContext() {
    ASSERT(IsLockedByCurrentThreadIfNeeded());

    return mPendingCommands.ExecuteCommandList(this);
}

ResultOrError<Ref<BindGroupBase>> Device::CreateBindGroupImpl(
    const BindGroupDescriptor* descriptor) {
    return BindGroup::Create(this, descriptor);
}
ResultOrError<Ref<BindGroupLayoutInternalBase>> Device::CreateBindGroupLayoutImpl(
    const BindGroupLayoutDescriptor* descriptor) {
    return BindGroupLayout::Create(this, descriptor);
}
ResultOrError<Ref<BufferBase>> Device::CreateBufferImpl(const BufferDescriptor* descriptor) {
    return Buffer::Create(this, descriptor);
}
ResultOrError<Ref<CommandBufferBase>> Device::CreateCommandBuffer(
    CommandEncoder* encoder,
    const CommandBufferDescriptor* descriptor) {
    return CommandBuffer::Create(encoder, descriptor);
}
Ref<ComputePipelineBase> Device::CreateUninitializedComputePipelineImpl(
    const ComputePipelineDescriptor* descriptor) {
    return ComputePipeline::CreateUninitialized(this, descriptor);
}
ResultOrError<Ref<PipelineLayoutBase>> Device::CreatePipelineLayoutImpl(
    const PipelineLayoutDescriptor* descriptor) {
    return PipelineLayout::Create(this, descriptor);
}
ResultOrError<Ref<QuerySetBase>> Device::CreateQuerySetImpl(const QuerySetDescriptor* descriptor) {
    return QuerySet::Create(this, descriptor);
}
Ref<RenderPipelineBase> Device::CreateUninitializedRenderPipelineImpl(
    const RenderPipelineDescriptor* descriptor) {
    return RenderPipeline::CreateUninitialized(this, descriptor);
}
ResultOrError<Ref<SamplerBase>> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
    return Sampler::Create(this, descriptor);
}
ResultOrError<Ref<ShaderModuleBase>> Device::CreateShaderModuleImpl(
    const ShaderModuleDescriptor* descriptor,
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* compilationMessages) {
    return ShaderModule::Create(this, descriptor, parseResult, compilationMessages);
}
ResultOrError<Ref<SwapChainBase>> Device::CreateSwapChainImpl(
    Surface* surface,
    SwapChainBase* previousSwapChain,
    const SwapChainDescriptor* descriptor) {
    return SwapChain::Create(this, surface, previousSwapChain, descriptor);
}
ResultOrError<Ref<TextureBase>> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
    return Texture::Create(this, descriptor);
}
ResultOrError<Ref<TextureViewBase>> Device::CreateTextureViewImpl(
    TextureBase* texture,
    const TextureViewDescriptor* descriptor) {
    return TextureView::Create(texture, descriptor);
}
void Device::InitializeComputePipelineAsyncImpl(Ref<ComputePipelineBase> computePipeline,
                                                WGPUCreateComputePipelineAsyncCallback callback,
                                                void* userdata) {
    ComputePipeline::InitializeAsync(std::move(computePipeline), callback, userdata);
}
void Device::InitializeRenderPipelineAsyncImpl(Ref<RenderPipelineBase> renderPipeline,
                                               WGPUCreateRenderPipelineAsyncCallback callback,
                                               void* userdata) {
    RenderPipeline::InitializeAsync(std::move(renderPipeline), callback, userdata);
}

MaybeError Device::CopyFromStagingToBufferImpl(BufferBase* source,
                                               uint64_t sourceOffset,
                                               BufferBase* destination,
                                               uint64_t destinationOffset,
                                               uint64_t size) {
    CommandRecordingContext* commandRecordingContext;
    DAWN_TRY_ASSIGN(commandRecordingContext, GetPendingCommandContext(Device::SubmitMode::Passive));

    Buffer* dstBuffer = ToBackend(destination);

    bool cleared;
    DAWN_TRY_ASSIGN(cleared, dstBuffer->EnsureDataInitializedAsDestination(
                                 commandRecordingContext, destinationOffset, size));
    DAWN_UNUSED(cleared);

    CopyFromStagingToBufferHelper(commandRecordingContext, source, sourceOffset, destination,
                                  destinationOffset, size);

    return {};
}

void Device::CopyFromStagingToBufferHelper(CommandRecordingContext* commandContext,
                                           BufferBase* source,
                                           uint64_t sourceOffset,
                                           BufferBase* destination,
                                           uint64_t destinationOffset,
                                           uint64_t size) {
    ASSERT(commandContext != nullptr);
    Buffer* dstBuffer = ToBackend(destination);
    Buffer* srcBuffer = ToBackend(source);
    dstBuffer->TrackUsageAndTransitionNow(commandContext, wgpu::BufferUsage::CopyDst);

    commandContext->GetCommandList()->CopyBufferRegion(
        dstBuffer->GetD3D12Resource(), destinationOffset, srcBuffer->GetD3D12Resource(),
        sourceOffset, size);
}

MaybeError Device::CopyFromStagingToTextureImpl(const BufferBase* source,
                                                const TextureDataLayout& src,
                                                const TextureCopy& dst,
                                                const Extent3D& copySizePixels) {
    CommandRecordingContext* commandContext;
    DAWN_TRY_ASSIGN(commandContext, GetPendingCommandContext(Device::SubmitMode::Passive));
    Texture* texture = ToBackend(dst.texture.Get());

    SubresourceRange range = GetSubresourcesAffectedByCopy(dst, copySizePixels);

    if (IsCompleteSubresourceCopiedTo(texture, copySizePixels, dst.mipLevel)) {
        texture->SetIsSubresourceContentInitialized(true, range);
    } else {
        DAWN_TRY(texture->EnsureSubresourceContentInitialized(commandContext, range));
    }

    texture->TrackUsageAndTransitionNow(commandContext, wgpu::TextureUsage::CopyDst, range);

    RecordBufferTextureCopyWithBufferHandle(BufferTextureCopyDirection::B2T,
                                            commandContext->GetCommandList(),
                                            ToBackend(source)->GetD3D12Resource(), src.offset,
                                            src.bytesPerRow, src.rowsPerImage, dst, copySizePixels);

    return {};
}

void Device::DeallocateMemory(ResourceHeapAllocation& allocation) {
    mResourceAllocatorManager->DeallocateMemory(allocation);
}

ResultOrError<ResourceHeapAllocation> Device::AllocateMemory(
    D3D12_HEAP_TYPE heapType,
    const D3D12_RESOURCE_DESC& resourceDescriptor,
    D3D12_RESOURCE_STATES initialUsage,
    uint32_t formatBytesPerBlock,
    bool forceAllocateAsCommittedResource) {
    // formatBytesPerBlock is needed only for color non-compressed formats for a workaround.
    return mResourceAllocatorManager->AllocateMemory(heapType, resourceDescriptor, initialUsage,
                                                     formatBytesPerBlock,
                                                     forceAllocateAsCommittedResource);
}

ResultOrError<Ref<d3d::Fence>> Device::CreateFence(
    const d3d::ExternalImageDXGIFenceDescriptor* descriptor) {
    return Fence::CreateFromHandle(mD3d12Device.Get(), descriptor->fenceHandle,
                                   descriptor->fenceValue);
}

ResultOrError<std::unique_ptr<d3d::ExternalImageDXGIImpl>> Device::CreateExternalImageDXGIImplImpl(
    const ExternalImageDescriptor* descriptor) {
    // ExternalImageDXGIImpl holds a weak reference to the device. If the device is destroyed before
    // the image is created, the image will have a dangling reference to the device which can cause
    // a use-after-free.
    DAWN_TRY(ValidateIsAlive());

    DAWN_INVALID_IF(descriptor->GetType() != ExternalImageType::DXGISharedHandle,
                    "descriptor is not an ExternalImageDescriptorDXGISharedHandle");

    const d3d::ExternalImageDescriptorDXGISharedHandle* sharedHandleDescriptor =
        static_cast<const d3d::ExternalImageDescriptorDXGISharedHandle*>(descriptor);

    Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource;
    DAWN_TRY(CheckHRESULT(GetD3D12Device()->OpenSharedHandle(sharedHandleDescriptor->sharedHandle,
                                                             IID_PPV_ARGS(&d3d12Resource)),
                          "D3D12 opening shared handle"));

    const TextureDescriptor* textureDescriptor =
        FromAPI(sharedHandleDescriptor->cTextureDescriptor);

    DAWN_TRY(
        ValidateTextureDescriptor(this, textureDescriptor, AllowMultiPlanarTextureFormat::Yes));

    DAWN_TRY_CONTEXT(d3d::ValidateTextureDescriptorCanBeWrapped(textureDescriptor),
                     "validating that a D3D12 external image can be wrapped with %s",
                     textureDescriptor);

    DAWN_TRY(ValidateTextureCanBeWrapped(d3d12Resource.Get(), textureDescriptor));

    // Shared handle is assumed to support resource sharing capability. The resource
    // shared capability tier must agree to share resources between D3D devices.
    const Format* format = GetInternalFormat(textureDescriptor->format).AcquireSuccess();
    if (format->IsMultiPlanar()) {
        DAWN_TRY(ValidateVideoTextureCanBeShared(
            this, d3d::DXGITextureFormat(textureDescriptor->format)));
    }

    return std::make_unique<d3d::ExternalImageDXGIImpl>(this, std::move(d3d12Resource),
                                                        textureDescriptor);
}

Ref<TextureBase> Device::CreateD3DExternalTexture(const TextureDescriptor* descriptor,
                                                  ComPtr<IUnknown> d3dTexture,
                                                  std::vector<Ref<d3d::Fence>> waitFences,
                                                  bool isSwapChainTexture,
                                                  bool isInitialized) {
    Ref<Texture> dawnTexture;
    if (ConsumedError(
            Texture::CreateExternalImage(this, descriptor, std::move(d3dTexture),
                                         std::move(waitFences), isSwapChainTexture, isInitialized),
            &dawnTexture)) {
        return nullptr;
    }
    return {dawnTexture};
}

const D3D12DeviceInfo& Device::GetDeviceInfo() const {
    return ToBackend(GetPhysicalDevice())->GetDeviceInfo();
}

MaybeError Device::WaitForIdleForDestruction() {
    // Immediately forget about all pending commands
    mPendingCommands.Release();

    DAWN_TRY(NextSerial());
    // Wait for all in-flight commands to finish executing
    DAWN_TRY(WaitForSerial(GetLastSubmittedCommandSerial()));

    return {};
}

void AppendDebugLayerMessagesToError(ID3D12InfoQueue* infoQueue,
                                     uint64_t totalErrors,
                                     ErrorData* error) {
    ASSERT(totalErrors > 0);
    ASSERT(error != nullptr);

    uint64_t errorsToPrint = std::min(kMaxDebugMessagesToPrint, totalErrors);
    for (uint64_t i = 0; i < errorsToPrint; ++i) {
        std::ostringstream messageStream;
        SIZE_T messageLength = 0;
        HRESULT hr = infoQueue->GetMessage(i, nullptr, &messageLength);
        if (FAILED(hr)) {
            messageStream << " ID3D12InfoQueue::GetMessage failed with " << hr;
            error->AppendBackendMessage(messageStream.str());
            continue;
        }

        std::unique_ptr<uint8_t[]> messageData(new uint8_t[messageLength]);
        D3D12_MESSAGE* message = reinterpret_cast<D3D12_MESSAGE*>(messageData.get());
        hr = infoQueue->GetMessage(i, message, &messageLength);
        if (FAILED(hr)) {
            messageStream << " ID3D12InfoQueue::GetMessage failed with " << hr;
            error->AppendBackendMessage(messageStream.str());
            continue;
        }

        messageStream << message->pDescription << " (" << message->ID << ")";
        error->AppendBackendMessage(messageStream.str());
    }
    if (errorsToPrint < totalErrors) {
        std::ostringstream messages;
        messages << (totalErrors - errorsToPrint) << " messages silenced";
        error->AppendBackendMessage(messages.str());
    }

    // We only print up to the first kMaxDebugMessagesToPrint errors
    infoQueue->ClearStoredMessages();
}

MaybeError Device::CheckDebugLayerAndGenerateErrors() {
    if (!GetPhysicalDevice()->GetInstance()->IsBackendValidationEnabled()) {
        return {};
    }

    ComPtr<ID3D12InfoQueue> infoQueue;
    DAWN_TRY(CheckHRESULT(mD3d12Device.As(&infoQueue),
                          "D3D12 QueryInterface ID3D12Device to ID3D12InfoQueue"));
    uint64_t totalErrors = infoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();

    // Check if any errors have occurred otherwise we would be creating an empty error. Note
    // that we use GetNumStoredMessagesAllowedByRetrievalFilter instead of GetNumStoredMessages
    // because we only convert WARNINGS or higher messages to dawn errors.
    if (totalErrors == 0) {
        return {};
    }

    auto error = DAWN_INTERNAL_ERROR("The D3D12 debug layer reported uncaught errors.");

    AppendDebugLayerMessagesToError(infoQueue.Get(), totalErrors, error.get());

    return error;
}

void Device::AppendDebugLayerMessages(ErrorData* error) {
    if (!GetPhysicalDevice()->GetInstance()->IsBackendValidationEnabled()) {
        return;
    }

    ComPtr<ID3D12InfoQueue> infoQueue;
    if (FAILED(mD3d12Device.As(&infoQueue))) {
        return;
    }
    uint64_t totalErrors = infoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();

    if (totalErrors == 0) {
        return;
    }

    AppendDebugLayerMessagesToError(infoQueue.Get(), totalErrors, error);
}

void Device::DestroyImpl() {
    ASSERT(GetState() == State::Disconnected);

    Base::DestroyImpl();

    mZeroBuffer = nullptr;

    // Immediately forget about all pending commands for the case where device is lost on its
    // own and WaitForIdleForDestruction isn't called.
    mPendingCommands.Release();

    if (mFenceEvent != nullptr) {
        ::CloseHandle(mFenceEvent);
    }

    // Release recycled resource heaps and all other objects waiting for deletion in the resource
    // allocation manager.
    mResourceAllocatorManager.reset();

    // We need to handle clearing up com object refs that were enqeued after TickImpl
    mUsedComObjectRefs.ClearUpTo(std::numeric_limits<ExecutionSerial>::max());

    ASSERT(mUsedComObjectRefs.Empty());
    ASSERT(!mPendingCommands.IsOpen());

    // Now that we've cleared out pending work from the queue, we can safely release it and reclaim
    // memory.
    mCommandQueue.Reset();
}

ShaderVisibleDescriptorAllocator* Device::GetViewShaderVisibleDescriptorAllocator() const {
    return mViewShaderVisibleDescriptorAllocator.get();
}

ShaderVisibleDescriptorAllocator* Device::GetSamplerShaderVisibleDescriptorAllocator() const {
    return mSamplerShaderVisibleDescriptorAllocator.get();
}

StagingDescriptorAllocator* Device::GetViewStagingDescriptorAllocator(
    uint32_t descriptorCount) const {
    ASSERT(descriptorCount <= kMaxViewDescriptorsPerBindGroup);
    // This is Log2 of the next power of two, plus 1.
    uint32_t allocatorIndex = descriptorCount == 0 ? 0 : Log2Ceil(descriptorCount) + 1;
    return mViewAllocators[allocatorIndex].get();
}

StagingDescriptorAllocator* Device::GetSamplerStagingDescriptorAllocator(
    uint32_t descriptorCount) const {
    ASSERT(descriptorCount <= kMaxSamplerDescriptorsPerBindGroup);
    // This is Log2 of the next power of two, plus 1.
    uint32_t allocatorIndex = descriptorCount == 0 ? 0 : Log2Ceil(descriptorCount) + 1;
    return mSamplerAllocators[allocatorIndex].get();
}

StagingDescriptorAllocator* Device::GetRenderTargetViewAllocator() const {
    return mRenderTargetViewAllocator.get();
}

StagingDescriptorAllocator* Device::GetDepthStencilViewAllocator() const {
    return mDepthStencilViewAllocator.get();
}

SamplerHeapCache* Device::GetSamplerHeapCache() {
    return mSamplerHeapCache.get();
}

uint32_t Device::GetOptimalBytesPerRowAlignment() const {
    return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}

// TODO(dawn:512): Once we optimize DynamicUploader allocation with offsets we
// should make this return D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT = 512.
// Current implementations would try to allocate additional 511 bytes,
// so we return 1 and let ComputeTextureCopySplits take care of the alignment.
uint64_t Device::GetOptimalBufferToTextureCopyOffsetAlignment() const {
    return 1;
}

float Device::GetTimestampPeriodInNS() const {
    return mTimestampPeriod;
}

bool Device::ShouldDuplicateNumWorkgroupsForDispatchIndirect(
    ComputePipelineBase* computePipeline) const {
    return ToBackend(computePipeline)->UsesNumWorkgroups();
}

void Device::SetLabelImpl() {
    SetDebugName(this, mD3d12Device.Get(), "Dawn_Device", GetLabel());
}

bool Device::MayRequireDuplicationOfIndirectParameters() const {
    return true;
}

bool Device::ShouldDuplicateParametersForDrawIndirect(
    const RenderPipelineBase* renderPipelineBase) const {
    return ToBackend(renderPipelineBase)->UsesVertexOrInstanceIndex();
}

uint64_t Device::GetBufferCopyOffsetAlignmentForDepthStencil() const {
    // On the D3D12 platforms where programmable MSAA is not supported, the source box specifying a
    // portion of the depth texture must all be 0, or an error and a device lost will occur, so on
    // these platforms the buffer copy offset must be a multiple of 512 when the texture is created
    // with D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL. See https://crbug.com/dawn/727 for more
    // details.
    if (IsToggleEnabled(
            Toggle::D3D12UseTempBufferInDepthStencilTextureAndBufferCopyWithNonZeroBufferOffset)) {
        return D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
    }
    return DeviceBase::GetBufferCopyOffsetAlignmentForDepthStencil();
}

}  // namespace dawn::native::d3d12
