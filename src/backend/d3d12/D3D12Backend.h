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

#ifndef BACKEND_D3D12_D3D12BACKEND_H_
#define BACKEND_D3D12_D3D12BACKEND_H_

#include "nxt/nxtcpp.h"

#include "backend/DepthStencilState.h"
#include "backend/Device.h"
#include "backend/RenderPass.h"
#include "backend/ToBackend.h"

#include "backend/d3d12/d3d12_platform.h"

namespace backend {
namespace d3d12 {

    class BindGroup;
    class BindGroupLayout;
    class BlendState;
    class Buffer;
    class BufferView;
    class CommandBuffer;
    class ComputePipeline;
    class DepthStencilState;
    class Device;
    class Framebuffer;
    class InputState;
    class PipelineLayout;
    class Queue;
    class RenderPass;
    class RenderPipeline;
    class Sampler;
    class ShaderModule;
    class SwapChain;
    class Texture;
    class TextureView;

    class CommandAllocatorManager;
    class DescriptorHeapAllocator;
    class MapReadRequestTracker;
    class ResourceAllocator;
    class ResourceUploader;

    struct D3D12BackendTraits {
        using BindGroupType = BindGroup;
        using BindGroupLayoutType = BindGroupLayout;
        using BlendStateType = BlendState;
        using BufferType = Buffer;
        using BufferViewType = BufferView;
        using CommandBufferType = CommandBuffer;
        using ComputePipelineType = ComputePipeline;
        using DepthStencilStateType = DepthStencilState;
        using DeviceType = Device;
        using FramebufferType = Framebuffer;
        using InputStateType = InputState;
        using PipelineLayoutType = PipelineLayout;
        using QueueType = Queue;
        using RenderPassType = RenderPass;
        using RenderPipelineType = RenderPipeline;
        using SamplerType = Sampler;
        using ShaderModuleType = ShaderModule;
        using SwapChainType = SwapChain;
        using TextureType = Texture;
        using TextureViewType = TextureView;
    };

    template<typename T>
    auto ToBackend(T&& common) -> decltype(ToBackendBase<D3D12BackendTraits>(common)) {
        return ToBackendBase<D3D12BackendTraits>(common);
    }

    void ASSERT_SUCCESS(HRESULT hr);

    // Definition of backend types
    class Device : public DeviceBase {
        public:
            Device(ComPtr<ID3D12Device> d3d12Device);
            ~Device();

            BindGroupBase* CreateBindGroup(BindGroupBuilder* builder) override;
            BindGroupLayoutBase* CreateBindGroupLayout(BindGroupLayoutBuilder* builder) override;
            BlendStateBase* CreateBlendState(BlendStateBuilder* builder) override;
            BufferBase* CreateBuffer(BufferBuilder* builder) override;
            BufferViewBase* CreateBufferView(BufferViewBuilder* builder) override;
            CommandBufferBase* CreateCommandBuffer(CommandBufferBuilder* builder) override;
            ComputePipelineBase* CreateComputePipeline(ComputePipelineBuilder* builder) override;
            DepthStencilStateBase* CreateDepthStencilState(DepthStencilStateBuilder* builder) override;
            FramebufferBase* CreateFramebuffer(FramebufferBuilder* builder) override;
            InputStateBase* CreateInputState(InputStateBuilder* builder) override;
            PipelineLayoutBase* CreatePipelineLayout(PipelineLayoutBuilder* builder) override;
            QueueBase* CreateQueue(QueueBuilder* builder) override;
            RenderPassBase* CreateRenderPass(RenderPassBuilder* builder) override;
            RenderPipelineBase* CreateRenderPipeline(RenderPipelineBuilder* builder) override;
            SamplerBase* CreateSampler(SamplerBuilder* builder) override;
            ShaderModuleBase* CreateShaderModule(ShaderModuleBuilder* builder) override;
            SwapChainBase* CreateSwapChain(SwapChainBuilder* builder) override;
            TextureBase* CreateTexture(TextureBuilder* builder) override;
            TextureViewBase* CreateTextureView(TextureViewBuilder* builder) override;

            void TickImpl() override;

            ComPtr<ID3D12Device> GetD3D12Device();
            ComPtr<ID3D12CommandQueue> GetCommandQueue();

            DescriptorHeapAllocator* GetDescriptorHeapAllocator();
            MapReadRequestTracker* GetMapReadRequestTracker() const;
            ResourceAllocator* GetResourceAllocator();
            ResourceUploader* GetResourceUploader();

            void OpenCommandList(ComPtr<ID3D12GraphicsCommandList>* commandList);
            ComPtr<ID3D12GraphicsCommandList> GetPendingCommandList();

            uint64_t GetSerial() const;
            void NextSerial();
            void WaitForSerial(uint64_t serial);

            void ExecuteCommandLists(std::initializer_list<ID3D12CommandList*> commandLists);

        private:
            uint64_t serial = 0;
            ComPtr<ID3D12Fence> fence;
            HANDLE fenceEvent;

            ComPtr<ID3D12Device> d3d12Device;
            ComPtr<ID3D12CommandQueue> commandQueue;

            CommandAllocatorManager* commandAllocatorManager;
            DescriptorHeapAllocator* descriptorHeapAllocator;
            MapReadRequestTracker* mapReadRequestTracker;
            ResourceAllocator* resourceAllocator;
            ResourceUploader* resourceUploader;

            struct PendingCommandList {
                ComPtr<ID3D12GraphicsCommandList> commandList;
                bool open = false;
            } pendingCommands;
    };

    class RenderPass : public RenderPassBase {
        public:
            RenderPass(Device* device, RenderPassBuilder* builder);

        private:
            Device* device;
    };

}
}

#endif // BACKEND_D3D12_D3D12BACKEND_H_
