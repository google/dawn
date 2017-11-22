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

#ifndef BACKEND_VULKAN_VULKANBACKEND_H_
#define BACKEND_VULKAN_VULKANBACKEND_H_

#include "nxt/nxtcpp.h"

#include "backend/vulkan/VulkanFunctions.h"
#include "backend/vulkan/VulkanInfo.h"
#include "backend/BindGroup.h"
#include "backend/BindGroupLayout.h"
#include "backend/BlendState.h"
#include "backend/Device.h"
#include "backend/CommandBuffer.h"
#include "backend/ComputePipeline.h"
#include "backend/DepthStencilState.h"
#include "backend/Framebuffer.h"
#include "backend/InputState.h"
#include "backend/PipelineLayout.h"
#include "backend/Queue.h"
#include "backend/RenderPass.h"
#include "backend/RenderPipeline.h"
#include "backend/Sampler.h"
#include "backend/ShaderModule.h"
#include "backend/SwapChain.h"
#include "backend/Texture.h"
#include "backend/ToBackend.h"
#include "common/DynamicLib.h"
#include "common/Serial.h"

namespace backend {
namespace vulkan {

    using BindGroup = BindGroupBase;
    using BindGroupLayout = BindGroupLayoutBase;
    using BlendState = BlendStateBase;
    class Buffer;
    using BufferView = BufferViewBase;
    using CommandBuffer = CommandBufferBase;
    using ComputePipeline = ComputePipelineBase;
    using DepthStencilState = DepthStencilStateBase;
    class Device;
    using Framebuffer = FramebufferBase;
    using InputState = InputStateBase;
    using PipelineLayout = PipelineLayoutBase;
    class Queue;
    using RenderPass = RenderPassBase;
    using RenderPipeline = RenderPipelineBase;
    using Sampler = SamplerBase;
    using ShaderModule = ShaderModuleBase;
    class SwapChain;
    class Texture;
    using TextureView = TextureViewBase;

    class MapReadRequestTracker;
    class MemoryAllocator;

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
    auto ToBackend(T&& common) -> decltype(ToBackendBase<VulkanBackendTraits>(common)) {
        return ToBackendBase<VulkanBackendTraits>(common);
    }

    class Device : public DeviceBase {
        public:
            Device();
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

            const VulkanDeviceInfo& GetDeviceInfo() const;
            MapReadRequestTracker* GetMapReadRequestTracker() const;
            MemoryAllocator* GetMemoryAllocator() const;
            Serial GetSerial() const;

            // Contains all the Vulkan entry points, vkDoFoo is called via device->fn.DoFoo.
            const VulkanFunctions fn;

            VkInstance GetInstance() const;
            VkDevice GetVkDevice() const;

        private:
            bool CreateInstance(VulkanGlobalKnobs* usedKnobs);
            bool CreateDevice(VulkanDeviceKnobs* usedKnobs);
            void GatherQueueFromDevice();

            bool RegisterDebugReport();
            static VkBool32 OnDebugReportCallback(VkDebugReportFlagsEXT flags,
                                                  VkDebugReportObjectTypeEXT objectType,
                                                  uint64_t object,
                                                  size_t location,
                                                  int32_t messageCode,
                                                  const char* pLayerPrefix,
                                                  const char* pMessage,
                                                  void* pUserdata);

            // To make it easier to use fn it is a public const member. However
            // the Device is allowed to mutate them through these private methods.
            VulkanFunctions* GetMutableFunctions();

            VulkanGlobalInfo globalInfo;
            VulkanDeviceInfo deviceInfo;

            DynamicLib vulkanLib;

            VkInstance instance = VK_NULL_HANDLE;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice vkDevice = VK_NULL_HANDLE;
            uint32_t queueFamily = 0;
            VkQueue queue = VK_NULL_HANDLE;
            VkDebugReportCallbackEXT debugReportCallback = VK_NULL_HANDLE;

            Serial nextSerial = 1;
            Serial completedSerial = 0;
            MapReadRequestTracker* mapReadRequestTracker = nullptr;
            MemoryAllocator* memoryAllocator = nullptr;
    };

    class Queue : public QueueBase {
        public:
            Queue(QueueBuilder* builder);
            ~Queue();

            // NXT API
            void Submit(uint32_t numCommands, CommandBuffer* const * commands);
    };

    class Texture : public TextureBase {
        public:
            Texture(TextureBuilder* builder);
            ~Texture();

        private:
            void TransitionUsageImpl(nxt::TextureUsageBit currentUsage, nxt::TextureUsageBit targetUsage) override;
    };

    class SwapChain : public SwapChainBase {
        public:
            SwapChain(SwapChainBuilder* builder);
            ~SwapChain();

        protected:
            TextureBase* GetNextTextureImpl(TextureBuilder* builder) override;
    };

}
}

#endif // BACKEND_VULKAN_VULKANBACKEND_H_
