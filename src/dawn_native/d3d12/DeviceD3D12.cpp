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

#include "dawn_native/d3d12/DeviceD3D12.h"

#include "common/Assert.h"
#include "dawn_native/BackendConnection.h"
#include "dawn_native/DynamicUploader.h"
#include "dawn_native/ErrorData.h"
#include "dawn_native/d3d12/AdapterD3D12.h"
#include "dawn_native/d3d12/BackendD3D12.h"
#include "dawn_native/d3d12/BindGroupD3D12.h"
#include "dawn_native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn_native/d3d12/BufferD3D12.h"
#include "dawn_native/d3d12/CommandAllocatorManager.h"
#include "dawn_native/d3d12/CommandBufferD3D12.h"
#include "dawn_native/d3d12/ComputePipelineD3D12.h"
#include "dawn_native/d3d12/D3D12Error.h"
#include "dawn_native/d3d12/DescriptorHeapAllocator.h"
#include "dawn_native/d3d12/PipelineLayoutD3D12.h"
#include "dawn_native/d3d12/PlatformFunctions.h"
#include "dawn_native/d3d12/QueueD3D12.h"
#include "dawn_native/d3d12/RenderPipelineD3D12.h"
#include "dawn_native/d3d12/ResidencyManagerD3D12.h"
#include "dawn_native/d3d12/ResourceAllocatorManagerD3D12.h"
#include "dawn_native/d3d12/SamplerD3D12.h"
#include "dawn_native/d3d12/ShaderModuleD3D12.h"
#include "dawn_native/d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"
#include "dawn_native/d3d12/StagingBufferD3D12.h"
#include "dawn_native/d3d12/SwapChainD3D12.h"
#include "dawn_native/d3d12/TextureD3D12.h"

namespace dawn_native { namespace d3d12 {

    Device::Device(Adapter* adapter, const DeviceDescriptor* descriptor)
        : DeviceBase(adapter, descriptor) {
        InitTogglesFromDriver();
        if (descriptor != nullptr) {
            ApplyToggleOverrides(descriptor);
        }
    }

    MaybeError Device::Initialize() {
        mD3d12Device = ToBackend(GetAdapter())->GetDevice();

        ASSERT(mD3d12Device != nullptr);

        // Create device-global objects
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        DAWN_TRY(
            CheckHRESULT(mD3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)),
                         "D3D12 create command queue"));

        // If PIX is not attached, the QueryInterface fails. Hence, no need to check the return
        // value.
        mCommandQueue.As(&mD3d12SharingContract);

        DAWN_TRY(CheckHRESULT(mD3d12Device->CreateFence(mLastSubmittedSerial, D3D12_FENCE_FLAG_NONE,
                                                        IID_PPV_ARGS(&mFence)),
                              "D3D12 create fence"));

        mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        ASSERT(mFenceEvent != nullptr);

        // Initialize backend services
        mCommandAllocatorManager = std::make_unique<CommandAllocatorManager>(this);
        mDescriptorHeapAllocator = std::make_unique<DescriptorHeapAllocator>(this);

        mShaderVisibleDescriptorAllocator =
            std::make_unique<ShaderVisibleDescriptorAllocator>(this);
        DAWN_TRY(mShaderVisibleDescriptorAllocator->Initialize());

        mMapRequestTracker = std::make_unique<MapRequestTracker>(this);
        mResidencyManager = std::make_unique<ResidencyManager>(this);
        mResourceAllocatorManager = std::make_unique<ResourceAllocatorManager>(this);

        DAWN_TRY(NextSerial());

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

        return {};
    }

    Device::~Device() {
        BaseDestructor();
    }

    ComPtr<ID3D12Device> Device::GetD3D12Device() const {
        return mD3d12Device;
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

    DescriptorHeapAllocator* Device::GetDescriptorHeapAllocator() const {
        return mDescriptorHeapAllocator.get();
    }

    ComPtr<IDXGIFactory4> Device::GetFactory() const {
        return ToBackend(GetAdapter())->GetBackend()->GetFactory();
    }

    const PlatformFunctions* Device::GetFunctions() const {
        return ToBackend(GetAdapter())->GetBackend()->GetFunctions();
    }

    MapRequestTracker* Device::GetMapRequestTracker() const {
        return mMapRequestTracker.get();
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

    Serial Device::GetCompletedCommandSerial() const {
        return mCompletedSerial;
    }

    Serial Device::GetLastSubmittedCommandSerial() const {
        return mLastSubmittedSerial;
    }

    Serial Device::GetPendingCommandSerial() const {
        return mLastSubmittedSerial + 1;
    }

    MaybeError Device::TickImpl() {
        // Perform cleanup operations to free unused objects
        mCompletedSerial = mFence->GetCompletedValue();

        // Uploader should tick before the resource allocator
        // as it enqueued resources to be released.
        mDynamicUploader->Deallocate(mCompletedSerial);

        mResourceAllocatorManager->Tick(mCompletedSerial);
        DAWN_TRY(mCommandAllocatorManager->Tick(mCompletedSerial));
        mShaderVisibleDescriptorAllocator->Tick(mCompletedSerial);
        mMapRequestTracker->Tick(mCompletedSerial);
        mUsedComObjectRefs.ClearUpTo(mCompletedSerial);
        DAWN_TRY(ExecutePendingCommandContext());
        DAWN_TRY(NextSerial());
        return {};
    }

    MaybeError Device::NextSerial() {
        mLastSubmittedSerial++;
        return CheckHRESULT(mCommandQueue->Signal(mFence.Get(), mLastSubmittedSerial),
                            "D3D12 command queue signal fence");
    }

    MaybeError Device::WaitForSerial(uint64_t serial) {
        mCompletedSerial = mFence->GetCompletedValue();
        if (mCompletedSerial < serial) {
            DAWN_TRY(CheckHRESULT(mFence->SetEventOnCompletion(serial, mFenceEvent),
                                  "D3D12 set event on completion"));
            WaitForSingleObject(mFenceEvent, INFINITE);
        }
        return {};
    }

    void Device::ReferenceUntilUnused(ComPtr<IUnknown> object) {
        mUsedComObjectRefs.Enqueue(object, GetPendingCommandSerial());
    }

    MaybeError Device::ExecutePendingCommandContext() {
        return mPendingCommands.ExecuteCommandList(this);
    }

    ResultOrError<BindGroupBase*> Device::CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) {
        return BindGroup::Create(this, descriptor);
    }
    ResultOrError<BindGroupLayoutBase*> Device::CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) {
        return new BindGroupLayout(this, descriptor);
    }
    ResultOrError<BufferBase*> Device::CreateBufferImpl(const BufferDescriptor* descriptor) {
        std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>(this, descriptor);
        DAWN_TRY(buffer->Initialize());
        return buffer.release();
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandEncoder* encoder,
                                                   const CommandBufferDescriptor* descriptor) {
        return new CommandBuffer(encoder, descriptor);
    }
    ResultOrError<ComputePipelineBase*> Device::CreateComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) {
        return ComputePipeline::Create(this, descriptor);
    }
    ResultOrError<PipelineLayoutBase*> Device::CreatePipelineLayoutImpl(
        const PipelineLayoutDescriptor* descriptor) {
        return PipelineLayout::Create(this, descriptor);
    }
    ResultOrError<QueueBase*> Device::CreateQueueImpl() {
        return new Queue(this);
    }
    ResultOrError<RenderPipelineBase*> Device::CreateRenderPipelineImpl(
        const RenderPipelineDescriptor* descriptor) {
        return RenderPipeline::Create(this, descriptor);
    }
    ResultOrError<SamplerBase*> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
        return new Sampler(this, descriptor);
    }
    ResultOrError<ShaderModuleBase*> Device::CreateShaderModuleImpl(
        const ShaderModuleDescriptor* descriptor) {
        return ShaderModule::Create(this, descriptor);
    }
    ResultOrError<SwapChainBase*> Device::CreateSwapChainImpl(
        const SwapChainDescriptor* descriptor) {
        return new SwapChain(this, descriptor);
    }
    ResultOrError<NewSwapChainBase*> Device::CreateSwapChainImpl(
        Surface* surface,
        NewSwapChainBase* previousSwapChain,
        const SwapChainDescriptor* descriptor) {
        return DAWN_VALIDATION_ERROR("New swapchains not implemented.");
    }
    ResultOrError<TextureBase*> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
        return Texture::Create(this, descriptor);
    }
    ResultOrError<TextureViewBase*> Device::CreateTextureViewImpl(
        TextureBase* texture,
        const TextureViewDescriptor* descriptor) {
        return new TextureView(texture, descriptor);
    }

    ResultOrError<std::unique_ptr<StagingBufferBase>> Device::CreateStagingBuffer(size_t size) {
        std::unique_ptr<StagingBufferBase> stagingBuffer =
            std::make_unique<StagingBuffer>(size, this);
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
        StagingBuffer* srcBuffer = ToBackend(source);
        dstBuffer->TrackUsageAndTransitionNow(commandRecordingContext, wgpu::BufferUsage::CopyDst);

        commandRecordingContext->GetCommandList()->CopyBufferRegion(
            dstBuffer->GetD3D12Resource().Get(), destinationOffset, srcBuffer->GetResource(),
            sourceOffset, size);

        return {};
    }

    void Device::DeallocateMemory(ResourceHeapAllocation& allocation) {
        mResourceAllocatorManager->DeallocateMemory(allocation);
    }

    ResultOrError<ResourceHeapAllocation> Device::AllocateMemory(
        D3D12_HEAP_TYPE heapType,
        const D3D12_RESOURCE_DESC& resourceDescriptor,
        D3D12_RESOURCE_STATES initialUsage) {
        return mResourceAllocatorManager->AllocateMemory(heapType, resourceDescriptor,
                                                         initialUsage);
    }

    TextureBase* Device::WrapSharedHandle(const ExternalImageDescriptor* descriptor,
                                          HANDLE sharedHandle,
                                          uint64_t acquireMutexKey,
                                          bool isSwapChainTexture) {
        TextureBase* dawnTexture;
        if (ConsumedError(Texture::Create(this, descriptor, sharedHandle, acquireMutexKey,
                                          isSwapChainTexture),
                          &dawnTexture))
            return nullptr;

        return dawnTexture;
    }

    // We use IDXGIKeyedMutexes to synchronize access between D3D11 and D3D12. D3D11/12 fences
    // are a viable alternative but are, unfortunately, not available on all versions of Windows
    // 10. Since D3D12 does not directly support keyed mutexes, we need to wrap the D3D12
    // resource using 11on12 and QueryInterface the D3D11 representation for the keyed mutex.
    ResultOrError<ComPtr<IDXGIKeyedMutex>> Device::CreateKeyedMutexForTexture(
        ID3D12Resource* d3d12Resource) {
        if (mD3d11On12Device == nullptr) {
            ComPtr<ID3D11Device> d3d11Device;
            ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
            D3D_FEATURE_LEVEL d3dFeatureLevel;
            IUnknown* const iUnknownQueue = mCommandQueue.Get();
            DAWN_TRY(CheckHRESULT(GetFunctions()->d3d11on12CreateDevice(
                                      mD3d12Device.Get(), 0, nullptr, 0, &iUnknownQueue, 1, 1,
                                      &d3d11Device, &d3d11DeviceContext, &d3dFeatureLevel),
                                  "D3D12 11on12 device create"));

            ComPtr<ID3D11On12Device> d3d11on12Device;
            DAWN_TRY(CheckHRESULT(d3d11Device.As(&d3d11on12Device),
                                  "D3D12 QueryInterface ID3D11Device to ID3D11On12Device"));

            ComPtr<ID3D11DeviceContext2> d3d11DeviceContext2;
            DAWN_TRY(
                CheckHRESULT(d3d11DeviceContext.As(&d3d11DeviceContext2),
                             "D3D12 QueryInterface ID3D11DeviceContext to ID3D11DeviceContext2"));

            mD3d11On12DeviceContext = std::move(d3d11DeviceContext2);
            mD3d11On12Device = std::move(d3d11on12Device);
        }

        ComPtr<ID3D11Texture2D> d3d11Texture;
        D3D11_RESOURCE_FLAGS resourceFlags;
        resourceFlags.BindFlags = 0;
        resourceFlags.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
        resourceFlags.CPUAccessFlags = 0;
        resourceFlags.StructureByteStride = 0;
        DAWN_TRY(CheckHRESULT(mD3d11On12Device->CreateWrappedResource(
                                  d3d12Resource, &resourceFlags, D3D12_RESOURCE_STATE_COMMON,
                                  D3D12_RESOURCE_STATE_COMMON, IID_PPV_ARGS(&d3d11Texture)),
                              "D3D12 creating a wrapped resource"));

        ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex;
        DAWN_TRY(CheckHRESULT(d3d11Texture.As(&dxgiKeyedMutex),
                              "D3D12 QueryInterface ID3D11Texture2D to IDXGIKeyedMutex"));

        return dxgiKeyedMutex;
    }

    void Device::ReleaseKeyedMutexForTexture(ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex) {
        ComPtr<ID3D11Resource> d3d11Resource;
        HRESULT hr = dxgiKeyedMutex.As(&d3d11Resource);
        if (FAILED(hr)) {
            return;
        }

        ID3D11Resource* d3d11ResourceRaw = d3d11Resource.Get();
        mD3d11On12Device->ReleaseWrappedResources(&d3d11ResourceRaw, 1);

        d3d11Resource.Reset();
        dxgiKeyedMutex.Reset();

        // 11on12 has a bug where D3D12 resources used only for keyed shared mutexes
        // are not released until work is submitted to the device context and flushed.
        // The most minimal work we can get away with is issuing a TiledResourceBarrier.

        // ID3D11DeviceContext2 is available in Win8.1 and above. This suffices for a
        // D3D12 backend since both D3D12 and 11on12 first appeared in Windows 10.
        mD3d11On12DeviceContext->TiledResourceBarrier(nullptr, nullptr);
        mD3d11On12DeviceContext->Flush();
    }

    const D3D12DeviceInfo& Device::GetDeviceInfo() const {
        return ToBackend(GetAdapter())->GetDeviceInfo();
    }

    void Device::InitTogglesFromDriver() {
        const bool useResourceHeapTier2 = (GetDeviceInfo().resourceHeapTier >= 2);
        SetToggle(Toggle::UseD3D12ResourceHeapTier2, useResourceHeapTier2);
        SetToggle(Toggle::UseD3D12RenderPass, GetDeviceInfo().supportsRenderPass);
        SetToggle(Toggle::UseD3D12ResidencyManagement, false);

        // By default use the maximum shader-visible heap size allowed.
        SetToggle(Toggle::UseD3D12SmallShaderVisibleHeapForTesting, false);
    }

    MaybeError Device::WaitForIdleForDestruction() {
        // Immediately forget about all pending commands
        mPendingCommands.Release();

        DAWN_TRY(NextSerial());
        // Wait for all in-flight commands to finish executing
        DAWN_TRY(WaitForSerial(mLastSubmittedSerial));

        // Call tick one last time so resources are cleaned up.
        DAWN_TRY(TickImpl());

        return {};
    }

    void Device::Destroy() {
        ASSERT(mLossStatus != LossStatus::AlreadyLost);

        // Immediately forget about all pending commands
        mPendingCommands.Release();

        // Free services explicitly so that they can free D3D12 resources before destruction of the
        // device.
        mDynamicUploader = nullptr;

        // GPU is no longer executing commands. Existing objects do not get freed until the device
        // is destroyed. To ensure objects are always released, force the completed serial to be
        // MAX.
        mCompletedSerial = std::numeric_limits<Serial>::max();

        if (mFenceEvent != nullptr) {
            ::CloseHandle(mFenceEvent);
        }

        mUsedComObjectRefs.ClearUpTo(mCompletedSerial);

        ASSERT(mUsedComObjectRefs.Empty());
        ASSERT(!mPendingCommands.IsOpen());
    }

    ShaderVisibleDescriptorAllocator* Device::GetShaderVisibleDescriptorAllocator() const {
        return mShaderVisibleDescriptorAllocator.get();
    }
}}  // namespace dawn_native::d3d12
