// Copyright 2017 The NXT Authors
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

#include "backend/d3d12/D3D12Backend.h"

#include "backend/d3d12/BindGroupD3D12.h"
#include "backend/d3d12/BindGroupLayoutD3D12.h"
#include "backend/d3d12/BlendStateD3D12.h"
#include "backend/d3d12/BufferD3D12.h"
#include "backend/d3d12/CommandAllocatorManager.h"
#include "backend/d3d12/CommandBufferD3D12.h"
#include "backend/d3d12/ComputePipelineD3D12.h"
#include "backend/d3d12/DepthStencilStateD3D12.h"
#include "backend/d3d12/DescriptorHeapAllocator.h"
#include "backend/d3d12/FramebufferD3D12.h"
#include "backend/d3d12/InputStateD3D12.h"
#include "backend/d3d12/NativeSwapChainImplD3D12.h"
#include "backend/d3d12/PipelineLayoutD3D12.h"
#include "backend/d3d12/QueueD3D12.h"
#include "backend/d3d12/RenderPipelineD3D12.h"
#include "backend/d3d12/ResourceAllocator.h"
#include "backend/d3d12/ResourceUploader.h"
#include "backend/d3d12/SamplerD3D12.h"
#include "backend/d3d12/ShaderModuleD3D12.h"
#include "backend/d3d12/SwapChainD3D12.h"
#include "backend/d3d12/TextureD3D12.h"
#include "common/Assert.h"
#include "common/SwapChainUtils.h"

namespace backend { namespace d3d12 {

    nxtProcTable GetNonValidatingProcs();
    nxtProcTable GetValidatingProcs();

    void Init(ComPtr<IDXGIFactory4> factory,
              ComPtr<ID3D12Device> d3d12Device,
              nxtProcTable* procs,
              nxtDevice* device) {
        *device = nullptr;
        *procs = GetValidatingProcs();
        *device = reinterpret_cast<nxtDevice>(new Device(factory, d3d12Device));
    }

    nxtSwapChainImplementation CreateNativeSwapChainImpl(nxtDevice device, HWND window) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        return CreateSwapChainImplementation(new NativeSwapChainImpl(backendDevice, window));
    }

    nxtTextureFormat GetNativeSwapChainPreferredFormat(
        const nxtSwapChainImplementation* swapChain) {
        NativeSwapChainImpl* impl = reinterpret_cast<NativeSwapChainImpl*>(swapChain->userData);
        return static_cast<nxtTextureFormat>(impl->GetPreferredFormat());
    }

    void ASSERT_SUCCESS(HRESULT hr) {
        ASSERT(SUCCEEDED(hr));
    }

    Device::Device(ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12Device> d3d12Device)
        : mFactory(factory),
          mD3d12Device(d3d12Device),
          mCommandAllocatorManager(new CommandAllocatorManager(this)),
          mDescriptorHeapAllocator(new DescriptorHeapAllocator(this)),
          mMapReadRequestTracker(new MapReadRequestTracker(this)),
          mResourceAllocator(new ResourceAllocator(this)),
          mResourceUploader(new ResourceUploader(this)) {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ASSERT_SUCCESS(d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

        ASSERT_SUCCESS(
            d3d12Device->CreateFence(mSerial, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
        mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        ASSERT(mFenceEvent != nullptr);

        NextSerial();
    }

    Device::~Device() {
        const uint64_t currentSerial = GetSerial();
        NextSerial();
        WaitForSerial(currentSerial);  // Wait for all in-flight commands to finish executing
        TickImpl();                    // Call tick one last time so resources are cleaned up
        delete mCommandAllocatorManager;
        delete mDescriptorHeapAllocator;
        delete mMapReadRequestTracker;
        delete mResourceAllocator;
        delete mResourceUploader;
    }

    ComPtr<IDXGIFactory4> Device::GetFactory() {
        return mFactory;
    }

    ComPtr<ID3D12Device> Device::GetD3D12Device() {
        return mD3d12Device;
    }

    ComPtr<ID3D12CommandQueue> Device::GetCommandQueue() {
        return mCommandQueue;
    }

    DescriptorHeapAllocator* Device::GetDescriptorHeapAllocator() {
        return mDescriptorHeapAllocator;
    }

    MapReadRequestTracker* Device::GetMapReadRequestTracker() const {
        return mMapReadRequestTracker;
    }

    ResourceAllocator* Device::GetResourceAllocator() {
        return mResourceAllocator;
    }

    ResourceUploader* Device::GetResourceUploader() {
        return mResourceUploader;
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

    void Device::TickImpl() {
        // Perform cleanup operations to free unused objects
        const uint64_t lastCompletedSerial = mFence->GetCompletedValue();
        mResourceAllocator->Tick(lastCompletedSerial);
        mCommandAllocatorManager->Tick(lastCompletedSerial);
        mDescriptorHeapAllocator->Tick(lastCompletedSerial);
        mMapReadRequestTracker->Tick(lastCompletedSerial);
        ExecuteCommandLists({});
        NextSerial();
    }

    uint64_t Device::GetSerial() const {
        return mSerial;
    }

    void Device::NextSerial() {
        ASSERT_SUCCESS(mCommandQueue->Signal(mFence.Get(), mSerial++));
    }

    void Device::WaitForSerial(uint64_t serial) {
        const uint64_t lastCompletedSerial = mFence->GetCompletedValue();
        if (lastCompletedSerial < serial) {
            ASSERT_SUCCESS(mFence->SetEventOnCompletion(serial, mFenceEvent));
            WaitForSingleObject(mFenceEvent, INFINITE);
        }
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
        } else {
            std::vector<ID3D12CommandList*> lists(commandLists);
            mCommandQueue->ExecuteCommandLists(static_cast<UINT>(commandLists.size()),
                                               lists.data());
        }
    }

    BindGroupBase* Device::CreateBindGroup(BindGroupBuilder* builder) {
        return new BindGroup(this, builder);
    }
    BindGroupLayoutBase* Device::CreateBindGroupLayout(BindGroupLayoutBuilder* builder) {
        return new BindGroupLayout(this, builder);
    }
    BlendStateBase* Device::CreateBlendState(BlendStateBuilder* builder) {
        return new BlendState(builder);
    }
    BufferBase* Device::CreateBuffer(BufferBuilder* builder) {
        return new Buffer(this, builder);
    }
    BufferViewBase* Device::CreateBufferView(BufferViewBuilder* builder) {
        return new BufferView(builder);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(this, builder);
    }
    ComputePipelineBase* Device::CreateComputePipeline(ComputePipelineBuilder* builder) {
        return new ComputePipeline(builder);
    }
    DepthStencilStateBase* Device::CreateDepthStencilState(DepthStencilStateBuilder* builder) {
        return new DepthStencilState(this, builder);
    }
    FramebufferBase* Device::CreateFramebuffer(FramebufferBuilder* builder) {
        return new Framebuffer(this, builder);
    }
    InputStateBase* Device::CreateInputState(InputStateBuilder* builder) {
        return new InputState(this, builder);
    }
    PipelineLayoutBase* Device::CreatePipelineLayout(PipelineLayoutBuilder* builder) {
        return new PipelineLayout(this, builder);
    }
    QueueBase* Device::CreateQueue(QueueBuilder* builder) {
        return new Queue(this, builder);
    }
    RenderPassBase* Device::CreateRenderPass(RenderPassBuilder* builder) {
        return new RenderPass(this, builder);
    }
    RenderPipelineBase* Device::CreateRenderPipeline(RenderPipelineBuilder* builder) {
        return new RenderPipeline(builder);
    }
    SamplerBase* Device::CreateSampler(SamplerBuilder* builder) {
        return new Sampler(builder);
    }
    ShaderModuleBase* Device::CreateShaderModule(ShaderModuleBuilder* builder) {
        return new ShaderModule(this, builder);
    }
    SwapChainBase* Device::CreateSwapChain(SwapChainBuilder* builder) {
        return new SwapChain(builder);
    }
    TextureBase* Device::CreateTexture(TextureBuilder* builder) {
        return new Texture(builder);
    }
    TextureViewBase* Device::CreateTextureView(TextureViewBuilder* builder) {
        return new TextureView(builder);
    }

    // RenderPass

    RenderPass::RenderPass(Device* device, RenderPassBuilder* builder)
        : RenderPassBase(builder), mDevice(device) {
    }

}}  // namespace backend::d3d12
