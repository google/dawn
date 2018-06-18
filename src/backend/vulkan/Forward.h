// Copyright 2018 The NXT Authors
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

#ifndef BACKEND_VULKAN_FORWARD_H_
#define BACKEND_VULKAN_FORWARD_H_

#include "backend/ToBackend.h"

namespace backend {
    class BufferViewBase;
}

namespace backend { namespace vulkan {

    class BindGroup;
    class BindGroupLayout;
    class BlendState;
    class Buffer;
    using BufferView = BufferViewBase;
    class CommandBuffer;
    class ComputePipeline;
    class DepthStencilState;
    class Device;
    class InputState;
    class PipelineLayout;
    class Queue;
    class RenderPassDescriptor;
    class RenderPipeline;
    class Sampler;
    class ShaderModule;
    class SwapChain;
    class Texture;
    class TextureView;

    struct VulkanBackendTraits {
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
    auto ToBackend(T&& common) -> decltype(ToBackendBase<VulkanBackendTraits>(common)) {
        return ToBackendBase<VulkanBackendTraits>(common);
    }

}}  // namespace backend::vulkan

#endif  // BACKEND_VULKAN_FORWARD_H_
