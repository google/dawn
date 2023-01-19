// Copyright 2018 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_FORWARD_H_
#define SRC_DAWN_NATIVE_VULKAN_FORWARD_H_

#include "dawn/native/ToBackend.h"

namespace dawn::native::vulkan {

class Adapter;
class BindGroup;
class BindGroupLayout;
class Buffer;
class CommandBuffer;
class ComputePipeline;
class Device;
class PipelineCache;
class PipelineLayout;
class QuerySet;
class Queue;
class RenderPipeline;
class ResourceHeap;
class Sampler;
class ShaderModule;
class SwapChain;
class Texture;
class TextureView;

struct VulkanBackendTraits {
    using AdapterType = Adapter;
    using BindGroupType = BindGroup;
    using BindGroupLayoutType = BindGroupLayout;
    using BufferType = Buffer;
    using CommandBufferType = CommandBuffer;
    using ComputePipelineType = ComputePipeline;
    using DeviceType = Device;
    using PipelineCacheType = PipelineCache;
    using PipelineLayoutType = PipelineLayout;
    using QuerySetType = QuerySet;
    using QueueType = Queue;
    using RenderPipelineType = RenderPipeline;
    using ResourceHeapType = ResourceHeap;
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

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_FORWARD_H_
