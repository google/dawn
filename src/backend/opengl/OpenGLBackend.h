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

#ifndef BACKEND_OPENGL_OPENGLBACKEND_H_
#define BACKEND_OPENGL_OPENGLBACKEND_H_

#include "nxt/nxtcpp.h"

#include "common/Buffer.h"
#include "common/BindGroup.h"
#include "common/BindGroupLayout.h"
#include "common/Device.h"
#include "common/InputState.h"
#include "common/Queue.h"
#include "common/ToBackend.h"

#include "glad/glad.h"

namespace backend {
namespace opengl {

    class BindGroup;
    class BindGroupLayout;
    class Buffer;
    class BufferView;
    class CommandBuffer;
    class InputState;
    class Pipeline;
    class PipelineLayout;
    class Queue;
    class Sampler;
    class ShaderModule;
    class Texture;
    class TextureView;

    struct OpenGLBackendTraits {
        using BindGroupType = BindGroup;
        using BindGroupLayoutType = BindGroupLayout;
        using BufferType = Buffer;
        using BufferViewType = BufferView;
        using CommandBufferType = CommandBuffer;
        using InputStateType = InputState;
        using PipelineType = Pipeline;
        using PipelineLayoutType = PipelineLayout;
        using QueueType = Queue;
        using SamplerType = Sampler;
        using ShaderModuleType = ShaderModule;
        using TextureType = Texture;
        using TextureViewType = TextureView;
    };

    template<typename T>
    auto ToBackend(T&& common) -> decltype(ToBackendBase<OpenGLBackendTraits>(common)) {
        return ToBackendBase<OpenGLBackendTraits>(common);
    }

    // Definition of backend types
    class Device : public DeviceBase {
        public:
            BindGroupBase* CreateBindGroup(BindGroupBuilder* builder) override;
            BindGroupLayoutBase* CreateBindGroupLayout(BindGroupLayoutBuilder* builder) override;
            BufferBase* CreateBuffer(BufferBuilder* builder) override;
            BufferViewBase* CreateBufferView(BufferViewBuilder* builder) override;
            CommandBufferBase* CreateCommandBuffer(CommandBufferBuilder* builder) override;
            InputStateBase* CreateInputState(InputStateBuilder* builder) override;
            PipelineBase* CreatePipeline(PipelineBuilder* builder) override;
            PipelineLayoutBase* CreatePipelineLayout(PipelineLayoutBuilder* builder) override;
            QueueBase* CreateQueue(QueueBuilder* builder) override;
            SamplerBase* CreateSampler(SamplerBuilder* builder) override;
            ShaderModuleBase* CreateShaderModule(ShaderModuleBuilder* builder) override;
            TextureBase* CreateTexture(TextureBuilder* builder) override;
            TextureViewBase* CreateTextureView(TextureViewBuilder* builder) override;

            // NXT API
            void Reference();
            void Release();
    };

    class BindGroup : public BindGroupBase {
        public:
            BindGroup(Device* device, BindGroupBuilder* builder);

        private:
            Device* device;
    };

    class BindGroupLayout : public BindGroupLayoutBase {
        public:
            BindGroupLayout(Device* device, BindGroupLayoutBuilder* builder);

        private:
            Device* device;
    };

    class Buffer : public BufferBase {
        public:
            Buffer(Device* device, BufferBuilder* builder);

            GLuint GetHandle() const;

        private:
            void SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) override;

            Device* device;
            GLuint buffer = 0;
    };

    class BufferView : public BufferViewBase {
        public:
            BufferView(Device* device, BufferViewBuilder* builder);

        private:
            Device* device;
    };

    class InputState : public InputStateBase {
        public:
            InputState(Device* device, InputStateBuilder* builder);
            GLuint GetVAO();

        private:
            Device* device;
            GLuint vertexArrayObject;
    };

    class Queue : public QueueBase {
        public:
            Queue(Device* device, QueueBuilder* builder);

            // NXT API
            void Submit(uint32_t numCommands, CommandBuffer* const * commands);

        private:
            Device* device;
    };

}
}

#endif // BACKEND_OPENGL_OPENGLBACKEND_H_
