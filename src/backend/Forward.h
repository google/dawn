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

#ifndef BACKEND_FORWARD_H_
#define BACKEND_FORWARD_H_

#include <cstdint>

namespace backend {

    class BindGroupBase;
    class BindGroupBuilder;
    class BindGroupLayoutBase;
    class BindGroupLayoutBuilder;
    class BlendStateBase;
    class BlendStateBuilder;
    class BufferBase;
    class BufferBuilder;
    class BufferViewBase;
    class BufferViewBuilder;
    class ComputePipelineBase;
    class ComputePipelineBuilder;
    class CommandBufferBase;
    class CommandBufferBuilder;
    class DepthStencilStateBase;
    class DepthStencilStateBuilder;
    class InputStateBase;
    class InputStateBuilder;
    class PipelineLayoutBase;
    class PipelineLayoutBuilder;
    class QueueBase;
    class QueueBuilder;
    class RenderPassDescriptorBase;
    class RenderPassDescriptorBuilder;
    class RenderPipelineBase;
    class RenderPipelineBuilder;
    class SamplerBase;
    class SamplerBuilder;
    class ShaderModuleBase;
    class ShaderModuleBuilder;
    class SwapChainBase;
    class SwapChainBuilder;
    class TextureBase;
    class TextureBuilder;
    class TextureViewBase;
    class TextureViewBuilder;

    class DeviceBase;

    template <typename T>
    class Ref;

    template <typename T>
    class PerStage;

    enum PushConstantType : uint8_t;
}  // namespace backend

#endif  // BACKEND_FORWARD_H_
