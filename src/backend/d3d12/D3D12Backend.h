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

#include "backend/Buffer.h"
#include "backend/BindGroup.h"
#include "backend/BindGroupLayout.h"
#include "backend/Device.h"
#include "backend/Framebuffer.h"
#include "backend/DepthStencilState.h"
#include "backend/InputState.h"
#include "backend/PipelineLayout.h"
#include "backend/Queue.h"
#include "backend/RenderPass.h"
#include "backend/Sampler.h"
#include "backend/Texture.h"
#include "backend/ToBackend.h"

#include "backend/d3d12/d3d12_platform.h"

namespace backend {
namespace d3d12 {

    class BindGroup;
    class BindGroupLayout;
    class Buffer;
    class BufferView;
    class CommandBuffer;
    class DepthStencilState;
    class Device;
    class InputState;
    class Pipeline;
    class PipelineLayout;
    class Queue;
    class Sampler;
    class ShaderModule;
    class Texture;
    class TextureView;
    class Framebuffer;
    class RenderPass;

    class CommandAllocatorManager;
    class DescriptorHeapAllocator;
    class MapReadRequestTracker;
    class ResourceAllocator;
    class ResourceUploader;

    struct D3D12BackendTraits {
        using BindGroupType = BindGroup;
        using BindGroupLayoutType = BindGroupLayout;
        using BufferType = Buffer;
        using BufferViewType = BufferView;
        using CommandBufferType = CommandBuffer;
        using DepthStencilStateType = DepthStencilState;
        using DeviceType = Device;
        using InputStateType = InputState;
        using PipelineType = Pipeline;
        using PipelineLayoutType = PipelineLayout;
        using QueueType = Queue;
        using SamplerType = Sampler;
        using ShaderModuleType = ShaderModule;
        using TextureType = Texture;
        using TextureViewType = TextureView;
        using FramebufferType = Framebuffer;
        using RenderPassType = RenderPass;
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
            BufferBase* CreateBuffer(BufferBuilder* builder) override;
            BufferViewBase* CreateBufferView(BufferViewBuilder* builder) override;
            CommandBufferBase* CreateCommandBuffer(CommandBufferBuilder* builder) override;
            DepthStencilStateBase* CreateDepthStencilState(DepthStencilStateBuilder* builder) override;
            InputStateBase* CreateInputState(InputStateBuilder* builder) override;
            FramebufferBase* CreateFramebuffer(FramebufferBuilder* builder) override;
            PipelineBase* CreatePipeline(PipelineBuilder* builder) override;
            PipelineLayoutBase* CreatePipelineLayout(PipelineLayoutBuilder* builder) override;
            QueueBase* CreateQueue(QueueBuilder* builder) override;
            RenderPassBase* CreateRenderPass(RenderPassBuilder* builder) override;
            SamplerBase* CreateSampler(SamplerBuilder* builder) override;
            ShaderModuleBase* CreateShaderModule(ShaderModuleBuilder* builder) override;
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

            ComPtr<ID3D12Resource> GetCurrentTexture();
            ComPtr<ID3D12Resource> GetCurrentDepthTexture();
            void SetNextTexture(ComPtr<ID3D12Resource> resource, ComPtr<ID3D12Resource> depthResource);

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

            ComPtr<ID3D12Resource> nextTexture;
            ComPtr<ID3D12Resource> nextDepthTexture;
    };

    class DepthStencilState : public DepthStencilStateBase {
        public:
            DepthStencilState(Device* device, DepthStencilStateBuilder* builder);

        private:
            Device* device;
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
