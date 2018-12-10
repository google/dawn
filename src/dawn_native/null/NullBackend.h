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

#ifndef DAWNNATIVE_NULL_NULLBACKEND_H_
#define DAWNNATIVE_NULL_NULLBACKEND_H_

#include "dawn_native/dawn_platform.h"

#include "dawn_native/BindGroup.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/BlendState.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandBuffer.h"
#include "dawn_native/ComputePipeline.h"
#include "dawn_native/DepthStencilState.h"
#include "dawn_native/Device.h"
#include "dawn_native/InputState.h"
#include "dawn_native/PipelineLayout.h"
#include "dawn_native/Queue.h"
#include "dawn_native/RenderPassDescriptor.h"
#include "dawn_native/RenderPipeline.h"
#include "dawn_native/Sampler.h"
#include "dawn_native/ShaderModule.h"
#include "dawn_native/SwapChain.h"
#include "dawn_native/Texture.h"
#include "dawn_native/ToBackend.h"

namespace dawn_native { namespace null {

    using BindGroup = BindGroupBase;
    using BindGroupLayout = BindGroupLayoutBase;
    using BlendState = BlendStateBase;
    class Buffer;
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

        void AddPendingOperation(std::unique_ptr<PendingOperation> operation);
        void SubmitPendingOperations();

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
        void InitFakePCIInfo();

        Serial mCompletedSerial = 0;
        Serial mLastSubmittedSerial = 0;
        std::vector<std::unique_ptr<PendingOperation>> mPendingOperations;
        dawn_native::PCIInfo mPCIInfo;
    };

    class Buffer : public BufferBase {
      public:
        Buffer(Device* device, const BufferDescriptor* descriptor);
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

      private:
        void SubmitImpl(uint32_t numCommands, CommandBufferBase* const* commands) override;
    };

    class SwapChain : public SwapChainBase {
      public:
        SwapChain(SwapChainBuilder* builder);
        ~SwapChain();

      protected:
        TextureBase* GetNextTextureImpl(const TextureDescriptor* descriptor) override;
        void OnBeforePresent(TextureBase*) override;
    };

}}  // namespace dawn_native::null

#endif  // DAWNNATIVE_NULL_NULLBACKEND_H_
