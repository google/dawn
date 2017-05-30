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

#ifndef BACKEND_COMMON_FORWARD_H_
#define BACKEND_COMMON_FORWARD_H_

#include <cassert>
#include <cstdint>

#define ASSERT assert

namespace backend {

    class BindGroupBase;
    class BindGroupBuilder;
    class BindGroupLayoutBase;
    class BindGroupLayoutBuilder;
    class BufferBase;
    class BufferBuilder;
    class BufferViewBase;
    class BufferViewBuilder;
    class CommandBufferBase;
    class CommandBufferBuilder;
    class DepthStencilStateBase;
    class DepthStencilStateBuilder;
    class FramebufferBase;
    class FramebufferBuilder;
    class InputStateBase;
    class InputStateBuilder;
    class PipelineBase;
    class PipelineBuilder;
    class PipelineLayoutBase;
    class PipelineLayoutBuilder;
    class QueueBase;
    class QueueBuilder;
    class RenderPassBase;
    class RenderPassBuilder;
    class SamplerBase;
    class SamplerBuilder;
    class ShaderModuleBase;
    class ShaderModuleBuilder;
    class TextureBase;
    class TextureBuilder;
    class TextureViewBase;
    class TextureViewBuilder;

    class DeviceBase;

    template<typename T>
    class Ref;

    template<typename T>
    class PerStage;

    // TODO(cwallez@chromium.org): where should constants live?
    static constexpr uint32_t kMaxPushConstants = 32u;
    static constexpr uint32_t kMaxBindGroups = 4u;
    static constexpr uint32_t kMaxBindingsPerGroup = 16u; // TODO(cwallez@chromium.org): investigate bindgroup limits
    static constexpr uint32_t kMaxVertexAttributes = 16u;
    static constexpr uint32_t kMaxVertexInputs = 16u;
    static constexpr uint32_t kNumStages = 3;
    static constexpr uint32_t kMaxColorAttachments = 4u;

    enum PushConstantType : uint8_t;
}

#endif // BACKEND_COMMON_FORWARD_H_
