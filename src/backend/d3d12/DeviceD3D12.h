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

#ifndef BACKEND_D3D12_DEVICED3D12_H_
#define BACKEND_D3D12_DEVICED3D12_H_

#include "nxt/nxtcpp.h"

#include "backend/Device.h"
#include "backend/d3d12/Forward.h"
#include "backend/d3d12/d3d12_platform.h"
#include "common/SerialQueue.h"

namespace backend { namespace d3d12 {

    class CommandAllocatorManager;
    class DescriptorHeapAllocator;
    class MapRequestTracker;
    class ResourceAllocator;
    class ResourceUploader;

    void ASSERT_SUCCESS(HRESULT hr);

    // Definition of backend types
    class Device : public DeviceBase {
      public:
        Device();
        ~Device();

        BindGroupBase* CreateBindGroup(BindGroupBuilder* builder) override;
        BlendStateBase* CreateBlendState(BlendStateBuilder* builder) override;
        BufferBase* CreateBuffer(BufferBuilder* builder) override;
        BufferViewBase* CreateBufferView(BufferViewBuilder* builder) override;
        CommandBufferBase* CreateCommandBuffer(CommandBufferBuilder* builder) override;
        ComputePipelineBase* CreateComputePipeline(ComputePipelineBuilder* builder) override;
        DepthStencilStateBase* CreateDepthStencilState(DepthStencilStateBuilder* builder) override;
        InputStateBase* CreateInputState(InputStateBuilder* builder) override;
        RenderPassDescriptorBase* CreateRenderPassDescriptor(
            RenderPassDescriptorBuilder* builder) override;
        RenderPipelineBase* CreateRenderPipeline(RenderPipelineBuilder* builder) override;
        ShaderModuleBase* CreateShaderModule(ShaderModuleBuilder* builder) override;
        SwapChainBase* CreateSwapChain(SwapChainBuilder* builder) override;
        TextureBase* CreateTexture(TextureBuilder* builder) override;
        TextureViewBase* CreateTextureView(TextureViewBuilder* builder) override;

        void TickImpl() override;

        ComPtr<IDXGIFactory4> GetFactory();
        ComPtr<ID3D12Device> GetD3D12Device();
        ComPtr<ID3D12CommandQueue> GetCommandQueue();

        DescriptorHeapAllocator* GetDescriptorHeapAllocator();
        MapRequestTracker* GetMapRequestTracker() const;
        ResourceAllocator* GetResourceAllocator();
        ResourceUploader* GetResourceUploader();

        void OpenCommandList(ComPtr<ID3D12GraphicsCommandList>* commandList);
        ComPtr<ID3D12GraphicsCommandList> GetPendingCommandList();

        uint64_t GetSerial() const;
        void NextSerial();
        void WaitForSerial(uint64_t serial);

        void ReferenceUntilUnused(ComPtr<IUnknown> object);

        void ExecuteCommandLists(std::initializer_list<ID3D12CommandList*> commandLists);

      private:
        ResultOrError<BindGroupLayoutBase*> CreateBindGroupLayoutImpl(
            const dawn::BindGroupLayoutDescriptor* descriptor) override;
        ResultOrError<PipelineLayoutBase*> CreatePipelineLayoutImpl(
            const dawn::PipelineLayoutDescriptor* descriptor) override;
        ResultOrError<QueueBase*> CreateQueueImpl() override;
        ResultOrError<SamplerBase*> CreateSamplerImpl(
            const dawn::SamplerDescriptor* descriptor) override;

        uint64_t mSerial = 0;
        ComPtr<ID3D12Fence> mFence;
        HANDLE mFenceEvent;

        ComPtr<IDXGIFactory4> mFactory;
        ComPtr<IDXGIAdapter1> mHardwareAdapter;
        ComPtr<ID3D12Device> mD3d12Device;
        ComPtr<ID3D12CommandQueue> mCommandQueue;

        CommandAllocatorManager* mCommandAllocatorManager = nullptr;
        DescriptorHeapAllocator* mDescriptorHeapAllocator = nullptr;
        MapRequestTracker* mMapRequestTracker = nullptr;
        ResourceAllocator* mResourceAllocator = nullptr;
        ResourceUploader* mResourceUploader = nullptr;

        struct PendingCommandList {
            ComPtr<ID3D12GraphicsCommandList> commandList;
            bool open = false;
        } mPendingCommands;

        SerialQueue<ComPtr<IUnknown>> mUsedComObjectRefs;
    };

}}  // namespace backend::d3d12

#endif  // BACKEND_D3D12_DEVICED3D12_H_
