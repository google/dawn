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
#include "dawn_native/d3d12/DescriptorHeapAllocator.h"
#include "dawn_native/d3d12/PipelineLayoutD3D12.h"
#include "dawn_native/d3d12/PlatformFunctions.h"
#include "dawn_native/d3d12/QueueD3D12.h"
#include "dawn_native/d3d12/RenderPipelineD3D12.h"
#include "dawn_native/d3d12/ResourceAllocator.h"
#include "dawn_native/d3d12/ResourceAllocatorManagerD3D12.h"
#include "dawn_native/d3d12/ResourceHeapD3D12.h"
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
        ASSERT_SUCCESS(mD3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

        ASSERT_SUCCESS(mD3d12Device->CreateFence(mLastSubmittedSerial, D3D12_FENCE_FLAG_NONE,
                                                 IID_PPV_ARGS(&mFence)));
        mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        ASSERT(mFenceEvent != nullptr);

        // Initialize backend services
        mCommandAllocatorManager = std::make_unique<CommandAllocatorManager>(this);
        mDescriptorHeapAllocator = std::make_unique<DescriptorHeapAllocator>(this);
        mMapRequestTracker = std::make_unique<MapRequestTracker>(this);
        mResourceAllocator = std::make_unique<ResourceAllocator>(this);
        mResourceAllocatorManager = std::make_unique<ResourceAllocatorManager>(this);

        NextSerial();

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
        if (mPendingCommands.open) {
            mPendingCommands.commandList->Close();
            mPendingCommands.open = false;
            mPendingCommands.commandList = nullptr;
        }
        NextSerial();
        WaitForSerial(mLastSubmittedSerial);  // Wait for all in-flight commands to finish executing

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
        ASSERT(mPendingCommands.commandList == nullptr);
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

    void Device::OpenCommandList(ComPtr<ID3D12GraphicsCommandList>* commandList) {
        ComPtr<ID3D12GraphicsCommandList>& cmdList = *commandList;
        if (!cmdList) {
            ASSERT_SUCCESS(mD3d12Device->CreateCommandList(
                0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                mCommandAllocatorManager->ReserveCommandAllocator().Get(), nullptr,
                IID_PPV_ARGS(&cmdList)));
        } else {
            ASSERT_SUCCESS(
                cmdList->Reset(mCommandAllocatorManager->ReserveCommandAllocator().Get(), nullptr));
        }
    }

    ComPtr<ID3D12GraphicsCommandList> Device::GetPendingCommandList() {
        // Callers of GetPendingCommandList do so to record commands. Only reserve a command
        // allocator when it is needed so we don't submit empty command lists
        if (!mPendingCommands.open) {
            OpenCommandList(&mPendingCommands.commandList);
            mPendingCommands.open = true;
        }
        return mPendingCommands.commandList;
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
        mCommandAllocatorManager->Tick(mCompletedSerial);
        mDescriptorHeapAllocator->Deallocate(mCompletedSerial);
        mMapRequestTracker->Tick(mCompletedSerial);
        mUsedComObjectRefs.ClearUpTo(mCompletedSerial);
        DAWN_TRY(ExecuteCommandList(nullptr));
        NextSerial();

        return {};
    }

    void Device::NextSerial() {
        mLastSubmittedSerial++;
        ASSERT_SUCCESS(mCommandQueue->Signal(mFence.Get(), mLastSubmittedSerial));
    }

    void Device::WaitForSerial(uint64_t serial) {
        mCompletedSerial = mFence->GetCompletedValue();
        if (mCompletedSerial < serial) {
            ASSERT_SUCCESS(mFence->SetEventOnCompletion(serial, mFenceEvent));
            WaitForSingleObject(mFenceEvent, INFINITE);
        }
    }

    void Device::ReferenceUntilUnused(ComPtr<IUnknown> object) {
        mUsedComObjectRefs.Enqueue(object, GetPendingCommandSerial());
    }

    MaybeError Device::ExecuteCommandList(ID3D12CommandList* d3d12CommandList) {
        UINT numLists = 0;
        std::array<ID3D12CommandList*, 2> d3d12CommandLists;

        // If there are pending commands, prepend them to ExecuteCommandLists
        if (mPendingCommands.open) {
            const HRESULT hr = mPendingCommands.commandList->Close();
            if (FAILED(hr)) {
                mPendingCommands.open = false;
                mPendingCommands.commandList.Reset();
                return DAWN_DEVICE_LOST_ERROR("Error closing pending command list.");
            }
            mPendingCommands.open = false;
            d3d12CommandLists[numLists++] = mPendingCommands.commandList.Get();
        }
        if (d3d12CommandList != nullptr) {
            d3d12CommandLists[numLists++] = d3d12CommandList;
        }
        if (numLists > 0) {
            mCommandQueue->ExecuteCommandLists(numLists, d3d12CommandLists.data());
            mPendingCommands.commandList.Reset();
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
        return new PipelineLayout(this, descriptor);
    }
    ResultOrError<QueueBase*> Device::CreateQueueImpl() {
        return new Queue(this);
    }
    ResultOrError<RenderPipelineBase*> Device::CreateRenderPipelineImpl(
        const RenderPipelineDescriptor* descriptor) {
        return new RenderPipeline(this, descriptor);
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
        return new Texture(this, descriptor);
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
        ToBackend(destination)
            ->TransitionUsageNow(GetPendingCommandList(), dawn::BufferUsage::CopyDst);

        GetPendingCommandList()->CopyBufferRegion(
            ToBackend(destination)->GetD3D12Resource().Get(), destinationOffset,
            ToBackend(source)->GetResource(), sourceOffset, size);

        return {};
    }

    void Device::DeallocateMemory(ResourceMemoryAllocation& allocation) {
        mResourceAllocatorManager->DeallocateMemory(allocation);
    }

    ResultOrError<ResourceMemoryAllocation> Device::AllocateMemory(
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

        return new Texture(this, descriptor, d3d12Resource.Get());
    }
}}  // namespace dawn_native::d3d12
