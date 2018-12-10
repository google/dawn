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

#include "dawn_native/vulkan/DeviceVk.h"

#include "common/Platform.h"
#include "common/SwapChainUtils.h"
#include "dawn_native/Commands.h"
#include "dawn_native/VulkanBackend.h"
#include "dawn_native/vulkan/BindGroupLayoutVk.h"
#include "dawn_native/vulkan/BindGroupVk.h"
#include "dawn_native/vulkan/BlendStateVk.h"
#include "dawn_native/vulkan/BufferUploader.h"
#include "dawn_native/vulkan/BufferVk.h"
#include "dawn_native/vulkan/CommandBufferVk.h"
#include "dawn_native/vulkan/ComputePipelineVk.h"
#include "dawn_native/vulkan/DepthStencilStateVk.h"
#include "dawn_native/vulkan/FencedDeleter.h"
#include "dawn_native/vulkan/InputStateVk.h"
#include "dawn_native/vulkan/NativeSwapChainImplVk.h"
#include "dawn_native/vulkan/PipelineLayoutVk.h"
#include "dawn_native/vulkan/QueueVk.h"
#include "dawn_native/vulkan/RenderPassCache.h"
#include "dawn_native/vulkan/RenderPassDescriptorVk.h"
#include "dawn_native/vulkan/RenderPipelineVk.h"
#include "dawn_native/vulkan/SamplerVk.h"
#include "dawn_native/vulkan/ShaderModuleVk.h"
#include "dawn_native/vulkan/SwapChainVk.h"
#include "dawn_native/vulkan/TextureVk.h"

#include <spirv-cross/spirv_cross.hpp>

#include <iostream>

#if DAWN_PLATFORM_LINUX
const char kVulkanLibName[] = "libvulkan.so.1";
#elif DAWN_PLATFORM_WINDOWS
const char kVulkanLibName[] = "vulkan-1.dll";
#else
#    error "Unimplemented Vulkan backend platform"
#endif

namespace dawn_native { namespace vulkan {

    dawnDevice CreateDevice(const std::vector<const char*>& requiredInstanceExtensions) {
        return reinterpret_cast<dawnDevice>(new Device(requiredInstanceExtensions));
    }

    VkInstance GetInstance(dawnDevice device) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        return backendDevice->GetInstance();
    }

    DAWN_NATIVE_EXPORT dawnSwapChainImplementation
    CreateNativeSwapChainImpl(dawnDevice device, VkSurfaceKHRNative surfaceNative) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        VkSurfaceKHR surface = VkSurfaceKHR::CreateFromHandle(surfaceNative);

        dawnSwapChainImplementation impl;
        impl = CreateSwapChainImplementation(new NativeSwapChainImpl(backendDevice, surface));
        impl.textureUsage = DAWN_TEXTURE_USAGE_BIT_PRESENT;

        return impl;
    }

    dawnTextureFormat GetNativeSwapChainPreferredFormat(
        const dawnSwapChainImplementation* swapChain) {
        NativeSwapChainImpl* impl = reinterpret_cast<NativeSwapChainImpl*>(swapChain->userData);
        return static_cast<dawnTextureFormat>(impl->GetPreferredFormat());
    }

    // Device

    Device::Device(const std::vector<const char*>& requiredInstanceExtensions) {
        if (ConsumedError(Initialize(requiredInstanceExtensions))) {
            ASSERT(false);
            return;
        }
    }

    MaybeError Device::Initialize(const std::vector<const char*>& requiredInstanceExtensions) {
        if (!mVulkanLib.Open(kVulkanLibName)) {
            return DAWN_CONTEXT_LOST_ERROR(std::string("Couldn't open ") + kVulkanLibName);
        }

        VulkanFunctions* functions = GetMutableFunctions();
        DAWN_TRY(functions->LoadGlobalProcs(mVulkanLib));

        DAWN_TRY_ASSIGN(mGlobalInfo, GatherGlobalInfo(*this));

        VulkanGlobalKnobs usedGlobalKnobs = {};
        DAWN_TRY_ASSIGN(usedGlobalKnobs, CreateInstance(requiredInstanceExtensions));
        *static_cast<VulkanGlobalKnobs*>(&mGlobalInfo) = usedGlobalKnobs;

        DAWN_TRY(functions->LoadInstanceProcs(mInstance, mGlobalInfo));

        if (usedGlobalKnobs.debugReport) {
            DAWN_TRY(RegisterDebugReport());
        }

        std::vector<VkPhysicalDevice> physicalDevices;
        DAWN_TRY_ASSIGN(physicalDevices, GetPhysicalDevices(*this));
        if (physicalDevices.empty()) {
            return DAWN_CONTEXT_LOST_ERROR("No physical devices");
        }
        // TODO(cwallez@chromium.org): Choose the physical device based on ???
        mPhysicalDevice = physicalDevices[0];

        DAWN_TRY_ASSIGN(mDeviceInfo, GatherDeviceInfo(*this, mPhysicalDevice));

        VulkanDeviceKnobs usedDeviceKnobs = {};
        DAWN_TRY_ASSIGN(usedDeviceKnobs, CreateDevice());
        *static_cast<VulkanDeviceKnobs*>(&mDeviceInfo) = usedDeviceKnobs;

        DAWN_TRY(functions->LoadDeviceProcs(mVkDevice, mDeviceInfo));

        GatherQueueFromDevice();

        mBufferUploader = std::make_unique<BufferUploader>(this);
        mDeleter = std::make_unique<FencedDeleter>(this);
        mMapRequestTracker = std::make_unique<MapRequestTracker>(this);
        mMemoryAllocator = std::make_unique<MemoryAllocator>(this);
        mRenderPassCache = std::make_unique<RenderPassCache>(this);

        mPCIInfo.deviceId = mDeviceInfo.properties.deviceID;
        mPCIInfo.vendorId = mDeviceInfo.properties.vendorID;
        mPCIInfo.name = mDeviceInfo.properties.deviceName;

        return {};
    }

    Device::~Device() {
        // Immediately forget about all pending commands so we don't try to submit them in Tick
        FreeCommands(&mPendingCommands);

        if (fn.QueueWaitIdle(mQueue) != VK_SUCCESS) {
            ASSERT(false);
        }
        CheckPassedFences();

        // Make sure all fences are complete by explicitly waiting on them all
        while (!mFencesInFlight.empty()) {
            VkFence fence = mFencesInFlight.front().first;
            Serial fenceSerial = mFencesInFlight.front().second;
            ASSERT(fenceSerial > mCompletedSerial);

            VkResult result = VK_TIMEOUT;
            do {
                result = fn.WaitForFences(mVkDevice, 1, &fence, true, UINT64_MAX);
            } while (result == VK_TIMEOUT);
            fn.DestroyFence(mVkDevice, fence, nullptr);

            mFencesInFlight.pop();
            mCompletedSerial = fenceSerial;
        }

        // Some operations might have been started since the last submit and waiting
        // on a serial that doesn't have a corresponding fence enqueued. Force all
        // operations to look as if they were completed (because they were).
        mCompletedSerial = mLastSubmittedSerial + 1;
        Tick();

        ASSERT(mCommandsInFlight.Empty());
        for (auto& commands : mUnusedCommands) {
            FreeCommands(&commands);
        }
        mUnusedCommands.clear();

        ASSERT(mWaitSemaphores.empty());

        for (VkFence fence : mUnusedFences) {
            fn.DestroyFence(mVkDevice, fence, nullptr);
        }
        mUnusedFences.clear();

        // Free services explicitly so that they can free Vulkan objects before vkDestroyDevice
        mBufferUploader = nullptr;
        mDeleter = nullptr;
        mMapRequestTracker = nullptr;
        mMemoryAllocator = nullptr;

        // The VkRenderPasses in the cache can be destroyed immediately since all commands referring
        // to them are guaranteed to be finished executing.
        mRenderPassCache = nullptr;

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

    ResultOrError<BindGroupBase*> Device::CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) {
        return new BindGroup(this, descriptor);
    }
    ResultOrError<BindGroupLayoutBase*> Device::CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) {
        return new BindGroupLayout(this, descriptor);
    }
    BlendStateBase* Device::CreateBlendState(BlendStateBuilder* builder) {
        return new BlendState(builder);
    }
    ResultOrError<BufferBase*> Device::CreateBufferImpl(const BufferDescriptor* descriptor) {
        return new Buffer(this, descriptor);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(builder);
    }
    ResultOrError<ComputePipelineBase*> Device::CreateComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) {
        return new ComputePipeline(this, descriptor);
    }
    DepthStencilStateBase* Device::CreateDepthStencilState(DepthStencilStateBuilder* builder) {
        return new DepthStencilState(builder);
    }
    InputStateBase* Device::CreateInputState(InputStateBuilder* builder) {
        return new InputState(builder);
    }
    ResultOrError<PipelineLayoutBase*> Device::CreatePipelineLayoutImpl(
        const PipelineLayoutDescriptor* descriptor) {
        return new PipelineLayout(this, descriptor);
    }
    ResultOrError<QueueBase*> Device::CreateQueueImpl() {
        return new Queue(this);
    }
    RenderPassDescriptorBase* Device::CreateRenderPassDescriptor(
        RenderPassDescriptorBuilder* builder) {
        return new RenderPassDescriptor(builder);
    }
    ResultOrError<RenderPipelineBase*> Device::CreateRenderPipelineImpl(
        const RenderPipelineDescriptor* descriptor) {
        return new RenderPipeline(this, descriptor);
    }
    ResultOrError<SamplerBase*> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
        return new Sampler(this, descriptor);
    }
    ResultOrError<ShaderModuleBase*> Device::CreateShaderModuleImpl(
        const ShaderModuleDescriptor* descriptor) {
        return new ShaderModule(this, descriptor);
    }
    SwapChainBase* Device::CreateSwapChain(SwapChainBuilder* builder) {
        return new SwapChain(builder);
    }
    ResultOrError<TextureBase*> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
        return new Texture(this, descriptor);
    }

    ResultOrError<TextureViewBase*> Device::CreateTextureViewImpl(
        TextureBase* texture,
        const TextureViewDescriptor* descriptor) {
        return new TextureView(texture, descriptor);
    }

    Serial Device::GetCompletedCommandSerial() const {
        return mCompletedSerial;
    }

    Serial Device::GetLastSubmittedCommandSerial() const {
        return mLastSubmittedSerial;
    }

    Serial Device::GetPendingCommandSerial() const {
        return mLastSubmittedSerial + 1;
    }

    void Device::TickImpl() {
        CheckPassedFences();
        RecycleCompletedCommands();

        mMapRequestTracker->Tick(mCompletedSerial);
        mBufferUploader->Tick(mCompletedSerial);
        mMemoryAllocator->Tick(mCompletedSerial);

        mDeleter->Tick(mCompletedSerial);

        if (mPendingCommands.pool != VK_NULL_HANDLE) {
            SubmitPendingCommands();
        } else if (mCompletedSerial == mLastSubmittedSerial) {
            // If there's no GPU work in flight we still need to artificially increment the serial
            // so that CPU operations waiting on GPU completion can know they don't have to wait.
            mCompletedSerial++;
            mLastSubmittedSerial++;
        }
    }

    const dawn_native::PCIInfo& Device::GetPCIInfo() const {
        return mPCIInfo;
    }

    const VulkanDeviceInfo& Device::GetDeviceInfo() const {
        return mDeviceInfo;
    }

    VkInstance Device::GetInstance() const {
        return mInstance;
    }

    VkPhysicalDevice Device::GetPhysicalDevice() const {
        return mPhysicalDevice;
    }

    VkDevice Device::GetVkDevice() const {
        return mVkDevice;
    }

    uint32_t Device::GetGraphicsQueueFamily() const {
        return mQueueFamily;
    }

    VkQueue Device::GetQueue() const {
        return mQueue;
    }

    MapRequestTracker* Device::GetMapRequestTracker() const {
        return mMapRequestTracker.get();
    }

    MemoryAllocator* Device::GetMemoryAllocator() const {
        return mMemoryAllocator.get();
    }

    BufferUploader* Device::GetBufferUploader() const {
        return mBufferUploader.get();
    }

    FencedDeleter* Device::GetFencedDeleter() const {
        return mDeleter.get();
    }

    RenderPassCache* Device::GetRenderPassCache() const {
        return mRenderPassCache.get();
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

        std::vector<VkPipelineStageFlags> dstStageMasks(mWaitSemaphores.size(),
                                                        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        VkSubmitInfo submitInfo;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = static_cast<uint32_t>(mWaitSemaphores.size());
        submitInfo.pWaitSemaphores = mWaitSemaphores.data();
        submitInfo.pWaitDstStageMask = dstStageMasks.data();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &mPendingCommands.commandBuffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = 0;

        VkFence fence = GetUnusedFence();
        if (fn.QueueSubmit(mQueue, 1, &submitInfo, fence) != VK_SUCCESS) {
            ASSERT(false);
        }

        mLastSubmittedSerial++;
        mCommandsInFlight.Enqueue(mPendingCommands, mLastSubmittedSerial);
        mPendingCommands = CommandPoolAndBuffer();
        mFencesInFlight.emplace(fence, mLastSubmittedSerial);

        for (VkSemaphore semaphore : mWaitSemaphores) {
            mDeleter->DeleteWhenUnused(semaphore);
        }
        mWaitSemaphores.clear();
    }

    void Device::AddWaitSemaphore(VkSemaphore semaphore) {
        mWaitSemaphores.push_back(semaphore);
    }

    ResultOrError<VulkanGlobalKnobs> Device::CreateInstance(
        const std::vector<const char*>& requiredExtensions) {
        VulkanGlobalKnobs usedKnobs = {};

        std::vector<const char*> layersToRequest;
        std::vector<const char*> extensionsToRequest = requiredExtensions;

        auto AddExtensionIfNotPresent = [](std::vector<const char*>* extensions,
                                           const char* extension) {
            for (const char* present : *extensions) {
                if (strcmp(present, extension) == 0) {
                    return;
                }
            }
            extensions->push_back(extension);
        };

        // vktrace works by instering a layer, but we hide it behind a macro due to the vktrace
        // layer crashes when used without vktrace server started, see this vktrace issue:
        // https://github.com/LunarG/VulkanTools/issues/254
        // Also it is good to put it in first position so that it doesn't see Vulkan calls inserted
        // by other layers.
#if defined(DAWN_USE_VKTRACE)
        if (mGlobalInfo.vktrace) {
            layersToRequest.push_back(kLayerNameLunargVKTrace);
            usedKnobs.vktrace = true;
        }
#endif
        // RenderDoc installs a layer at the system level for its capture but we don't want to use
        // it unless we are debugging in RenderDoc so we hide it behind a macro.
#if defined(DAWN_USE_RENDERDOC)
        if (mGlobalInfo.renderDocCapture) {
            layersToRequest.push_back(kLayerNameRenderDocCapture);
            usedKnobs.renderDocCapture = true;
        }
#endif
#if defined(DAWN_ENABLE_ASSERTS)
        if (mGlobalInfo.standardValidation) {
            layersToRequest.push_back(kLayerNameLunargStandardValidation);
            usedKnobs.standardValidation = true;
        }
        if (mGlobalInfo.debugReport) {
            AddExtensionIfNotPresent(&extensionsToRequest, kExtensionNameExtDebugReport);
            usedKnobs.debugReport = true;
        }
#endif
        if (mGlobalInfo.surface) {
            AddExtensionIfNotPresent(&extensionsToRequest, kExtensionNameKhrSurface);
            usedKnobs.surface = true;
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
            return DAWN_CONTEXT_LOST_ERROR("vkCreateInstance failed");
        }

        return usedKnobs;
    }

    ResultOrError<VulkanDeviceKnobs> Device::CreateDevice() {
        VulkanDeviceKnobs usedKnobs = {};

        float zero = 0.0f;
        std::vector<const char*> layersToRequest;
        std::vector<const char*> extensionsToRequest;
        std::vector<VkDeviceQueueCreateInfo> queuesToRequest;

        if (mDeviceInfo.swapchain) {
            extensionsToRequest.push_back(kExtensionNameKhrSwapchain);
            usedKnobs.swapchain = true;
        }

        // Always require independentBlend because it is a core Dawn feature
        usedKnobs.features.independentBlend = VK_TRUE;
        // Always require imageCubeArray because it is a core Dawn feature
        usedKnobs.features.imageCubeArray = VK_TRUE;

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
                return DAWN_CONTEXT_LOST_ERROR("No universal queue family");
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
        createInfo.pEnabledFeatures = &usedKnobs.features;

        if (fn.CreateDevice(mPhysicalDevice, &createInfo, nullptr, &mVkDevice) != VK_SUCCESS) {
            return DAWN_CONTEXT_LOST_ERROR("vkCreateDevice failed");
        }

        return usedKnobs;
    }

    void Device::GatherQueueFromDevice() {
        fn.GetDeviceQueue(mVkDevice, mQueueFamily, 0, &mQueue);
    }

    MaybeError Device::RegisterDebugReport() {
        VkDebugReportCallbackCreateInfoEXT createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = Device::OnDebugReportCallback;
        createInfo.pUserData = this;

        if (fn.CreateDebugReportCallbackEXT(mInstance, &createInfo, nullptr,
                                            &mDebugReportCallback) != VK_SUCCESS) {
            return DAWN_CONTEXT_LOST_ERROR("vkCreateDebugReportCallbackEXT failed");
        }

        return {};
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

}}  // namespace dawn_native::vulkan
