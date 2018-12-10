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

#ifndef DAWNNATIVE_D3D12_DEVICED3D12_H_
#define DAWNNATIVE_D3D12_DEVICED3D12_H_

#include "dawn_native/dawn_platform.h"

#include "common/SerialQueue.h"
#include "dawn_native/Device.h"
#include "dawn_native/d3d12/Forward.h"
#include "dawn_native/d3d12/d3d12_platform.h"

#include <memory>

namespace dawn_native { namespace d3d12 {

    class CommandAllocatorManager;
    class DescriptorHeapAllocator;
    class MapRequestTracker;
    class PlatformFunctions;
    class ResourceAllocator;
    class ResourceUploader;

    void ASSERT_SUCCESS(HRESULT hr);

    // Definition of backend types
    class Device : public DeviceBase {
      public:
        Device();
        ~Device();

        BlendStateBase* CreateBlendState(BlendStateBuilder* builder) override;
        CommandBufferBase* CreateCommandBuffer(CommandBufferBuilder* builder) override;
        DepthStencilStateBase* CreateDepthStencilState(DepthStencilStateBuilder* builder) override;
        InputStateBase* CreateInputState(InputStateBuilder* builder) override;
        RenderPassDescriptorBase* CreateRenderPassDescriptor(
            RenderPassDescriptorBuilder* builder) override;
        SwapChainBase* CreateSwapChain(SwapChainBuilder* builder) override;

        Serial GetCompletedCommandSerial() const final override;
        Serial GetLastSubmittedCommandSerial() const final override;
        void TickImpl() override;

        const dawn_native::PCIInfo& GetPCIInfo() const override;

        ComPtr<IDXGIFactory4> GetFactory();
        ComPtr<ID3D12Device> GetD3D12Device();
        ComPtr<ID3D12CommandQueue> GetCommandQueue();

        DescriptorHeapAllocator* GetDescriptorHeapAllocator();
        MapRequestTracker* GetMapRequestTracker() const;
        const PlatformFunctions* GetFunctions();
        ResourceAllocator* GetResourceAllocator();
        ResourceUploader* GetResourceUploader();

        void OpenCommandList(ComPtr<ID3D12GraphicsCommandList>* commandList);
        ComPtr<ID3D12GraphicsCommandList> GetPendingCommandList();
        Serial GetPendingCommandSerial() const;

        void NextSerial();
        void WaitForSerial(Serial serial);

        void ReferenceUntilUnused(ComPtr<IUnknown> object);

        void ExecuteCommandLists(std::initializer_list<ID3D12CommandList*> commandLists);

      private:
        ResultOrError<BindGroupBase*> CreateBindGroupImpl(
            const BindGroupDescriptor* descriptor) override;
        ResultOrError<BindGroupLayoutBase*> CreateBindGroupLayoutImpl(
            const BindGroupLayoutDescriptor* descriptor) override;
        ResultOrError<BufferBase*> CreateBufferImpl(const BufferDescriptor* descriptor) override;
        ResultOrError<ComputePipelineBase*> CreateComputePipelineImpl(
            const ComputePipelineDescriptor* descriptor) override;
        ResultOrError<PipelineLayoutBase*> CreatePipelineLayoutImpl(
            const PipelineLayoutDescriptor* descriptor) override;
        ResultOrError<QueueBase*> CreateQueueImpl() override;
        ResultOrError<RenderPipelineBase*> CreateRenderPipelineImpl(
            const RenderPipelineDescriptor* descriptor) override;
        ResultOrError<SamplerBase*> CreateSamplerImpl(const SamplerDescriptor* descriptor) override;
        ResultOrError<ShaderModuleBase*> CreateShaderModuleImpl(
            const ShaderModuleDescriptor* descriptor) override;
        ResultOrError<TextureBase*> CreateTextureImpl(const TextureDescriptor* descriptor) override;
        ResultOrError<TextureViewBase*> CreateTextureViewImpl(
            TextureBase* texture,
            const TextureViewDescriptor* descriptor) override;
        void CollectPCIInfo();

        // Keep mFunctions as the first member so that in the destructor it is freed. Otherwise the
        // D3D12 DLLs are unloaded before we are done using it.
        std::unique_ptr<PlatformFunctions> mFunctions;

        Serial mCompletedSerial = 0;
        Serial mLastSubmittedSerial = 0;
        ComPtr<ID3D12Fence> mFence;
        HANDLE mFenceEvent;

        ComPtr<IDXGIFactory4> mFactory;
        ComPtr<IDXGIAdapter1> mHardwareAdapter;
        ComPtr<ID3D12Device> mD3d12Device;
        ComPtr<ID3D12CommandQueue> mCommandQueue;

        struct PendingCommandList {
            ComPtr<ID3D12GraphicsCommandList> commandList;
            bool open = false;
        } mPendingCommands;

        SerialQueue<ComPtr<IUnknown>> mUsedComObjectRefs;

        std::unique_ptr<CommandAllocatorManager> mCommandAllocatorManager;
        std::unique_ptr<DescriptorHeapAllocator> mDescriptorHeapAllocator;
        std::unique_ptr<MapRequestTracker> mMapRequestTracker;
        std::unique_ptr<ResourceAllocator> mResourceAllocator;
        std::unique_ptr<ResourceUploader> mResourceUploader;

        dawn_native::PCIInfo mPCIInfo;
    };

}}  // namespace dawn_native::d3d12

#endif  // DAWNNATIVE_D3D12_DEVICED3D12_H_
