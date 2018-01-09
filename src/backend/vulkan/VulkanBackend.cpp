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
#include "backend/vulkan/BufferUploader.h"
#include "backend/vulkan/BufferVk.h"
#include "backend/vulkan/CommandBufferVk.h"
#include "backend/vulkan/FencedDeleter.h"
#include "backend/vulkan/FramebufferVk.h"
#include "backend/vulkan/InputStateVk.h"
#include "backend/vulkan/PipelineLayoutVk.h"
#include "backend/vulkan/RenderPassVk.h"
#include "backend/vulkan/TextureVk.h"
#include "common/Platform.h"

#include <spirv-cross/spirv_cross.hpp>

#include <iostream>

#if NXT_PLATFORM_LINUX
const char kVulkanLibName[] = "libvulkan.so.1";
#elif NXT_PLATFORM_WINDOWS
const char kVulkanLibName[] = "vulkan-1.dll";
#else
#    error "Unimplemented Vulkan backend platform"
#endif

namespace backend { namespace vulkan {

    nxtProcTable GetNonValidatingProcs();
    nxtProcTable GetValidatingProcs();

    void Init(nxtProcTable* procs, nxtDevice* device) {
        *procs = GetValidatingProcs();
        *device = reinterpret_cast<nxtDevice>(new Device);
    }

    // Device

    Device::Device() {
        if (!mVulkanLib.Open(kVulkanLibName)) {
            ASSERT(false);
            return;
        }

        VulkanFunctions* functions = GetMutableFunctions();

        if (!functions->LoadGlobalProcs(mVulkanLib)) {
            ASSERT(false);
            return;
        }

        if (!GatherGlobalInfo(*this, &mGlobalInfo)) {
            ASSERT(false);
            return;
        }

        VulkanGlobalKnobs usedGlobalKnobs = {};
        if (!CreateInstance(&usedGlobalKnobs)) {
            ASSERT(false);
            return;
        }
        *static_cast<VulkanGlobalKnobs*>(&mGlobalInfo) = usedGlobalKnobs;

        if (!functions->LoadInstanceProcs(mInstance, usedGlobalKnobs)) {
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
        mPhysicalDevice = physicalDevices[0];

        if (!GatherDeviceInfo(*this, mPhysicalDevice, &mDeviceInfo)) {
            ASSERT(false);
            return;
        }

        VulkanDeviceKnobs usedDeviceKnobs = {};
        if (!CreateDevice(&usedDeviceKnobs)) {
            ASSERT(false);
            return;
        }
        *static_cast<VulkanDeviceKnobs*>(&mDeviceInfo) = usedDeviceKnobs;

        if (!functions->LoadDeviceProcs(mVkDevice, usedDeviceKnobs)) {
            ASSERT(false);
            return;
        }

        GatherQueueFromDevice();

        mBufferUploader = new BufferUploader(this);
        mDeleter = new FencedDeleter(this);
        mMapReadRequestTracker = new MapReadRequestTracker(this);
        mMemoryAllocator = new MemoryAllocator(this);
    }

    Device::~Device() {
        // Immediately forget about all pending commands so we don't try to submit them in Tick
        FreeCommands(&mPendingCommands);

        if (fn.QueueWaitIdle(mQueue) != VK_SUCCESS) {
            ASSERT(false);
        }
        CheckPassedFences();
        ASSERT(mFencesInFlight.empty());

        // Some operations might have been started since the last submit and waiting
        // on a serial that doesn't have a corresponding fence enqueued. Force all
        // operations to look as if they were completed (because they were).
        mCompletedSerial = mNextSerial;
        Tick();

        ASSERT(mCommandsInFlight.Empty());
        for (auto& commands : mUnusedCommands) {
            FreeCommands(&commands);
        }
        mUnusedCommands.clear();

        for (VkFence fence : mUnusedFences) {
            fn.DestroyFence(mVkDevice, fence, nullptr);
        }
        mUnusedFences.clear();

        delete mBufferUploader;
        mBufferUploader = nullptr;

        delete mDeleter;
        mDeleter = nullptr;

        delete mMapReadRequestTracker;
        mMapReadRequestTracker = nullptr;

        delete mMemoryAllocator;
        mMemoryAllocator = nullptr;

        // VkQueues are destroyed when the VkDevice is destroyed
        if (mVkDevice != VK_NULL_HANDLE) {
            fn.DestroyDevice(mVkDevice, nullptr);
            mVkDevice = VK_NULL_HANDLE;
        }

        if (mDebugReportCallback != VK_NULL_HANDLE) {
            fn.DestroyDebugReportCallbackEXT(mInstance, mDebugReportCallback, nullptr);
            mDebugReportCallback = VK_NULL_HANDLE;
        }

        // VkPhysicalDevices are destroyed when the VkInstance is destroyed
        if (mInstance != VK_NULL_HANDLE) {
            fn.DestroyInstance(mInstance, nullptr);
            mInstance = VK_NULL_HANDLE;
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
        CheckPassedFences();
        RecycleCompletedCommands();

        mMapReadRequestTracker->Tick(mCompletedSerial);
        mBufferUploader->Tick(mCompletedSerial);
        mMemoryAllocator->Tick(mCompletedSerial);

        mDeleter->Tick(mCompletedSerial);

        if (mPendingCommands.pool != VK_NULL_HANDLE) {
            SubmitPendingCommands();
        } else if (mCompletedSerial == mNextSerial - 1) {
            // If there's no GPU work in flight we still need to artificially increment the serial
            // so that CPU operations waiting on GPU completion can know they don't have to wait.
            mCompletedSerial++;
            mNextSerial++;
        }
    }

    const VulkanDeviceInfo& Device::GetDeviceInfo() const {
        return mDeviceInfo;
    }

    MapReadRequestTracker* Device::GetMapReadRequestTracker() const {
        return mMapReadRequestTracker;
    }

    MemoryAllocator* Device::GetMemoryAllocator() const {
        return mMemoryAllocator;
    }

    BufferUploader* Device::GetBufferUploader() const {
        return mBufferUploader;
    }

    FencedDeleter* Device::GetFencedDeleter() const {
        return mDeleter;
    }

    Serial Device::GetSerial() const {
        return mNextSerial;
    }

    VkCommandBuffer Device::GetPendingCommandBuffer() {
        if (mPendingCommands.pool == VK_NULL_HANDLE) {
            mPendingCommands = GetUnusedCommands();

            VkCommandBufferBeginInfo beginInfo;
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext = nullptr;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            beginInfo.pInheritanceInfo = nullptr;

            if (fn.BeginCommandBuffer(mPendingCommands.commandBuffer, &beginInfo) != VK_SUCCESS) {
                ASSERT(false);
            }
        }

        return mPendingCommands.commandBuffer;
    }

    void Device::SubmitPendingCommands() {
        if (mPendingCommands.pool == VK_NULL_HANDLE) {
            return;
        }

        if (fn.EndCommandBuffer(mPendingCommands.commandBuffer) != VK_SUCCESS) {
            ASSERT(false);
        }

        VkSubmitInfo submitInfo;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = 0;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &mPendingCommands.commandBuffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = 0;

        VkFence fence = GetUnusedFence();
        if (fn.QueueSubmit(mQueue, 1, &submitInfo, fence) != VK_SUCCESS) {
            ASSERT(false);
        }

        mCommandsInFlight.Enqueue(mPendingCommands, mNextSerial);
        mPendingCommands = CommandPoolAndBuffer();
        mFencesInFlight.emplace(fence, mNextSerial);
        mNextSerial++;
    }

    VkInstance Device::GetInstance() const {
        return mInstance;
    }

    VkDevice Device::GetVkDevice() const {
        return mVkDevice;
    }

    bool Device::CreateInstance(VulkanGlobalKnobs* usedKnobs) {
        std::vector<const char*> layersToRequest;
        std::vector<const char*> extensionsToRequest;

#if defined(NXT_ENABLE_ASSERTS)
        if (mGlobalInfo.standardValidation) {
            layersToRequest.push_back(kLayerNameLunargStandardValidation);
            usedKnobs->standardValidation = true;
        }
        if (mGlobalInfo.debugReport) {
            extensionsToRequest.push_back(kExtensionNameExtDebugReport);
            usedKnobs->debugReport = true;
        }
#endif
        if (mGlobalInfo.surface) {
            extensionsToRequest.push_back(kExtensionNameKhrSurface);
            usedKnobs->surface = true;
        }

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

        if (fn.CreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
            return false;
        }

        return true;
    }

    bool Device::CreateDevice(VulkanDeviceKnobs* usedKnobs) {
        float zero = 0.0f;
        std::vector<const char*> layersToRequest;
        std::vector<const char*> extensionsToRequest;
        std::vector<VkDeviceQueueCreateInfo> queuesToRequest;

        if (mDeviceInfo.swapchain) {
            extensionsToRequest.push_back(kExtensionNameKhrSwapchain);
            usedKnobs->swapchain = true;
        }

        // Find a universal queue family
        {
            constexpr uint32_t kUniversalFlags =
                VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            int universalQueueFamily = -1;
            for (unsigned int i = 0; i < mDeviceInfo.queueFamilies.size(); ++i) {
                if ((mDeviceInfo.queueFamilies[i].queueFlags & kUniversalFlags) ==
                    kUniversalFlags) {
                    universalQueueFamily = i;
                    break;
                }
            }

            if (universalQueueFamily == -1) {
                return false;
            }
            mQueueFamily = static_cast<uint32_t>(universalQueueFamily);
        }

        // Choose to create a single universal queue
        {
            VkDeviceQueueCreateInfo queueCreateInfo;
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.pNext = nullptr;
            queueCreateInfo.flags = 0;
            queueCreateInfo.queueFamilyIndex = static_cast<uint32_t>(mQueueFamily);
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

        if (fn.CreateDevice(mPhysicalDevice, &createInfo, nullptr, &mVkDevice) != VK_SUCCESS) {
            return false;
        }

        return true;
    }

    void Device::GatherQueueFromDevice() {
        fn.GetDeviceQueue(mVkDevice, mQueueFamily, 0, &mQueue);
    }

    bool Device::RegisterDebugReport() {
        VkDebugReportCallbackCreateInfoEXT createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = Device::OnDebugReportCallback;
        createInfo.pUserData = this;

        if (fn.CreateDebugReportCallbackEXT(mInstance, &createInfo, nullptr,
                                            &mDebugReportCallback) != VK_SUCCESS) {
            return false;
        }

        return true;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL
    Device::OnDebugReportCallback(VkDebugReportFlagsEXT flags,
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

    VkFence Device::GetUnusedFence() {
        if (!mUnusedFences.empty()) {
            VkFence fence = mUnusedFences.back();
            mUnusedFences.pop_back();
            return fence;
        }

        VkFenceCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;

        VkFence fence = VK_NULL_HANDLE;
        if (fn.CreateFence(mVkDevice, &createInfo, nullptr, &fence) != VK_SUCCESS) {
            ASSERT(false);
        }

        return fence;
    }

    void Device::CheckPassedFences() {
        while (!mFencesInFlight.empty()) {
            VkFence fence = mFencesInFlight.front().first;
            Serial fenceSerial = mFencesInFlight.front().second;

            VkResult result = fn.GetFenceStatus(mVkDevice, fence);
            ASSERT(result == VK_SUCCESS || result == VK_NOT_READY);

            // Fence are added in order, so we can stop searching as soon
            // as we see one that's not ready.
            if (result == VK_NOT_READY) {
                return;
            }

            if (fn.ResetFences(mVkDevice, 1, &fence) != VK_SUCCESS) {
                ASSERT(false);
            }
            mUnusedFences.push_back(fence);

            mFencesInFlight.pop();

            ASSERT(fenceSerial > mCompletedSerial);
            mCompletedSerial = fenceSerial;
        }
    }

    Device::CommandPoolAndBuffer Device::GetUnusedCommands() {
        if (!mUnusedCommands.empty()) {
            CommandPoolAndBuffer commands = mUnusedCommands.back();
            mUnusedCommands.pop_back();
            return commands;
        }

        CommandPoolAndBuffer commands;

        VkCommandPoolCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        createInfo.queueFamilyIndex = mQueueFamily;

        if (fn.CreateCommandPool(mVkDevice, &createInfo, nullptr, &commands.pool) != VK_SUCCESS) {
            ASSERT(false);
        }

        VkCommandBufferAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.commandPool = commands.pool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;

        if (fn.AllocateCommandBuffers(mVkDevice, &allocateInfo, &commands.commandBuffer) !=
            VK_SUCCESS) {
            ASSERT(false);
        }

        return commands;
    }

    void Device::RecycleCompletedCommands() {
        for (auto& commands : mCommandsInFlight.IterateUpTo(mCompletedSerial)) {
            if (fn.ResetCommandPool(mVkDevice, commands.pool, 0) != VK_SUCCESS) {
                ASSERT(false);
            }
            mUnusedCommands.push_back(commands);
        }
        mCommandsInFlight.ClearUpTo(mCompletedSerial);
    }

    void Device::FreeCommands(CommandPoolAndBuffer* commands) {
        if (commands->pool != VK_NULL_HANDLE) {
            fn.DestroyCommandPool(mVkDevice, commands->pool, nullptr);
            commands->pool = VK_NULL_HANDLE;
        }

        // Command buffers are implicitly destroyed when the command pool is.
        commands->commandBuffer = VK_NULL_HANDLE;
    }

    // Queue

    Queue::Queue(QueueBuilder* builder) : QueueBase(builder) {
    }

    Queue::~Queue() {
    }

    void Queue::Submit(uint32_t numCommands, CommandBuffer* const* commands) {
        Device* device = ToBackend(GetDevice());

        VkCommandBuffer commandBuffer = device->GetPendingCommandBuffer();
        for (uint32_t i = 0; i < numCommands; ++i) {
            commands[i]->RecordCommands(commandBuffer);
        }

        device->SubmitPendingCommands();
    }

    // SwapChain

    SwapChain::SwapChain(SwapChainBuilder* builder) : SwapChainBase(builder) {
        const auto& im = GetImplementation();
        im.Init(im.userData, nullptr);
    }

    SwapChain::~SwapChain() {
    }

    TextureBase* SwapChain::GetNextTextureImpl(TextureBuilder* builder) {
        return GetDevice()->CreateTexture(builder);
    }
}}  // namespace backend::vulkan
