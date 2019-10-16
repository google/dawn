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
#include "dawn_native/d3d12/ResourceAllocator.h"
#include "dawn_native/d3d12/ResourceAllocatorManagerD3D12.h"
#include "dawn_native/d3d12/SamplerD3D12.h"
#include "dawn_native/d3d12/ShaderModuleD3D12.h"
#include "dawn_native/d3d12/StagingBufferD3D12.h"
#include "dawn_native/d3d12/SwapChainD3D12.h"
#include "dawn_native/d3d12/TextureD3D12.h"

namespace dawn_native { namespace d3d12 {

    Device::Device(Adapter* adapter, const DeviceDescriptor* descriptor)
        : DeviceBase(adapter, descriptor) {
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

        DAWN_TRY(CheckHRESULT(mD3d12Device->CreateFence(mLastSubmittedSerial, D3D12_FENCE_FLAG_NONE,
                                                        IID_PPV_ARGS(&mFence)),
                              "D3D12 create fence"));

        mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        ASSERT(mFenceEvent != nullptr);

        // Initialize backend services
        mCommandAllocatorManager = std::make_unique<CommandAllocatorManager>(this);
        mDescriptorHeapAllocator = std::make_unique<DescriptorHeapAllocator>(this);
        mMapRequestTracker = std::make_unique<MapRequestTracker>(this);
        mResourceAllocator = std::make_unique<ResourceAllocator>(this);
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
        // Immediately forget about all pending commands
        mPendingCommands.Release();

        ConsumedError(NextSerial());
        // Wait for all in-flight commands to finish executing
        ConsumedError(WaitForSerial(mLastSubmittedSerial));

        // Call tick one last time so resources are cleaned up. Ignore the return value so we can
        // continue shutting down in an orderly fashion.
        ConsumedError(TickImpl());

        // Free services explicitly so that they can free D3D12 resources before destruction of the
        // device.
        mDynamicUploader = nullptr;

        // GPU is no longer executing commands. Existing objects do not get freed until the device
        // is destroyed. To ensure objects are always released, force the completed serial to be
        // MAX.
        mCompletedSerial = std::numeric_limits<Serial>::max();

        // Releasing the uploader enqueues buffers to be released.
        // Call Tick() again to clear them before releasing the allocator.
        mResourceAllocator->Tick(mCompletedSerial);

        if (mFenceEvent != nullptr) {
            ::CloseHandle(mFenceEvent);
        }

        mUsedComObjectRefs.ClearUpTo(mCompletedSerial);

        ASSERT(mUsedComObjectRefs.Empty());
        ASSERT(!mPendingCommands.IsOpen());
    }

    ComPtr<ID3D12Device> Device::GetD3D12Device() const {
        return mD3d12Device;
    }

    ComPtr<ID3D12CommandQueue> Device::GetCommandQueue() const {
        return mCommandQueue;
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

    ResourceAllocator* Device::GetResourceAllocator() const {
        return mResourceAllocator.get();
    }

    CommandAllocatorManager* Device::GetCommandAllocatorManager() const {
        return mCommandAllocatorManager.get();
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

        mResourceAllocator->Tick(mCompletedSerial);
        DAWN_TRY(mCommandAllocatorManager->Tick(mCompletedSerial));
        mDescriptorHeapAllocator->Deallocate(mCompletedSerial);
        mMapRequestTracker->Tick(mCompletedSerial);
        mUsedComObjectRefs.ClearUpTo(mCompletedSerial);
        DAWN_TRY(ExecuteCommandContext(nullptr));
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

    MaybeError Device::ExecuteCommandContext(CommandRecordingContext* commandContext) {
        UINT numLists = 0;
        std::array<ID3D12CommandList*, 2> d3d12CommandLists;

        // If there are pending commands, prepend them to ExecuteCommandLists
        if (mPendingCommands.IsOpen()) {
            ID3D12GraphicsCommandList* d3d12CommandList;
            DAWN_TRY_ASSIGN(d3d12CommandList, mPendingCommands.Close());
            d3d12CommandLists[numLists++] = d3d12CommandList;
        }
        if (commandContext != nullptr) {
            ID3D12GraphicsCommandList* d3d12CommandList;
            DAWN_TRY_ASSIGN(d3d12CommandList, commandContext->Close());
            d3d12CommandLists[numLists++] = d3d12CommandList;
        }
        if (numLists > 0) {
            mCommandQueue->ExecuteCommandLists(numLists, d3d12CommandLists.data());
        }

        return {};
    }

    ResultOrError<BindGroupBase*> Device::CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) {
        return new BindGroup(this, descriptor);
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
    CommandBufferBase* Device::CreateCommandBuffer(CommandEncoderBase* encoder,
                                                   const CommandBufferDescriptor* descriptor) {
        return new CommandBuffer(encoder, descriptor);
    }
    ResultOrError<ComputePipelineBase*> Device::CreateComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) {
        return new ComputePipeline(this, descriptor);
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
        return new ShaderModule(this, descriptor);
    }
    ResultOrError<SwapChainBase*> Device::CreateSwapChainImpl(
        const SwapChainDescriptor* descriptor) {
        return new SwapChain(this, descriptor);
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

        ToBackend(destination)
            ->TransitionUsageNow(commandRecordingContext, dawn::BufferUsage::CopyDst);

        commandRecordingContext->GetCommandList()->CopyBufferRegion(
            ToBackend(destination)->GetD3D12Resource().Get(), destinationOffset,
            ToBackend(source)->GetResource(), sourceOffset, size);

        return {};
    }

    void Device::DeallocateMemory(ResourceHeapAllocation& allocation) {
        mResourceAllocatorManager->DeallocateMemory(allocation);
    }

    ResultOrError<ResourceHeapAllocation> Device::AllocateMemory(
        D3D12_HEAP_TYPE heapType,
        const D3D12_RESOURCE_DESC& resourceDescriptor,
        D3D12_RESOURCE_STATES initialUsage,
        D3D12_HEAP_FLAGS heapFlags) {
        return mResourceAllocatorManager->AllocateMemory(heapType, resourceDescriptor, initialUsage,
                                                         heapFlags);
    }

    TextureBase* Device::WrapSharedHandle(const TextureDescriptor* descriptor,
                                          HANDLE sharedHandle) {
        if (ConsumedError(ValidateTextureDescriptor(this, descriptor))) {
            return nullptr;
        }

        if (ConsumedError(ValidateTextureDescriptorCanBeWrapped(descriptor))) {
            return nullptr;
        }

        ComPtr<ID3D12Resource> d3d12Resource;
        const HRESULT hr =
            mD3d12Device->OpenSharedHandle(sharedHandle, IID_PPV_ARGS(&d3d12Resource));
        if (FAILED(hr)) {
            return nullptr;
        }

        if (ConsumedError(ValidateD3D12TextureCanBeWrapped(d3d12Resource.Get(), descriptor))) {
            return nullptr;
        }

        return new Texture(this, descriptor, std::move(d3d12Resource));
    }
}}  // namespace dawn_native::d3d12
