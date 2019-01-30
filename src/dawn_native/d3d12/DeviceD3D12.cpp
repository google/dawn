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
#include "dawn_native/d3d12/InputStateD3D12.h"
#include "dawn_native/d3d12/PipelineLayoutD3D12.h"
#include "dawn_native/d3d12/PlatformFunctions.h"
#include "dawn_native/d3d12/QueueD3D12.h"
#include "dawn_native/d3d12/RenderPassDescriptorD3D12.h"
#include "dawn_native/d3d12/RenderPipelineD3D12.h"
#include "dawn_native/d3d12/ResourceAllocator.h"
#include "dawn_native/d3d12/SamplerD3D12.h"
#include "dawn_native/d3d12/ShaderModuleD3D12.h"
#include "dawn_native/d3d12/StagingBufferD3D12.h"
#include "dawn_native/d3d12/SwapChainD3D12.h"
#include "dawn_native/d3d12/TextureD3D12.h"

namespace dawn_native { namespace d3d12 {

    void ASSERT_SUCCESS(HRESULT hr) {
        ASSERT(SUCCEEDED(hr));
    }

    Device::Device(Adapter* adapter, ComPtr<ID3D12Device> d3d12Device)
        : DeviceBase(adapter), mD3d12Device(d3d12Device) {
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
        mDynamicUploader = std::make_unique<DynamicUploader>(this);

        NextSerial();
    }

    Device::~Device() {
        NextSerial();
        WaitForSerial(mLastSubmittedSerial);  // Wait for all in-flight commands to finish executing
        TickImpl();                    // Call tick one last time so resources are cleaned up

        ASSERT(mUsedComObjectRefs.Empty());
        ASSERT(mPendingCommands.commandList == nullptr);
    }

    ComPtr<ID3D12Device> Device::GetD3D12Device() const {
        return mD3d12Device;
    }

    ComPtr<ID3D12CommandQueue> Device::GetCommandQueue() const {
        return mCommandQueue;
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

    void Device::TickImpl() {
        // Perform cleanup operations to free unused objects
        mCompletedSerial = mFence->GetCompletedValue();
        mResourceAllocator->Tick(mCompletedSerial);
        mCommandAllocatorManager->Tick(mCompletedSerial);
        mDescriptorHeapAllocator->Tick(mCompletedSerial);
        mMapRequestTracker->Tick(mCompletedSerial);
        mUsedComObjectRefs.ClearUpTo(mCompletedSerial);
        mDynamicUploader->Tick(mCompletedSerial);
        ExecuteCommandLists({});
        NextSerial();
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

    void Device::ExecuteCommandLists(std::initializer_list<ID3D12CommandList*> commandLists) {
        // If there are pending commands, prepend them to ExecuteCommandLists
        if (mPendingCommands.open) {
            std::vector<ID3D12CommandList*> lists(commandLists.size() + 1);
            mPendingCommands.commandList->Close();
            mPendingCommands.open = false;
            lists[0] = mPendingCommands.commandList.Get();
            std::copy(commandLists.begin(), commandLists.end(), lists.begin() + 1);
            mCommandQueue->ExecuteCommandLists(static_cast<UINT>(commandLists.size() + 1),
                                               lists.data());
            mPendingCommands.commandList = nullptr;
        } else {
            std::vector<ID3D12CommandList*> lists(commandLists);
            mCommandQueue->ExecuteCommandLists(static_cast<UINT>(commandLists.size()),
                                               lists.data());
        }
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
        return new Buffer(this, descriptor);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(builder);
    }
    ResultOrError<ComputePipelineBase*> Device::CreateComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) {
        return new ComputePipeline(this, descriptor);
    }
    InputStateBase* Device::CreateInputState(InputStateBuilder* builder) {
        return new InputState(builder);
    }
    ResultOrError<PipelineLayoutBase*> Device::CreatePipelineLayoutImpl(
        const PipelineLayoutDescriptor* descriptor) {
        return new PipelineLayout(this, descriptor);
    }
    ResultOrError<QueueBase*> Device::CreateQueueImpl() {
        return new Queue(this);
    }
    RenderPassDescriptorBase* Device::CreateRenderPassDescriptor(
        RenderPassDescriptorBuilder* builder) {
        return new RenderPassDescriptor(builder);
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
    SwapChainBase* Device::CreateSwapChain(SwapChainBuilder* builder) {
        return new SwapChain(builder);
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
        return std::move(stagingBuffer);
    }

    MaybeError Device::CopyFromStagingToBuffer(StagingBufferBase* source,
                                               uint32_t sourceOffset,
                                               BufferBase* destination,
                                               uint32_t destinationOffset,
                                               uint32_t size) {
        ToBackend(destination)
            ->TransitionUsageNow(GetPendingCommandList(), dawn::BufferUsageBit::TransferDst);

        GetPendingCommandList()->CopyBufferRegion(
            ToBackend(destination)->GetD3D12Resource().Get(), destinationOffset,
            ToBackend(source)->GetResource(), sourceOffset, size);

        return {};
    }

    ResultOrError<DynamicUploader*> Device::GetDynamicUploader() const {
        // TODO(b-brber): Refactor this into device init once moved into DeviceBase.
        if (mDynamicUploader->IsEmpty()) {
            DAWN_TRY(mDynamicUploader->CreateAndAppendBuffer(kDefaultUploadBufferSize));
        }
        return mDynamicUploader.get();
    }

}}  // namespace dawn_native::d3d12
