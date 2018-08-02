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
#include "common/SwapChainUtils.h"
#include "dawn_native/D3D12backend.h"
#include "dawn_native/d3d12/BindGroupD3D12.h"
#include "dawn_native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn_native/d3d12/BlendStateD3D12.h"
#include "dawn_native/d3d12/BufferD3D12.h"
#include "dawn_native/d3d12/CommandAllocatorManager.h"
#include "dawn_native/d3d12/CommandBufferD3D12.h"
#include "dawn_native/d3d12/ComputePipelineD3D12.h"
#include "dawn_native/d3d12/DepthStencilStateD3D12.h"
#include "dawn_native/d3d12/DescriptorHeapAllocator.h"
#include "dawn_native/d3d12/InputStateD3D12.h"
#include "dawn_native/d3d12/NativeSwapChainImplD3D12.h"
#include "dawn_native/d3d12/PipelineLayoutD3D12.h"
#include "dawn_native/d3d12/QueueD3D12.h"
#include "dawn_native/d3d12/RenderPassDescriptorD3D12.h"
#include "dawn_native/d3d12/RenderPipelineD3D12.h"
#include "dawn_native/d3d12/ResourceAllocator.h"
#include "dawn_native/d3d12/ResourceUploader.h"
#include "dawn_native/d3d12/SamplerD3D12.h"
#include "dawn_native/d3d12/ShaderModuleD3D12.h"
#include "dawn_native/d3d12/SwapChainD3D12.h"
#include "dawn_native/d3d12/TextureD3D12.h"

namespace dawn_native { namespace d3d12 {

    dawnDevice CreateDevice() {
        return reinterpret_cast<dawnDevice>(new Device());
    }

    dawnSwapChainImplementation CreateNativeSwapChainImpl(dawnDevice device, HWND window) {
        Device* backendDevice = reinterpret_cast<Device*>(device);

        dawnSwapChainImplementation impl;
        impl = CreateSwapChainImplementation(new NativeSwapChainImpl(backendDevice, window));
        impl.textureUsage = DAWN_TEXTURE_USAGE_BIT_PRESENT;

        return impl;
    }

    dawnTextureFormat GetNativeSwapChainPreferredFormat(
        const dawnSwapChainImplementation* swapChain) {
        NativeSwapChainImpl* impl = reinterpret_cast<NativeSwapChainImpl*>(swapChain->userData);
        return static_cast<dawnTextureFormat>(impl->GetPreferredFormat());
    }

    void ASSERT_SUCCESS(HRESULT hr) {
        ASSERT(SUCCEEDED(hr));
    }

    namespace {
        ComPtr<IDXGIFactory4> CreateFactory() {
            ComPtr<IDXGIFactory4> factory;

            uint32_t dxgiFactoryFlags = 0;
#if defined(DAWN_ENABLE_ASSERTS)
            // Enable the debug layer (requires the Graphics Tools "optional feature").
            {
                ComPtr<ID3D12Debug> debugController;
                if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
                    debugController->EnableDebugLayer();

                    // Enable additional debug layers.
                    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
                }

                ComPtr<IDXGIDebug1> dxgiDebug;
                if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)))) {
                    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
                                                 DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_ALL));
                }
            }
#endif  // defined(DAWN_ENABLE_ASSERTS)

            ASSERT_SUCCESS(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
            return factory;
        }

        ComPtr<IDXGIAdapter1> GetHardwareAdapter(ComPtr<IDXGIFactory4> factory) {
            for (uint32_t adapterIndex = 0;; ++adapterIndex) {
                IDXGIAdapter1* adapter = nullptr;
                if (factory->EnumAdapters1(adapterIndex, &adapter) == DXGI_ERROR_NOT_FOUND) {
                    break;  // No more adapters to enumerate.
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the actual
                // device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0,
                                                _uuidof(ID3D12Device), nullptr))) {
                    return adapter;
                }
                adapter->Release();
            }
            return nullptr;
        }

    }  // anonymous namespace

    Device::Device() {
        // Create the connection to DXGI and the D3D12 device
        mFactory = CreateFactory();
        ASSERT(mFactory.Get() != nullptr);

        mHardwareAdapter = GetHardwareAdapter(mFactory);
        ASSERT(mHardwareAdapter.Get() != nullptr);

        ASSERT_SUCCESS(D3D12CreateDevice(mHardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                         IID_PPV_ARGS(&mD3d12Device)));

        // Create device-global objects
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ASSERT_SUCCESS(mD3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

        ASSERT_SUCCESS(
            mD3d12Device->CreateFence(mSerial, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
        mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        ASSERT(mFenceEvent != nullptr);

        // Initialize backend services
        mCommandAllocatorManager = new CommandAllocatorManager(this);
        mDescriptorHeapAllocator = new DescriptorHeapAllocator(this);
        mMapRequestTracker = new MapRequestTracker(this);
        mResourceAllocator = new ResourceAllocator(this);
        mResourceUploader = new ResourceUploader(this);

        NextSerial();
    }

    Device::~Device() {
        const uint64_t currentSerial = GetSerial();
        NextSerial();
        WaitForSerial(currentSerial);  // Wait for all in-flight commands to finish executing
        TickImpl();                    // Call tick one last time so resources are cleaned up
        ASSERT(mUsedComObjectRefs.Empty());

        delete mCommandAllocatorManager;
        delete mDescriptorHeapAllocator;
        delete mMapRequestTracker;
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

    MapRequestTracker* Device::GetMapRequestTracker() const {
        return mMapRequestTracker;
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
        mMapRequestTracker->Tick(lastCompletedSerial);
        mUsedComObjectRefs.ClearUpTo(lastCompletedSerial);
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

    void Device::ReferenceUntilUnused(ComPtr<IUnknown> object) {
        mUsedComObjectRefs.Enqueue(object, mSerial);
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
    ResultOrError<BindGroupLayoutBase*> Device::CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) {
        return new BindGroupLayout(this, descriptor);
    }
    BlendStateBase* Device::CreateBlendState(BlendStateBuilder* builder) {
        return new BlendState(builder);
    }
    BufferBase* Device::CreateBuffer(BufferBuilder* builder) {
        return new Buffer(builder);
    }
    BufferViewBase* Device::CreateBufferView(BufferViewBuilder* builder) {
        return new BufferView(builder);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(builder);
    }
    ComputePipelineBase* Device::CreateComputePipeline(ComputePipelineBuilder* builder) {
        return new ComputePipeline(builder);
    }
    DepthStencilStateBase* Device::CreateDepthStencilState(DepthStencilStateBuilder* builder) {
        return new DepthStencilState(this, builder);
    }
    InputStateBase* Device::CreateInputState(InputStateBuilder* builder) {
        return new InputState(this, builder);
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
        return new RenderPassDescriptor(this, builder);
    }
    RenderPipelineBase* Device::CreateRenderPipeline(RenderPipelineBuilder* builder) {
        return new RenderPipeline(builder);
    }
    ResultOrError<SamplerBase*> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
        return new Sampler(this, descriptor);
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

}}  // namespace dawn_native::d3d12
