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
#include "dawn_native/D3D12Backend.h"
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
#include "dawn_native/d3d12/PlatformFunctions.h"
#include "dawn_native/d3d12/QueueD3D12.h"
#include "dawn_native/d3d12/RenderPassDescriptorD3D12.h"
#include "dawn_native/d3d12/RenderPipelineD3D12.h"
#include "dawn_native/d3d12/ResourceAllocator.h"
#include "dawn_native/d3d12/ResourceUploader.h"
#include "dawn_native/d3d12/SamplerD3D12.h"
#include "dawn_native/d3d12/ShaderModuleD3D12.h"
#include "dawn_native/d3d12/SwapChainD3D12.h"
#include "dawn_native/d3d12/TextureD3D12.h"

#include <locale>

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
        ComPtr<IDXGIFactory4> CreateFactory(const PlatformFunctions* functions) {
            ComPtr<IDXGIFactory4> factory;

            uint32_t dxgiFactoryFlags = 0;
#if defined(DAWN_ENABLE_ASSERTS)
            // Enable the debug layer (requires the Graphics Tools "optional feature").
            {
                ComPtr<ID3D12Debug> debugController;
                if (SUCCEEDED(functions->d3d12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
                    debugController->EnableDebugLayer();

                    // Enable additional debug layers.
                    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
                }

                ComPtr<IDXGIDebug1> dxgiDebug;
                if (SUCCEEDED(functions->dxgiGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)))) {
                    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
                                                 DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_ALL));
                }
            }
#endif  // defined(DAWN_ENABLE_ASSERTS)

            ASSERT_SUCCESS(functions->createDxgiFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
            return factory;
        }

        ComPtr<IDXGIAdapter1> GetHardwareAdapter(ComPtr<IDXGIFactory4> factory,
                                                 const PlatformFunctions* functions) {
            for (uint32_t adapterIndex = 0;; ++adapterIndex) {
                IDXGIAdapter1* adapter = nullptr;
                if (factory->EnumAdapters1(adapterIndex, &adapter) == DXGI_ERROR_NOT_FOUND) {
                    break;  // No more adapters to enumerate.
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the actual
                // device yet.
                if (SUCCEEDED(functions->d3d12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0,
                                                           _uuidof(ID3D12Device), nullptr))) {
                    return adapter;
                }
                adapter->Release();
            }
            return nullptr;
        }

    }  // anonymous namespace

    Device::Device() {
        mFunctions = std::make_unique<PlatformFunctions>();

        {
            MaybeError status = mFunctions->LoadFunctions();
            ASSERT(status.IsSuccess());
        }

        // Create the connection to DXGI and the D3D12 device
        mFactory = CreateFactory(mFunctions.get());
        ASSERT(mFactory.Get() != nullptr);

        mHardwareAdapter = GetHardwareAdapter(mFactory, mFunctions.get());
        ASSERT(mHardwareAdapter.Get() != nullptr);

        ASSERT_SUCCESS(mFunctions->d3d12CreateDevice(mHardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                                     IID_PPV_ARGS(&mD3d12Device)));

        // Collect GPU information
        CollectPCIInfo();

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
        mResourceUploader = std::make_unique<ResourceUploader>(this);

        NextSerial();
    }

    Device::~Device() {
        NextSerial();
        WaitForSerial(mLastSubmittedSerial);  // Wait for all in-flight commands to finish executing
        TickImpl();                    // Call tick one last time so resources are cleaned up

        ASSERT(mUsedComObjectRefs.Empty());
        ASSERT(mPendingCommands.commandList == nullptr);
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
        return mDescriptorHeapAllocator.get();
    }

    const PlatformFunctions* Device::GetFunctions() {
        return mFunctions.get();
    }

    MapRequestTracker* Device::GetMapRequestTracker() const {
        return mMapRequestTracker.get();
    }

    ResourceAllocator* Device::GetResourceAllocator() {
        return mResourceAllocator.get();
    }

    ResourceUploader* Device::GetResourceUploader() {
        return mResourceUploader.get();
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
        ExecuteCommandLists({});
        NextSerial();
    }

    const dawn_native::PCIInfo& Device::GetPCIInfo() const {
        return mPCIInfo;
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
    BlendStateBase* Device::CreateBlendState(BlendStateBuilder* builder) {
        return new BlendState(builder);
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
    DepthStencilStateBase* Device::CreateDepthStencilState(DepthStencilStateBuilder* builder) {
        return new DepthStencilState(builder);
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
    RenderPipelineBase* Device::CreateRenderPipeline(RenderPipelineBuilder* builder) {
        return new RenderPipeline(builder);
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

    void Device::CollectPCIInfo() {
        memset(&mPCIInfo, 0, sizeof(mPCIInfo));

        DXGI_ADAPTER_DESC1 adapterDesc;
        mHardwareAdapter->GetDesc1(&adapterDesc);

        mPCIInfo.deviceId = adapterDesc.DeviceId;
        mPCIInfo.vendorId = adapterDesc.VendorId;

        std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> converter(
            "Error converting");
        mPCIInfo.name = converter.to_bytes(adapterDesc.Description);
    }

}}  // namespace dawn_native::d3d12
