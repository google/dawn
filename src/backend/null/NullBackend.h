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

#ifndef BACKEND_NULL_NULLBACKEND_H_
#define BACKEND_NULL_NULLBACKEND_H_

#include "nxt/nxtcpp.h"

#include "common/Buffer.h"
#include "common/BindGroup.h"
#include "common/BindGroupLayout.h"
#include "common/Device.h"
#include "common/CommandBuffer.h"
#include "common/DepthStencilState.h"
#include "common/InputState.h"
#include "common/Framebuffer.h"
#include "common/Pipeline.h"
#include "common/PipelineLayout.h"
#include "common/Queue.h"
#include "common/RenderPass.h"
#include "common/Sampler.h"
#include "common/ShaderModule.h"
#include "common/Texture.h"
#include "common/ToBackend.h"

namespace backend {
namespace null {

    using BindGroup = BindGroupBase;
    using BindGroupLayout = BindGroupLayoutBase;
    class Buffer;
    using BufferView = BufferViewBase;
    using CommandBuffer = CommandBufferBase;
    using DepthStencilState = DepthStencilStateBase;
    class Device;
    using InputState = InputStateBase;
    using Framebuffer = FramebufferBase;
    using Pipeline = PipelineBase;
    using PipelineLayout = PipelineLayoutBase;
    class Queue;
    using RenderPass = RenderPassBase;
    using Sampler = SamplerBase;
    using ShaderModule = ShaderModuleBase;
    using Texture = TextureBase;
    using TextureView = TextureViewBase;

    struct NullBackendTraits {
        using BindGroupType = BindGroup;
        using BindGroupLayoutType = BindGroupLayout;
        using BufferType = Buffer;
        using BufferViewType = BufferView;
        using CommandBufferType = CommandBuffer;
        using DepthStencilStateType = DepthStencilState;
        using DeviceType = Device;
        using InputStateType = InputState;
        using FramebufferType = Framebuffer;
        using PipelineType = Pipeline;
        using PipelineLayoutType = PipelineLayout;
        using QueueType = Queue;
        using RenderPassType = RenderPass;
        using SamplerType = Sampler;
        using ShaderModuleType = ShaderModule;
        using TextureType = Texture;
        using TextureViewType = TextureView;
    };

    template<typename T>
    auto ToBackend(T&& common) -> decltype(ToBackendBase<NullBackendTraits>(common)) {
        return ToBackendBase<NullBackendTraits>(common);
    }

    class Device : public DeviceBase {
        public:
            Device();
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

            // NXT API
            void Reference();
            void Release();
    };

    class Buffer : public BufferBase {
        public:
            Buffer(BufferBuilder* builder);
            ~Buffer();

        private:
            void SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) override;
            void MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) override;
            void UnmapImpl() override;
    };

    class Queue : public QueueBase {
        public:
            Queue(QueueBuilder* builder);
            ~Queue();

            // NXT API
            void Submit(uint32_t numCommands, CommandBuffer* const * commands);
    };

}
}

#endif // BACKEND_NULL_NULLBACKEND_H_
