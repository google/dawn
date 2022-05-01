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
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d12/AdapterD3D12.h"
#include "dawn/native/d3d12/BackendD3D12.h"
#include "dawn/native/d3d12/BindGroupD3D12.h"
#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn/native/d3d12/CommandAllocatorManager.h"
#include "dawn/native/d3d12/CommandBufferD3D12.h"
#include "dawn/native/d3d12/ComputePipelineD3D12.h"
#include "dawn/native/d3d12/D3D11on12Util.h"
#include "dawn/native/d3d12/D3D12Error.h"
#include "dawn/native/d3d12/PipelineLayoutD3D12.h"
#include "dawn/native/d3d12/PlatformFunctions.h"
#include "dawn/native/d3d12/QuerySetD3D12.h"
#include "dawn/native/d3d12/QueueD3D12.h"
#include "dawn/native/d3d12/RenderPipelineD3D12.h"
#include "dawn/native/d3d12/ResidencyManagerD3D12.h"
#include "dawn/native/d3d12/ResourceAllocatorManagerD3D12.h"
#include "dawn/native/d3d12/SamplerD3D12.h"
#include "dawn/native/d3d12/SamplerHeapCacheD3D12.h"
#include "dawn/native/d3d12/ShaderModuleD3D12.h"
#include "dawn/native/d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"
#include "dawn/native/d3d12/StagingBufferD3D12.h"
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
ResultOrError<Ref<Device>> Device::Create(Adapter* adapter, const DeviceDescriptor* descriptor) {
    Ref<Device> device = AcquireRef(new Device(adapter, descriptor));
    DAWN_TRY(device->Initialize(descriptor));
    return device;
}

MaybeError Device::Initialize(const DeviceDescriptor* descriptor) {
    InitTogglesFromDriver();

    mD3d12Device = ToBackend(GetAdapter())->GetDevice();

    ASSERT(mD3d12Device != nullptr);

    // Create device-global objects
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    DAWN_TRY(
        CheckHRESULT(mD3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)),
                     "D3D12 create command queue"));

    if (IsFeatureEnabled(Feature::TimestampQuery) &&
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
                                                    D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)),
                          "D3D12 create fence"));

    mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    ASSERT(mFenceEvent != nullptr);

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

    GetD3D12Device()->CreateCommandSignature(&programDesc, NULL,
                                             IID_PPV_ARGS(&mDispatchIndirectSignature));

    argumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
    programDesc.ByteStride = 4 * sizeof(uint32_t);

    GetD3D12Device()->CreateCommandSignature(&programDesc, NULL,
                                             IID_PPV_ARGS(&mDrawIndirectSignature));

    argumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
    programDesc.ByteStride = 5 * sizeof(uint32_t);

    GetD3D12Device()->CreateCommandSignature(&programDesc, NULL,
                                             IID_PPV_ARGS(&mDrawIndexedIndirectSignature));

    DAWN_TRY(DeviceBase::Initialize(Queue::Create(this, &descriptor->defaultQueue)));
    // Device shouldn't be used until after DeviceBase::Initialize so we must wait until after
    // device initialization to call NextSerial
    DAWN_TRY(NextSerial());

    // The environment can only use DXC when it's available. Override the decision if it is not
    // applicable.
    DAWN_TRY(ApplyUseDxcToggle());

    DAWN_TRY(CreateZeroBuffer());

    SetLabelImpl();

    return {};
}

Device::~Device() {
    Destroy();
}

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

ComPtr<IDXGIFactory4> Device::GetFactory() const {
    return ToBackend(GetAdapter())->GetBackend()->GetFactory();
}

MaybeError Device::ApplyUseDxcToggle() {
    if (!ToBackend(GetAdapter())->GetBackend()->GetFunctions()->IsDXCAvailable()) {
        ForceSetToggle(Toggle::UseDXC, false);
    } else if (IsFeatureEnabled(Feature::ShaderFloat16)) {
        // Currently we can only use DXC to compile HLSL shaders using float16.
        ForceSetToggle(Toggle::UseDXC, true);
    }

    if (IsToggleEnabled(Toggle::UseDXC)) {
        DAWN_TRY(ToBackend(GetAdapter())->GetBackend()->EnsureDxcCompiler());
        DAWN_TRY(ToBackend(GetAdapter())->GetBackend()->EnsureDxcLibrary());
        DAWN_TRY(ToBackend(GetAdapter())->GetBackend()->EnsureDxcValidator());
    }

    return {};
}

ComPtr<IDxcLibrary> Device::GetDxcLibrary() const {
    return ToBackend(GetAdapter())->GetBackend()->GetDxcLibrary();
}

ComPtr<IDxcCompiler> Device::GetDxcCompiler() const {
    return ToBackend(GetAdapter())->GetBackend()->GetDxcCompiler();
}

ComPtr<IDxcValidator> Device::GetDxcValidator() const {
    return ToBackend(GetAdapter())->GetBackend()->GetDxcValidator();
}

const PlatformFunctions* Device::GetFunctions() const {
    return ToBackend(GetAdapter())->GetBackend()->GetFunctions();
}

CommandAllocatorManager* Device::GetCommandAllocatorManager() const {
    return mCommandAllocatorManager.get();
}

ResidencyManager* Device::GetResidencyManager() const {
    return mResidencyManager.get();
}

ResultOrError<CommandRecordingContext*> Device::GetPendingCommandContext() {
    // Callers of GetPendingCommandList do so to record commands. Only reserve a command
    // allocator when it is needed so we don't submit empty command lists
    if (!mPendingCommands.IsOpen()) {
        DAWN_TRY(mPendingCommands.Open(mD3d12Device.Get(), mCommandAllocatorManager.get()));
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

        CopyFromStagingToBufferImpl(commandContext, uploadHandle.stagingBuffer,
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

    if (mPendingCommands.IsOpen()) {
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

MaybeError Device::ExecutePendingCommandContext() {
    return mPendingCommands.ExecuteCommandList(this);
}

ResultOrError<Ref<BindGroupBase>> Device::CreateBindGroupImpl(
    const BindGroupDescriptor* descriptor) {
    return BindGroup::Create(this, descriptor);
}
ResultOrError<Ref<BindGroupLayoutBase>> Device::CreateBindGroupLayoutImpl(
    const BindGroupLayoutDescriptor* descriptor,
    PipelineCompatibilityToken pipelineCompatibilityToken) {
    return BindGroupLayout::Create(this, descriptor, pipelineCompatibilityToken);
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
    ShaderModuleParseResult* parseResult) {
    return ShaderModule::Create(this, descriptor, parseResult);
}
ResultOrError<Ref<SwapChainBase>> Device::CreateSwapChainImpl(
    const SwapChainDescriptor* descriptor) {
    return OldSwapChain::Create(this, descriptor);
}
ResultOrError<Ref<NewSwapChainBase>> Device::CreateSwapChainImpl(
    Surface* surface,
    NewSwapChainBase* previousSwapChain,
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

ResultOrError<std::unique_ptr<StagingBufferBase>> Device::CreateStagingBuffer(size_t size) {
    std::unique_ptr<StagingBufferBase> stagingBuffer = std::make_unique<StagingBuffer>(size, this);
    DAWN_TRY(stagingBuffer->Initialize());
    return std::move(stagingBuffer);
}

MaybeError Device::CopyFromStagingToBuffer(StagingBufferBase* source,
                                           uint64_t sourceOffset,
                                           BufferBase* destination,
                                           uint64_t destinationOffset,
                                           uint64_t size) {
    CommandRecordingContext* commandRecordingContext;
    DAWN_TRY_ASSIGN(commandRecordingContext, GetPendingCommandContext());

    Buffer* dstBuffer = ToBackend(destination);

    bool cleared;
    DAWN_TRY_ASSIGN(cleared, dstBuffer->EnsureDataInitializedAsDestination(
                                 commandRecordingContext, destinationOffset, size));
    DAWN_UNUSED(cleared);

    CopyFromStagingToBufferImpl(commandRecordingContext, source, sourceOffset, destination,
                                destinationOffset, size);

    return {};
}

void Device::CopyFromStagingToBufferImpl(CommandRecordingContext* commandContext,
                                         StagingBufferBase* source,
                                         uint64_t sourceOffset,
                                         BufferBase* destination,
                                         uint64_t destinationOffset,
                                         uint64_t size) {
    ASSERT(commandContext != nullptr);
    Buffer* dstBuffer = ToBackend(destination);
    StagingBuffer* srcBuffer = ToBackend(source);
    dstBuffer->TrackUsageAndTransitionNow(commandContext, wgpu::BufferUsage::CopyDst);

    commandContext->GetCommandList()->CopyBufferRegion(dstBuffer->GetD3D12Resource(),
                                                       destinationOffset, srcBuffer->GetResource(),
                                                       sourceOffset, size);
}

MaybeError Device::CopyFromStagingToTexture(const StagingBufferBase* source,
                                            const TextureDataLayout& src,
                                            TextureCopy* dst,
                                            const Extent3D& copySizePixels) {
    CommandRecordingContext* commandContext;
    DAWN_TRY_ASSIGN(commandContext, GetPendingCommandContext());
    Texture* texture = ToBackend(dst->texture.Get());

    SubresourceRange range = GetSubresourcesAffectedByCopy(*dst, copySizePixels);

    if (IsCompleteSubresourceCopiedTo(texture, copySizePixels, dst->mipLevel)) {
        texture->SetIsSubresourceContentInitialized(true, range);
    } else {
        texture->EnsureSubresourceContentInitialized(commandContext, range);
    }

    texture->TrackUsageAndTransitionNow(commandContext, wgpu::TextureUsage::CopyDst, range);

    RecordBufferTextureCopyWithBufferHandle(
        BufferTextureCopyDirection::B2T, commandContext->GetCommandList(),
        ToBackend(source)->GetResource(), src.offset, src.bytesPerRow, src.rowsPerImage, *dst,
        copySizePixels);

    return {};
}

void Device::DeallocateMemory(ResourceHeapAllocation& allocation) {
    mResourceAllocatorManager->DeallocateMemory(allocation);
}

ResultOrError<ResourceHeapAllocation> Device::AllocateMemory(
    D3D12_HEAP_TYPE heapType,
    const D3D12_RESOURCE_DESC& resourceDescriptor,
    D3D12_RESOURCE_STATES initialUsage) {
    return mResourceAllocatorManager->AllocateMemory(heapType, resourceDescriptor, initialUsage);
}

Ref<TextureBase> Device::CreateD3D12ExternalTexture(
    const TextureDescriptor* descriptor,
    ComPtr<ID3D12Resource> d3d12Texture,
    Ref<D3D11on12ResourceCacheEntry> d3d11on12Resource,
    bool isSwapChainTexture,
    bool isInitialized) {
    Ref<Texture> dawnTexture;
    if (ConsumedError(Texture::CreateExternalImage(this, descriptor, std::move(d3d12Texture),
                                                   std::move(d3d11on12Resource), isSwapChainTexture,
                                                   isInitialized),
                      &dawnTexture)) {
        return nullptr;
    }
    return {dawnTexture};
}

ComPtr<ID3D11On12Device> Device::GetOrCreateD3D11on12Device() {
    if (mD3d11On12Device == nullptr) {
        ComPtr<ID3D11Device> d3d11Device;
        D3D_FEATURE_LEVEL d3dFeatureLevel;
        IUnknown* const iUnknownQueue = mCommandQueue.Get();
        if (FAILED(GetFunctions()->d3d11on12CreateDevice(mD3d12Device.Get(), 0, nullptr, 0,
                                                         &iUnknownQueue, 1, 1, &d3d11Device,
                                                         nullptr, &d3dFeatureLevel))) {
            return nullptr;
        }

        ComPtr<ID3D11On12Device> d3d11on12Device;
        HRESULT hr = d3d11Device.As(&d3d11on12Device);
        ASSERT(SUCCEEDED(hr));

        mD3d11On12Device = std::move(d3d11on12Device);
    }
    return mD3d11On12Device;
}

const D3D12DeviceInfo& Device::GetDeviceInfo() const {
    return ToBackend(GetAdapter())->GetDeviceInfo();
}

void Device::InitTogglesFromDriver() {
    const bool useResourceHeapTier2 = (GetDeviceInfo().resourceHeapTier >= 2);
    SetToggle(Toggle::UseD3D12ResourceHeapTier2, useResourceHeapTier2);
    SetToggle(Toggle::UseD3D12RenderPass, GetDeviceInfo().supportsRenderPass);
    SetToggle(Toggle::UseD3D12ResidencyManagement, true);
    SetToggle(Toggle::UseDXC, false);

    // Disable optimizations when using FXC
    // See https://crbug.com/dawn/1203
    SetToggle(Toggle::FxcOptimizations, false);

    // By default use the maximum shader-visible heap size allowed.
    SetToggle(Toggle::UseD3D12SmallShaderVisibleHeapForTesting, false);

    uint32_t deviceId = GetAdapter()->GetDeviceId();
    uint32_t vendorId = GetAdapter()->GetVendorId();

    // Currently this workaround is only needed on Intel Gen9 and Gen9.5 GPUs.
    // See http://crbug.com/1161355 for more information.
    if (gpu_info::IsIntel(vendorId) &&
        (gpu_info::IsSkylake(deviceId) || gpu_info::IsKabylake(deviceId) ||
         gpu_info::IsCoffeelake(deviceId))) {
        constexpr gpu_info::D3DDriverVersion kFirstDriverVersionWithFix = {30, 0, 100, 9864};
        if (gpu_info::CompareD3DDriverVersion(vendorId, ToBackend(GetAdapter())->GetDriverVersion(),
                                              kFirstDriverVersionWithFix) < 0) {
            SetToggle(
                Toggle::UseTempBufferInSmallFormatTextureToTextureCopyFromGreaterToLessMipLevel,
                true);
        }
    }

    // Currently this workaround is needed on any D3D12 backend for some particular situations.
    // But we may need to limit it if D3D12 runtime fixes the bug on its new release. See
    // https://crbug.com/dawn/1289 for more information.
    SetToggle(Toggle::D3D12SplitBufferTextureCopyForRowsPerImagePaddings, true);
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
    if (!GetAdapter()->GetInstance()->IsBackendValidationEnabled()) {
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
    if (!GetAdapter()->GetInstance()->IsBackendValidationEnabled()) {
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

    // Immediately forget about all pending commands for the case where device is lost on its
    // own and WaitForIdleForDestruction isn't called.
    mPendingCommands.Release();

    if (mFenceEvent != nullptr) {
        ::CloseHandle(mFenceEvent);
    }

    // Release recycled resource heaps.
    if (mResourceAllocatorManager != nullptr) {
        mResourceAllocatorManager->DestroyPool();
    }

    // We need to handle clearing up com object refs that were enqeued after TickImpl
    mUsedComObjectRefs.ClearUpTo(std::numeric_limits<ExecutionSerial>::max());

    ASSERT(mUsedComObjectRefs.Empty());
    ASSERT(!mPendingCommands.IsOpen());
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

}  // namespace dawn::native::d3d12
