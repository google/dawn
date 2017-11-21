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

#include "backend/vulkan/VulkanBackend.h"

#include "backend/Commands.h"
#include "common/Platform.h"

#include <spirv-cross/spirv_cross.hpp>

#include <iostream>

#if NXT_PLATFORM_LINUX
    const char kVulkanLibName[] = "libvulkan.so.1";
#elif NXT_PLATFORM_WINDOWS
    const char kVulkanLibName[] = "vulkan-1.dll";
#else
    #error "Unimplemented Vulkan backend platform"
#endif

namespace backend {
namespace vulkan {

    nxtProcTable GetNonValidatingProcs();
    nxtProcTable GetValidatingProcs();

    void Init(nxtProcTable* procs, nxtDevice* device) {
        *procs = GetValidatingProcs();
        *device = reinterpret_cast<nxtDevice>(new Device);
    }

    // Device

    Device::Device() {
        if (!vulkanLib.Open(kVulkanLibName)) {
            ASSERT(false);
            return;
        }

        VulkanFunctions* functions = GetMutableFunctions();

        if (!functions->LoadGlobalProcs(vulkanLib)) {
            ASSERT(false);
            return;
        }

        if (!GatherGlobalInfo(*this, &globalInfo)) {
            ASSERT(false);
            return;
        }

        VulkanGlobalKnobs usedGlobalKnobs = {};
        if (!CreateInstance(&usedGlobalKnobs)) {
            ASSERT(false);
            return;
        }
        *static_cast<VulkanGlobalKnobs*>(&globalInfo) = usedGlobalKnobs;

        if (!functions->LoadInstanceProcs(instance, usedGlobalKnobs)) {
            ASSERT(false);
            return;
        }

        if (usedGlobalKnobs.debugReport) {
            if (!RegisterDebugReport()) {
                ASSERT(false);
                return;
            }
        }

        std::vector<VkPhysicalDevice> physicalDevices;
        if (!GetPhysicalDevices(*this, &physicalDevices) || physicalDevices.empty()) {
            ASSERT(false);
            return;
        }
        // TODO(cwallez@chromium.org): Choose the physical device based on ???
        physicalDevice = physicalDevices[0];

        if (!GatherDeviceInfo(*this, physicalDevice, &deviceInfo)) {
            ASSERT(false);
            return;
        }

        VulkanDeviceKnobs usedDeviceKnobs = {};
        if (!CreateDevice(&usedDeviceKnobs)) {
            ASSERT(false);
            return;
        }
        *static_cast<VulkanDeviceKnobs*>(&deviceInfo) = usedDeviceKnobs;

        if (!functions->LoadDeviceProcs(vkDevice, usedDeviceKnobs)) {
            ASSERT(false);
            return;
        }
    }

    Device::~Device() {
        if (vkDevice != VK_NULL_HANDLE) {
            fn.DestroyDevice(vkDevice, nullptr);
            vkDevice = VK_NULL_HANDLE;
        }

        if (debugReportCallback != VK_NULL_HANDLE) {
            fn.DestroyDebugReportCallbackEXT(instance, debugReportCallback, nullptr);
            debugReportCallback = VK_NULL_HANDLE;
        }

        if (instance != VK_NULL_HANDLE) {
            fn.DestroyInstance(instance, nullptr);
            instance = VK_NULL_HANDLE;
        }
    }

    BindGroupBase* Device::CreateBindGroup(BindGroupBuilder* builder) {
        return new BindGroup(builder);
    }
    BindGroupLayoutBase* Device::CreateBindGroupLayout(BindGroupLayoutBuilder* builder) {
        return new BindGroupLayout(builder);
    }
    BlendStateBase* Device::CreateBlendState(BlendStateBuilder* builder) {
        return new BlendState(builder);
    }
    BufferBase* Device::CreateBuffer(BufferBuilder* builder) {
        return new Buffer(builder);
    }
    BufferViewBase* Device::CreateBufferView(BufferViewBuilder* builder) {
        return new BufferView(builder);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(builder);
    }
    ComputePipelineBase* Device::CreateComputePipeline(ComputePipelineBuilder* builder) {
        return new ComputePipeline(builder);
    }
    DepthStencilStateBase* Device::CreateDepthStencilState(DepthStencilStateBuilder* builder) {
        return new DepthStencilState(builder);
    }
    FramebufferBase* Device::CreateFramebuffer(FramebufferBuilder* builder) {
        return new Framebuffer(builder);
    }
    InputStateBase* Device::CreateInputState(InputStateBuilder* builder) {
        return new InputState(builder);
    }
    PipelineLayoutBase* Device::CreatePipelineLayout(PipelineLayoutBuilder* builder) {
        return new PipelineLayout(builder);
    }
    QueueBase* Device::CreateQueue(QueueBuilder* builder) {
        return new Queue(builder);
    }
    RenderPassBase* Device::CreateRenderPass(RenderPassBuilder* builder) {
        return new RenderPass(builder);
    }
    RenderPipelineBase* Device::CreateRenderPipeline(RenderPipelineBuilder* builder) {
        return new RenderPipeline(builder);
    }
    SamplerBase* Device::CreateSampler(SamplerBuilder* builder) {
        return new Sampler(builder);
    }
    ShaderModuleBase* Device::CreateShaderModule(ShaderModuleBuilder* builder) {
        auto module = new ShaderModule(builder);

        spirv_cross::Compiler compiler(builder->AcquireSpirv());
        module->ExtractSpirvInfo(compiler);

        return module;
    }
    SwapChainBase* Device::CreateSwapChain(SwapChainBuilder* builder) {
        return new SwapChain(builder);
    }
    TextureBase* Device::CreateTexture(TextureBuilder* builder) {
        return new Texture(builder);
    }
    TextureViewBase* Device::CreateTextureView(TextureViewBuilder* builder) {
        return new TextureView(builder);
    }

    void Device::TickImpl() {
    }

    VkInstance Device::GetInstance() const {
        return instance;
    }

    bool Device::CreateInstance(VulkanGlobalKnobs* usedKnobs) {
        std::vector<const char*> layersToRequest;
        std::vector<const char*> extensionsToRequest;

        #if defined(NXT_ENABLE_ASSERTS)
            if (globalInfo.standardValidation) {
                layersToRequest.push_back(kLayerNameLunargStandardValidation);
                usedKnobs->standardValidation = true;
            }
            if (globalInfo.debugReport) {
                extensionsToRequest.push_back(kExtensionNameExtDebugReport);
                usedKnobs->debugReport = true;
            }
        #endif

        VkApplicationInfo appInfo;
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = nullptr;
        appInfo.applicationVersion = 0;
        appInfo.pEngineName = nullptr;
        appInfo.engineVersion = 0;
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(layersToRequest.size());
        createInfo.ppEnabledLayerNames = layersToRequest.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionsToRequest.size());
        createInfo.ppEnabledExtensionNames = extensionsToRequest.data();

        if (fn.CreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            return false;
        }

        return true;
    }

    bool Device::CreateDevice(VulkanDeviceKnobs* usedKnobs) {
        float zero = 0.0f;
        std::vector<const char*> layersToRequest;
        std::vector<const char*> extensionsToRequest;
        std::vector<VkDeviceQueueCreateInfo> queuesToRequest;

        if (deviceInfo.swapchain) {
            extensionsToRequest.push_back(kExtensionNameKhrSwapchain);
            usedKnobs->swapchain = true;
        }

        // Find a universal queue family
        {
            constexpr uint32_t kUniversalFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            ssize_t universalQueueFamily = -1;
            for (size_t i = 0; i < deviceInfo.queueFamilies.size(); ++i) {
                if ((deviceInfo.queueFamilies[i].queueFlags & kUniversalFlags) == kUniversalFlags) {
                    universalQueueFamily = i;
                    break;
                }
            }

            if (universalQueueFamily == -1) {
                return false;
            }
            queueFamily = static_cast<uint32_t>(universalQueueFamily);
        }

        // Choose to create a single universal queue
        {
            VkDeviceQueueCreateInfo queueCreateInfo;
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.pNext = nullptr;
            queueCreateInfo.flags = 0;
            queueCreateInfo.queueFamilyIndex = static_cast<uint32_t>(queueFamily);
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &zero;

            queuesToRequest.push_back(queueCreateInfo);
        }

        VkDeviceCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queuesToRequest.size());
        createInfo.pQueueCreateInfos = queuesToRequest.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(layersToRequest.size());
        createInfo.ppEnabledLayerNames = layersToRequest.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionsToRequest.size());
        createInfo.ppEnabledExtensionNames = extensionsToRequest.data();
        createInfo.pEnabledFeatures = &usedKnobs->features;

        if (fn.CreateDevice(physicalDevice, &createInfo, nullptr, &vkDevice) != VK_SUCCESS) {
            return false;
        }

        return true;
    }

    bool Device::RegisterDebugReport() {
        VkDebugReportCallbackCreateInfoEXT createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = Device::OnDebugReportCallback;
        createInfo.pUserData = this;

        if (fn.CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &debugReportCallback) != VK_SUCCESS) {
            return false;
        }

        return true;
    }

    VkBool32 Device::OnDebugReportCallback(VkDebugReportFlagsEXT flags,
                                           VkDebugReportObjectTypeEXT /*objectType*/,
                                           uint64_t /*object*/,
                                           size_t /*location*/,
                                           int32_t /*messageCode*/,
                                           const char* /*pLayerPrefix*/,
                                           const char* pMessage,
                                           void* /*pUserdata*/) {
        std::cout << pMessage << std::endl;
        ASSERT((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) == 0);

        return VK_FALSE;
    }

    VulkanFunctions* Device::GetMutableFunctions() {
        return const_cast<VulkanFunctions*>(&fn);
    }

    // Buffer

    Buffer::Buffer(BufferBuilder* builder)
        : BufferBase(builder) {
    }

    Buffer::~Buffer() {
    }

    void Buffer::SetSubDataImpl(uint32_t, uint32_t, const uint32_t*) {
    }

    void Buffer::MapReadAsyncImpl(uint32_t, uint32_t, uint32_t) {
    }

    void Buffer::UnmapImpl() {
    }

    void Buffer::TransitionUsageImpl(nxt::BufferUsageBit, nxt::BufferUsageBit) {
    }

    // Queue

    Queue::Queue(QueueBuilder* builder)
        : QueueBase(builder) {
    }

    Queue::~Queue() {
    }

    void Queue::Submit(uint32_t, CommandBuffer* const*) {
    }

    // Texture

    Texture::Texture(TextureBuilder* builder)
        : TextureBase(builder) {
    }

    Texture::~Texture() {
    }

    void Texture::TransitionUsageImpl(nxt::TextureUsageBit, nxt::TextureUsageBit) {
    }

    // SwapChain

    SwapChain::SwapChain(SwapChainBuilder* builder)
        : SwapChainBase(builder) {
        const auto& im = GetImplementation();
        im.Init(im.userData, nullptr);
    }

    SwapChain::~SwapChain() {
    }

    TextureBase* SwapChain::GetNextTextureImpl(TextureBuilder* builder) {
        return GetDevice()->CreateTexture(builder);
    }
}
}
