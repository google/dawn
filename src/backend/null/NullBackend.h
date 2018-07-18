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

#ifndef BACKEND_NULL_NULLBACKEND_H_
#define BACKEND_NULL_NULLBACKEND_H_

#include "dawn/dawncpp.h"

#include "backend/BindGroup.h"
#include "backend/BindGroupLayout.h"
#include "backend/BlendState.h"
#include "backend/Buffer.h"
#include "backend/CommandBuffer.h"
#include "backend/ComputePipeline.h"
#include "backend/DepthStencilState.h"
#include "backend/Device.h"
#include "backend/InputState.h"
#include "backend/PipelineLayout.h"
#include "backend/Queue.h"
#include "backend/RenderPassDescriptor.h"
#include "backend/RenderPipeline.h"
#include "backend/Sampler.h"
#include "backend/ShaderModule.h"
#include "backend/SwapChain.h"
#include "backend/Texture.h"
#include "backend/ToBackend.h"

namespace backend { namespace null {

    using BindGroup = BindGroupBase;
    using BindGroupLayout = BindGroupLayoutBase;
    using BlendState = BlendStateBase;
    class Buffer;
    using BufferView = BufferViewBase;
    class CommandBuffer;
    using ComputePipeline = ComputePipelineBase;
    using DepthStencilState = DepthStencilStateBase;
    class Device;
    using InputState = InputStateBase;
    using PipelineLayout = PipelineLayoutBase;
    class Queue;
    using RenderPassDescriptor = RenderPassDescriptorBase;
    using RenderPipeline = RenderPipelineBase;
    using Sampler = SamplerBase;
    using ShaderModule = ShaderModuleBase;
    class SwapChain;
    using Texture = TextureBase;
    using TextureView = TextureViewBase;

    struct NullBackendTraits {
        using BindGroupType = BindGroup;
        using BindGroupLayoutType = BindGroupLayout;
        using BlendStateType = BlendState;
        using BufferType = Buffer;
        using BufferViewType = BufferView;
        using CommandBufferType = CommandBuffer;
        using ComputePipelineType = ComputePipeline;
        using DepthStencilStateType = DepthStencilState;
        using DeviceType = Device;
        using InputStateType = InputState;
        using PipelineLayoutType = PipelineLayout;
        using QueueType = Queue;
        using RenderPassDescriptorType = RenderPassDescriptor;
        using RenderPipelineType = RenderPipeline;
        using SamplerType = Sampler;
        using ShaderModuleType = ShaderModule;
        using SwapChainType = SwapChain;
        using TextureType = Texture;
        using TextureViewType = TextureView;
    };

    template <typename T>
    auto ToBackend(T&& common) -> decltype(ToBackendBase<NullBackendTraits>(common)) {
        return ToBackendBase<NullBackendTraits>(common);
    }

    struct PendingOperation {
        virtual ~PendingOperation() = default;
        virtual void Execute() = 0;
    };

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

        void AddPendingOperation(std::unique_ptr<PendingOperation> operation);
        std::vector<std::unique_ptr<PendingOperation>> AcquirePendingOperations();

      private:
        ResultOrError<BindGroupLayoutBase*> CreateBindGroupLayoutImpl(
            const dawn::BindGroupLayoutDescriptor* descriptor) override;
        ResultOrError<PipelineLayoutBase*> CreatePipelineLayoutImpl(
            const dawn::PipelineLayoutDescriptor* descriptor) override;
        ResultOrError<QueueBase*> CreateQueueImpl() override;
        ResultOrError<SamplerBase*> CreateSamplerImpl(
            const dawn::SamplerDescriptor* descriptor) override;

        std::vector<std::unique_ptr<PendingOperation>> mPendingOperations;
    };

    class Buffer : public BufferBase {
      public:
        Buffer(BufferBuilder* builder);
        ~Buffer();

        void MapReadOperationCompleted(uint32_t serial, void* ptr, bool isWrite);

      private:
        void SetSubDataImpl(uint32_t start, uint32_t count, const uint8_t* data) override;
        void MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) override;
        void MapWriteAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) override;
        void UnmapImpl() override;

        void MapAsyncImplCommon(uint32_t serial, uint32_t start, uint32_t count, bool isWrite);

        std::unique_ptr<char[]> mBackingData;
    };

    class CommandBuffer : public CommandBufferBase {
      public:
        CommandBuffer(CommandBufferBuilder* builder);
        ~CommandBuffer();

      private:
        CommandIterator mCommands;
    };

    class Queue : public QueueBase {
      public:
        Queue(Device* device);
        ~Queue();

        // Dawn API
        void Submit(uint32_t numCommands, CommandBuffer* const* commands);
    };

    class SwapChain : public SwapChainBase {
      public:
        SwapChain(SwapChainBuilder* builder);
        ~SwapChain();

      protected:
        TextureBase* GetNextTextureImpl(TextureBuilder* builder) override;
        void OnBeforePresent(TextureBase*) override;
    };

}}  // namespace backend::null

#endif  // BACKEND_NULL_NULLBACKEND_H_
