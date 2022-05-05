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

#include "dawn/native/vulkan/DeviceVk.h"

#include "dawn/common/Log.h"
#include "dawn/common/Platform.h"
#include "dawn/native/BackendConnection.h"
#include "dawn/native/ChainUtils_autogen.h"
#include "dawn/native/Error.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/Instance.h"
#include "dawn/native/VulkanBackend.h"
#include "dawn/native/vulkan/AdapterVk.h"
#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/BindGroupLayoutVk.h"
#include "dawn/native/vulkan/BindGroupVk.h"
#include "dawn/native/vulkan/BufferVk.h"
#include "dawn/native/vulkan/CommandBufferVk.h"
#include "dawn/native/vulkan/ComputePipelineVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PipelineCacheVk.h"
#include "dawn/native/vulkan/PipelineLayoutVk.h"
#include "dawn/native/vulkan/QuerySetVk.h"
#include "dawn/native/vulkan/QueueVk.h"
#include "dawn/native/vulkan/RenderPassCache.h"
#include "dawn/native/vulkan/RenderPipelineVk.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/native/vulkan/SamplerVk.h"
#include "dawn/native/vulkan/ShaderModuleVk.h"
#include "dawn/native/vulkan/StagingBufferVk.h"
#include "dawn/native/vulkan/SwapChainVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

// static
ResultOrError<Ref<Device>> Device::Create(Adapter* adapter, const DeviceDescriptor* descriptor) {
    Ref<Device> device = AcquireRef(new Device(adapter, descriptor));
    DAWN_TRY(device->Initialize(descriptor));
    return device;
}

Device::Device(Adapter* adapter, const DeviceDescriptor* descriptor)
    : DeviceBase(adapter, descriptor), mDebugPrefix(GetNextDeviceDebugPrefix()) {
    InitTogglesFromDriver();
}

MaybeError Device::Initialize(const DeviceDescriptor* descriptor) {
    // Copy the adapter's device info to the device so that we can change the "knobs"
    mDeviceInfo = ToBackend(GetAdapter())->GetDeviceInfo();

    // Initialize the "instance" procs of our local function table.
    VulkanFunctions* functions = GetMutableFunctions();
    *functions = ToBackend(GetAdapter())->GetVulkanInstance()->GetFunctions();

    // Two things are crucial if device initialization fails: the function pointers to destroy
    // objects, and the fence deleter that calls these functions. Do not do anything before
    // these two are set up, so that a failed initialization doesn't cause a crash in
    // DestroyImpl()
    {
        VkPhysicalDevice physicalDevice = ToBackend(GetAdapter())->GetPhysicalDevice();

        VulkanDeviceKnobs usedDeviceKnobs = {};
        DAWN_TRY_ASSIGN(usedDeviceKnobs, CreateDevice(physicalDevice));
        *static_cast<VulkanDeviceKnobs*>(&mDeviceInfo) = usedDeviceKnobs;

        DAWN_TRY(functions->LoadDeviceProcs(mVkDevice, mDeviceInfo));

        // The queue can be loaded before the fenced deleter because their lifetime is tied to
        // the device.
        GatherQueueFromDevice();

        mDeleter = std::make_unique<FencedDeleter>(this);
    }

    mRenderPassCache = std::make_unique<RenderPassCache>(this);
    mResourceMemoryAllocator = std::make_unique<ResourceMemoryAllocator>(this);

    mExternalMemoryService = std::make_unique<external_memory::Service>(this);
    mExternalSemaphoreService = std::make_unique<external_semaphore::Service>(this);

    DAWN_TRY(PrepareRecordingContext());

    // The environment can request to various options for depth-stencil formats that could be
    // unavailable. Override the decision if it is not applicable.
    ApplyDepthStencilFormatToggles();

    // The environment can only request to use VK_KHR_zero_initialize_workgroup_memory when the
    // extension is available. Override the decision if it is no applicable.
    ApplyUseZeroInitializeWorkgroupMemoryExtensionToggle();

    SetLabelImpl();

    ToBackend(GetAdapter())->GetVulkanInstance()->StartListeningForDeviceMessages(this);

    return DeviceBase::Initialize(Queue::Create(this, &descriptor->defaultQueue));
}

Device::~Device() {
    Destroy();
}

ResultOrError<Ref<BindGroupBase>> Device::CreateBindGroupImpl(
    const BindGroupDescriptor* descriptor) {
    return BindGroup::Create(this, descriptor);
}
ResultOrError<Ref<BindGroupLayoutBase>> Device::CreateBindGroupLayoutImpl(
    const BindGroupLayoutDescriptor* descriptor,
    PipelineCompatibilityToken pipelineCompatibilityToken) {
    return BindGroupLayout::Create(this, descriptor, pipelineCompatibilityToken);
}
ResultOrError<Ref<BufferBase>> Device::CreateBufferImpl(const BufferDescriptor* descriptor) {
    return Buffer::Create(this, descriptor);
}
ResultOrError<Ref<CommandBufferBase>> Device::CreateCommandBuffer(
    CommandEncoder* encoder,
    const CommandBufferDescriptor* descriptor) {
    return CommandBuffer::Create(encoder, descriptor);
}
Ref<ComputePipelineBase> Device::CreateUninitializedComputePipelineImpl(
    const ComputePipelineDescriptor* descriptor) {
    return ComputePipeline::CreateUninitialized(this, descriptor);
}
ResultOrError<Ref<PipelineLayoutBase>> Device::CreatePipelineLayoutImpl(
    const PipelineLayoutDescriptor* descriptor) {
    return PipelineLayout::Create(this, descriptor);
}
ResultOrError<Ref<QuerySetBase>> Device::CreateQuerySetImpl(const QuerySetDescriptor* descriptor) {
    return QuerySet::Create(this, descriptor);
}
Ref<RenderPipelineBase> Device::CreateUninitializedRenderPipelineImpl(
    const RenderPipelineDescriptor* descriptor) {
    return RenderPipeline::CreateUninitialized(this, descriptor);
}
ResultOrError<Ref<SamplerBase>> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
    return Sampler::Create(this, descriptor);
}
ResultOrError<Ref<ShaderModuleBase>> Device::CreateShaderModuleImpl(
    const ShaderModuleDescriptor* descriptor,
    ShaderModuleParseResult* parseResult) {
    return ShaderModule::Create(this, descriptor, parseResult);
}
ResultOrError<Ref<SwapChainBase>> Device::CreateSwapChainImpl(
    const SwapChainDescriptor* descriptor) {
    return OldSwapChain::Create(this, descriptor);
}
ResultOrError<Ref<NewSwapChainBase>> Device::CreateSwapChainImpl(
    Surface* surface,
    NewSwapChainBase* previousSwapChain,
    const SwapChainDescriptor* descriptor) {
    return SwapChain::Create(this, surface, previousSwapChain, descriptor);
}
ResultOrError<Ref<TextureBase>> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
    return Texture::Create(this, descriptor);
}
ResultOrError<Ref<TextureViewBase>> Device::CreateTextureViewImpl(
    TextureBase* texture,
    const TextureViewDescriptor* descriptor) {
    return TextureView::Create(texture, descriptor);
}
Ref<PipelineCacheBase> Device::GetOrCreatePipelineCacheImpl(const CacheKey& key) {
    return PipelineCache::Create(this, key);
}
void Device::InitializeComputePipelineAsyncImpl(Ref<ComputePipelineBase> computePipeline,
                                                WGPUCreateComputePipelineAsyncCallback callback,
                                                void* userdata) {
    ComputePipeline::InitializeAsync(std::move(computePipeline), callback, userdata);
}
void Device::InitializeRenderPipelineAsyncImpl(Ref<RenderPipelineBase> renderPipeline,
                                               WGPUCreateRenderPipelineAsyncCallback callback,
                                               void* userdata) {
    RenderPipeline::InitializeAsync(std::move(renderPipeline), callback, userdata);
}

MaybeError Device::TickImpl() {
    RecycleCompletedCommands();

    ExecutionSerial completedSerial = GetCompletedCommandSerial();

    for (Ref<DescriptorSetAllocator>& allocator :
         mDescriptorAllocatorsPendingDeallocation.IterateUpTo(completedSerial)) {
        allocator->FinishDeallocation(completedSerial);
    }

    mResourceMemoryAllocator->Tick(completedSerial);
    mDeleter->Tick(completedSerial);
    mDescriptorAllocatorsPendingDeallocation.ClearUpTo(completedSerial);

    if (mRecordingContext.used) {
        DAWN_TRY(SubmitPendingCommands());
    }

    DAWN_TRY(CheckDebugLayerAndGenerateErrors());

    return {};
}

VkInstance Device::GetVkInstance() const {
    return ToBackend(GetAdapter())->GetVulkanInstance()->GetVkInstance();
}
const VulkanDeviceInfo& Device::GetDeviceInfo() const {
    return mDeviceInfo;
}

const VulkanGlobalInfo& Device::GetGlobalInfo() const {
    return ToBackend(GetAdapter())->GetVulkanInstance()->GetGlobalInfo();
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

FencedDeleter* Device::GetFencedDeleter() const {
    return mDeleter.get();
}

RenderPassCache* Device::GetRenderPassCache() const {
    return mRenderPassCache.get();
}

ResourceMemoryAllocator* Device::GetResourceMemoryAllocator() const {
    return mResourceMemoryAllocator.get();
}

void Device::EnqueueDeferredDeallocation(DescriptorSetAllocator* allocator) {
    mDescriptorAllocatorsPendingDeallocation.Enqueue(allocator, GetPendingCommandSerial());
}

CommandRecordingContext* Device::GetPendingRecordingContext() {
    ASSERT(mRecordingContext.commandBuffer != VK_NULL_HANDLE);
    mRecordingContext.used = true;
    return &mRecordingContext;
}

MaybeError Device::SubmitPendingCommands() {
    if (!mRecordingContext.used) {
        return {};
    }

    DAWN_TRY(
        CheckVkSuccess(fn.EndCommandBuffer(mRecordingContext.commandBuffer), "vkEndCommandBuffer"));

    std::vector<VkPipelineStageFlags> dstStageMasks(mRecordingContext.waitSemaphores.size(),
                                                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(mRecordingContext.waitSemaphores.size());
    submitInfo.pWaitSemaphores = AsVkArray(mRecordingContext.waitSemaphores.data());
    submitInfo.pWaitDstStageMask = dstStageMasks.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mRecordingContext.commandBuffer;
    submitInfo.signalSemaphoreCount =
        static_cast<uint32_t>(mRecordingContext.signalSemaphores.size());
    submitInfo.pSignalSemaphores = AsVkArray(mRecordingContext.signalSemaphores.data());

    VkFence fence = VK_NULL_HANDLE;
    DAWN_TRY_ASSIGN(fence, GetUnusedFence());
    DAWN_TRY_WITH_CLEANUP(
        CheckVkSuccess(fn.QueueSubmit(mQueue, 1, &submitInfo, fence), "vkQueueSubmit"), {
            // If submitting to the queue fails, move the fence back into the unused fence
            // list, as if it were never acquired. Not doing so would leak the fence since
            // it would be neither in the unused list nor in the in-flight list.
            mUnusedFences.push_back(fence);
        });

    // Enqueue the semaphores before incrementing the serial, so that they can be deleted as
    // soon as the current submission is finished.
    for (VkSemaphore semaphore : mRecordingContext.waitSemaphores) {
        mDeleter->DeleteWhenUnused(semaphore);
    }
    for (VkSemaphore semaphore : mRecordingContext.signalSemaphores) {
        mDeleter->DeleteWhenUnused(semaphore);
    }

    IncrementLastSubmittedCommandSerial();
    ExecutionSerial lastSubmittedSerial = GetLastSubmittedCommandSerial();
    mFencesInFlight.emplace(fence, lastSubmittedSerial);

    CommandPoolAndBuffer submittedCommands = {mRecordingContext.commandPool,
                                              mRecordingContext.commandBuffer};
    mCommandsInFlight.Enqueue(submittedCommands, lastSubmittedSerial);
    mRecordingContext = CommandRecordingContext();
    DAWN_TRY(PrepareRecordingContext());

    return {};
}

ResultOrError<VulkanDeviceKnobs> Device::CreateDevice(VkPhysicalDevice physicalDevice) {
    VulkanDeviceKnobs usedKnobs = {};

    // Default to asking for all avilable known extensions.
    usedKnobs.extensions = mDeviceInfo.extensions;

    // However only request the extensions that haven't been promoted in the device's apiVersion
    std::vector<const char*> extensionNames;
    for (DeviceExt ext : IterateBitSet(usedKnobs.extensions)) {
        const DeviceExtInfo& info = GetDeviceExtInfo(ext);

        if (info.versionPromoted > mDeviceInfo.properties.apiVersion) {
            extensionNames.push_back(info.name);
        }
    }

    // Some device features can only be enabled using a VkPhysicalDeviceFeatures2 struct, which
    // is supported by the VK_EXT_get_physical_properties2 instance extension, which was
    // promoted as a core API in Vulkan 1.1.
    //
    // Prepare a VkPhysicalDeviceFeatures2 struct for this use case, it will only be populated
    // if HasExt(DeviceExt::GetPhysicalDeviceProperties2) is true.
    VkPhysicalDeviceFeatures2 features2 = {};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = nullptr;
    PNextChainBuilder featuresChain(&features2);

    // Required for core WebGPU features.
    usedKnobs.features.depthBiasClamp = VK_TRUE;
    usedKnobs.features.fragmentStoresAndAtomics = VK_TRUE;
    usedKnobs.features.fullDrawIndexUint32 = VK_TRUE;
    usedKnobs.features.imageCubeArray = VK_TRUE;
    usedKnobs.features.independentBlend = VK_TRUE;
    usedKnobs.features.sampleRateShading = VK_TRUE;

    if (IsRobustnessEnabled()) {
        usedKnobs.features.robustBufferAccess = VK_TRUE;
    }

    if (mDeviceInfo.HasExt(DeviceExt::SubgroupSizeControl)) {
        ASSERT(usedKnobs.HasExt(DeviceExt::SubgroupSizeControl));

        // Always request all the features from VK_EXT_subgroup_size_control when available.
        usedKnobs.subgroupSizeControlFeatures = mDeviceInfo.subgroupSizeControlFeatures;
        featuresChain.Add(&usedKnobs.subgroupSizeControlFeatures);

        mComputeSubgroupSize = FindComputeSubgroupSize();
    }

    if (mDeviceInfo.HasExt(DeviceExt::ZeroInitializeWorkgroupMemory)) {
        ASSERT(usedKnobs.HasExt(DeviceExt::ZeroInitializeWorkgroupMemory));

        usedKnobs.zeroInitializeWorkgroupMemoryFeatures.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES_KHR;

        // Always allow initializing workgroup memory with OpConstantNull when available.
        // Note that the driver still won't initialize workgroup memory unless the workgroup
        // variable is explicitly initialized with OpConstantNull.
        usedKnobs.zeroInitializeWorkgroupMemoryFeatures.shaderZeroInitializeWorkgroupMemory =
            VK_TRUE;
        featuresChain.Add(&usedKnobs.zeroInitializeWorkgroupMemoryFeatures);
    }

    if (mDeviceInfo.features.samplerAnisotropy == VK_TRUE) {
        usedKnobs.features.samplerAnisotropy = VK_TRUE;
    }

    if (IsFeatureEnabled(Feature::TextureCompressionBC)) {
        ASSERT(ToBackend(GetAdapter())->GetDeviceInfo().features.textureCompressionBC == VK_TRUE);
        usedKnobs.features.textureCompressionBC = VK_TRUE;
    }

    if (IsFeatureEnabled(Feature::TextureCompressionETC2)) {
        ASSERT(ToBackend(GetAdapter())->GetDeviceInfo().features.textureCompressionETC2 == VK_TRUE);
        usedKnobs.features.textureCompressionETC2 = VK_TRUE;
    }

    if (IsFeatureEnabled(Feature::TextureCompressionASTC)) {
        ASSERT(ToBackend(GetAdapter())->GetDeviceInfo().features.textureCompressionASTC_LDR ==
               VK_TRUE);
        usedKnobs.features.textureCompressionASTC_LDR = VK_TRUE;
    }

    if (IsFeatureEnabled(Feature::PipelineStatisticsQuery)) {
        ASSERT(ToBackend(GetAdapter())->GetDeviceInfo().features.pipelineStatisticsQuery ==
               VK_TRUE);
        usedKnobs.features.pipelineStatisticsQuery = VK_TRUE;
    }

    if (IsFeatureEnabled(Feature::ShaderFloat16)) {
        const VulkanDeviceInfo& deviceInfo = ToBackend(GetAdapter())->GetDeviceInfo();
        ASSERT(deviceInfo.HasExt(DeviceExt::ShaderFloat16Int8) &&
               deviceInfo.shaderFloat16Int8Features.shaderFloat16 == VK_TRUE &&
               deviceInfo.HasExt(DeviceExt::_16BitStorage) &&
               deviceInfo._16BitStorageFeatures.storageBuffer16BitAccess == VK_TRUE &&
               deviceInfo._16BitStorageFeatures.uniformAndStorageBuffer16BitAccess == VK_TRUE);

        usedKnobs.shaderFloat16Int8Features.shaderFloat16 = VK_TRUE;
        usedKnobs._16BitStorageFeatures.storageBuffer16BitAccess = VK_TRUE;
        usedKnobs._16BitStorageFeatures.uniformAndStorageBuffer16BitAccess = VK_TRUE;

        featuresChain.Add(&usedKnobs.shaderFloat16Int8Features,
                          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR);
        featuresChain.Add(&usedKnobs._16BitStorageFeatures,
                          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES);
    }

    if (IsFeatureEnabled(Feature::DepthClamping)) {
        ASSERT(ToBackend(GetAdapter())->GetDeviceInfo().features.depthClamp == VK_TRUE);
        usedKnobs.features.depthClamp = VK_TRUE;
    }

    // Find a universal queue family
    {
        // Note that GRAPHICS and COMPUTE imply TRANSFER so we don't need to check for it.
        constexpr uint32_t kUniversalFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
        int universalQueueFamily = -1;
        for (unsigned int i = 0; i < mDeviceInfo.queueFamilies.size(); ++i) {
            if ((mDeviceInfo.queueFamilies[i].queueFlags & kUniversalFlags) == kUniversalFlags) {
                universalQueueFamily = i;
                break;
            }
        }

        if (universalQueueFamily == -1) {
            return DAWN_INTERNAL_ERROR("No universal queue family");
        }
        mQueueFamily = static_cast<uint32_t>(universalQueueFamily);
    }

    // Choose to create a single universal queue
    std::vector<VkDeviceQueueCreateInfo> queuesToRequest;
    float zero = 0.0f;
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
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
    createInfo.ppEnabledExtensionNames = extensionNames.data();

    // When we have DeviceExt::GetPhysicalDeviceProperties2, use features2 so that features not
    // covered by VkPhysicalDeviceFeatures can be enabled.
    if (mDeviceInfo.HasExt(DeviceExt::GetPhysicalDeviceProperties2)) {
        features2.features = usedKnobs.features;
        createInfo.pNext = &features2;
        createInfo.pEnabledFeatures = nullptr;
    } else {
        ASSERT(features2.pNext == nullptr);
        createInfo.pEnabledFeatures = &usedKnobs.features;
    }

    DAWN_TRY(CheckVkSuccess(fn.CreateDevice(physicalDevice, &createInfo, nullptr, &mVkDevice),
                            "vkCreateDevice"));

    return usedKnobs;
}

uint32_t Device::FindComputeSubgroupSize() const {
    if (!mDeviceInfo.HasExt(DeviceExt::SubgroupSizeControl)) {
        return 0;
    }

    const VkPhysicalDeviceSubgroupSizeControlPropertiesEXT& ext =
        mDeviceInfo.subgroupSizeControlProperties;

    if (ext.minSubgroupSize == ext.maxSubgroupSize) {
        return 0;
    }

    // At the moment, only Intel devices support varying subgroup sizes and 16, which is the
    // next value after the minimum of 8, is the sweet spot according to [1]. Hence the
    // following heuristics, which may need to be adjusted in the future for other
    // architectures, or if a specific API is added to let client code select the size..
    //
    // [1] https://bugs.freedesktop.org/show_bug.cgi?id=108875
    uint32_t subgroupSize = ext.minSubgroupSize * 2;
    if (subgroupSize <= ext.maxSubgroupSize) {
        return subgroupSize;
    } else {
        return ext.minSubgroupSize;
    }
}

void Device::GatherQueueFromDevice() {
    fn.GetDeviceQueue(mVkDevice, mQueueFamily, 0, &mQueue);
}

// Note that this function is called before mDeviceInfo is initialized.
void Device::InitTogglesFromDriver() {
    // TODO(crbug.com/dawn/857): tighten this workaround when this issue is fixed in both
    // Vulkan SPEC and drivers.
    SetToggle(Toggle::UseTemporaryBufferInCompressedTextureToTextureCopy, true);

    // By default try to use D32S8 for Depth24PlusStencil8
    SetToggle(Toggle::VulkanUseD32S8, true);

    // By default try to initialize workgroup memory with OpConstantNull according to the Vulkan
    // extension VK_KHR_zero_initialize_workgroup_memory.
    SetToggle(Toggle::VulkanUseZeroInitializeWorkgroupMemoryExtension, true);

    // By default try to use S8 if available.
    SetToggle(Toggle::VulkanUseS8, true);
}

void Device::ApplyDepthStencilFormatToggles() {
    bool supportsD32s8 =
        ToBackend(GetAdapter())->IsDepthStencilFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT);
    bool supportsD24s8 =
        ToBackend(GetAdapter())->IsDepthStencilFormatSupported(VK_FORMAT_D24_UNORM_S8_UINT);
    bool supportsS8 = ToBackend(GetAdapter())->IsDepthStencilFormatSupported(VK_FORMAT_S8_UINT);

    ASSERT(supportsD32s8 || supportsD24s8);

    if (!supportsD24s8) {
        ForceSetToggle(Toggle::VulkanUseD32S8, true);
    }
    if (!supportsD32s8) {
        ForceSetToggle(Toggle::VulkanUseD32S8, false);
    }
    if (!supportsS8) {
        ForceSetToggle(Toggle::VulkanUseS8, false);
    }
}

void Device::ApplyUseZeroInitializeWorkgroupMemoryExtensionToggle() {
    if (!mDeviceInfo.HasExt(DeviceExt::ZeroInitializeWorkgroupMemory)) {
        ForceSetToggle(Toggle::VulkanUseZeroInitializeWorkgroupMemoryExtension, false);
    }
}

VulkanFunctions* Device::GetMutableFunctions() {
    return const_cast<VulkanFunctions*>(&fn);
}

ResultOrError<VkFence> Device::GetUnusedFence() {
    if (!mUnusedFences.empty()) {
        VkFence fence = mUnusedFences.back();
        DAWN_TRY(CheckVkSuccess(fn.ResetFences(mVkDevice, 1, &*fence), "vkResetFences"));

        mUnusedFences.pop_back();
        return fence;
    }

    VkFenceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    VkFence fence = VK_NULL_HANDLE;
    DAWN_TRY(
        CheckVkSuccess(fn.CreateFence(mVkDevice, &createInfo, nullptr, &*fence), "vkCreateFence"));

    return fence;
}

ResultOrError<ExecutionSerial> Device::CheckAndUpdateCompletedSerials() {
    ExecutionSerial fenceSerial(0);
    while (!mFencesInFlight.empty()) {
        VkFence fence = mFencesInFlight.front().first;
        ExecutionSerial tentativeSerial = mFencesInFlight.front().second;
        VkResult result = VkResult::WrapUnsafe(
            INJECT_ERROR_OR_RUN(fn.GetFenceStatus(mVkDevice, fence), VK_ERROR_DEVICE_LOST));

        // Fence are added in order, so we can stop searching as soon
        // as we see one that's not ready.
        if (result == VK_NOT_READY) {
            return fenceSerial;
        } else {
            DAWN_TRY(CheckVkSuccess(::VkResult(result), "GetFenceStatus"));
        }

        // Update fenceSerial since fence is ready.
        fenceSerial = tentativeSerial;

        mUnusedFences.push_back(fence);

        ASSERT(fenceSerial > GetCompletedCommandSerial());
        mFencesInFlight.pop();
    }
    return fenceSerial;
}

MaybeError Device::PrepareRecordingContext() {
    ASSERT(!mRecordingContext.used);
    ASSERT(mRecordingContext.commandBuffer == VK_NULL_HANDLE);
    ASSERT(mRecordingContext.commandPool == VK_NULL_HANDLE);

    // First try to recycle unused command pools.
    if (!mUnusedCommands.empty()) {
        CommandPoolAndBuffer commands = mUnusedCommands.back();
        mUnusedCommands.pop_back();
        DAWN_TRY_WITH_CLEANUP(
            CheckVkSuccess(fn.ResetCommandPool(mVkDevice, commands.pool, 0), "vkResetCommandPool"),
            {
                // vkResetCommandPool failed (it may return out-of-memory).
                // Free the commands in the cleanup step before returning to
                // reclaim memory.

                // The VkCommandBuffer memory should be wholly owned by the
                // pool and freed when it is destroyed, but that's not the
                // case in some drivers and they leak memory. So we call
                // FreeCommandBuffers before DestroyCommandPool to be safe.
                // TODO(enga): Only do this on a known list of bad drivers.
                fn.FreeCommandBuffers(mVkDevice, commands.pool, 1, &commands.commandBuffer);
                fn.DestroyCommandPool(mVkDevice, commands.pool, nullptr);
            });

        mRecordingContext.commandBuffer = commands.commandBuffer;
        mRecordingContext.commandPool = commands.pool;
    } else {
        // Create a new command pool for our commands and allocate the command buffer.
        VkCommandPoolCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        createInfo.queueFamilyIndex = mQueueFamily;

        DAWN_TRY(CheckVkSuccess(
            fn.CreateCommandPool(mVkDevice, &createInfo, nullptr, &*mRecordingContext.commandPool),
            "vkCreateCommandPool"));

        VkCommandBufferAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.commandPool = mRecordingContext.commandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;

        DAWN_TRY(CheckVkSuccess(
            fn.AllocateCommandBuffers(mVkDevice, &allocateInfo, &mRecordingContext.commandBuffer),
            "vkAllocateCommandBuffers"));
    }

    // Start the recording of commands in the command buffer.
    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    return CheckVkSuccess(fn.BeginCommandBuffer(mRecordingContext.commandBuffer, &beginInfo),
                          "vkBeginCommandBuffer");
}

void Device::RecycleCompletedCommands() {
    for (auto& commands : mCommandsInFlight.IterateUpTo(GetCompletedCommandSerial())) {
        mUnusedCommands.push_back(commands);
    }
    mCommandsInFlight.ClearUpTo(GetCompletedCommandSerial());
}

ResultOrError<std::unique_ptr<StagingBufferBase>> Device::CreateStagingBuffer(size_t size) {
    std::unique_ptr<StagingBufferBase> stagingBuffer = std::make_unique<StagingBuffer>(size, this);
    DAWN_TRY(stagingBuffer->Initialize());
    return std::move(stagingBuffer);
}

MaybeError Device::CopyFromStagingToBuffer(StagingBufferBase* source,
                                           uint64_t sourceOffset,
                                           BufferBase* destination,
                                           uint64_t destinationOffset,
                                           uint64_t size) {
    // It is a validation error to do a 0-sized copy in Vulkan, check it is skipped prior to
    // calling this function.
    ASSERT(size != 0);

    CommandRecordingContext* recordingContext = GetPendingRecordingContext();

    ToBackend(destination)
        ->EnsureDataInitializedAsDestination(recordingContext, destinationOffset, size);

    // There is no need of a barrier to make host writes available and visible to the copy
    // operation for HOST_COHERENT memory. The Vulkan spec for vkQueueSubmit describes that it
    // does an implicit availability, visibility and domain operation.

    // Insert pipeline barrier to ensure correct ordering with previous memory operations on the
    // buffer.
    ToBackend(destination)->TransitionUsageNow(recordingContext, wgpu::BufferUsage::CopyDst);

    VkBufferCopy copy;
    copy.srcOffset = sourceOffset;
    copy.dstOffset = destinationOffset;
    copy.size = size;

    this->fn.CmdCopyBuffer(recordingContext->commandBuffer, ToBackend(source)->GetBufferHandle(),
                           ToBackend(destination)->GetHandle(), 1, &copy);

    return {};
}

MaybeError Device::CopyFromStagingToTexture(const StagingBufferBase* source,
                                            const TextureDataLayout& src,
                                            TextureCopy* dst,
                                            const Extent3D& copySizePixels) {
    // There is no need of a barrier to make host writes available and visible to the copy
    // operation for HOST_COHERENT memory. The Vulkan spec for vkQueueSubmit describes that it
    // does an implicit availability, visibility and domain operation.

    CommandRecordingContext* recordingContext = GetPendingRecordingContext();

    VkBufferImageCopy region = ComputeBufferImageCopyRegion(src, *dst, copySizePixels);
    VkImageSubresourceLayers subresource = region.imageSubresource;

    SubresourceRange range = GetSubresourcesAffectedByCopy(*dst, copySizePixels);

    if (IsCompleteSubresourceCopiedTo(dst->texture.Get(), copySizePixels, subresource.mipLevel)) {
        // Since texture has been overwritten, it has been "initialized"
        dst->texture->SetIsSubresourceContentInitialized(true, range);
    } else {
        ToBackend(dst->texture)->EnsureSubresourceContentInitialized(recordingContext, range);
    }
    // Insert pipeline barrier to ensure correct ordering with previous memory operations on the
    // texture.
    ToBackend(dst->texture)
        ->TransitionUsageNow(recordingContext, wgpu::TextureUsage::CopyDst, range);
    VkImage dstImage = ToBackend(dst->texture)->GetHandle();

    // Dawn guarantees dstImage be in the TRANSFER_DST_OPTIMAL layout after the
    // copy command.
    this->fn.CmdCopyBufferToImage(recordingContext->commandBuffer,
                                  ToBackend(source)->GetBufferHandle(), dstImage,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    return {};
}

MaybeError Device::ImportExternalImage(const ExternalImageDescriptorVk* descriptor,
                                       ExternalMemoryHandle memoryHandle,
                                       VkImage image,
                                       const std::vector<ExternalSemaphoreHandle>& waitHandles,
                                       VkSemaphore* outSignalSemaphore,
                                       VkDeviceMemory* outAllocation,
                                       std::vector<VkSemaphore>* outWaitSemaphores) {
    const TextureDescriptor* textureDescriptor = FromAPI(descriptor->cTextureDescriptor);

    const DawnTextureInternalUsageDescriptor* internalUsageDesc = nullptr;
    FindInChain(textureDescriptor->nextInChain, &internalUsageDesc);

    wgpu::TextureUsage usage = textureDescriptor->usage;
    if (internalUsageDesc != nullptr) {
        usage |= internalUsageDesc->internalUsage;
    }

    // Check services support this combination of handle type / image info
    DAWN_INVALID_IF(!mExternalSemaphoreService->Supported(),
                    "External semaphore usage not supported");

    DAWN_INVALID_IF(!mExternalMemoryService->SupportsImportMemory(
                        VulkanImageFormat(this, textureDescriptor->format), VK_IMAGE_TYPE_2D,
                        VK_IMAGE_TILING_OPTIMAL,
                        VulkanImageUsage(usage, GetValidInternalFormat(textureDescriptor->format)),
                        VK_IMAGE_CREATE_ALIAS_BIT_KHR),
                    "External memory usage not supported");

    // Create an external semaphore to signal when the texture is done being used
    DAWN_TRY_ASSIGN(*outSignalSemaphore, mExternalSemaphoreService->CreateExportableSemaphore());

    // Import the external image's memory
    external_memory::MemoryImportParams importParams;
    DAWN_TRY_ASSIGN(importParams, mExternalMemoryService->GetMemoryImportParams(descriptor, image));
    DAWN_TRY_ASSIGN(*outAllocation,
                    mExternalMemoryService->ImportMemory(memoryHandle, importParams, image));

    // Import semaphores we have to wait on before using the texture
    for (const ExternalSemaphoreHandle& handle : waitHandles) {
        VkSemaphore semaphore = VK_NULL_HANDLE;
        DAWN_TRY_ASSIGN(semaphore, mExternalSemaphoreService->ImportSemaphore(handle));
        outWaitSemaphores->push_back(semaphore);
    }

    return {};
}

bool Device::SignalAndExportExternalTexture(
    Texture* texture,
    VkImageLayout desiredLayout,
    ExternalImageExportInfoVk* info,
    std::vector<ExternalSemaphoreHandle>* semaphoreHandles) {
    return !ConsumedError([&]() -> MaybeError {
        DAWN_TRY(ValidateObject(texture));

        VkSemaphore signalSemaphore;
        VkImageLayout releasedOldLayout;
        VkImageLayout releasedNewLayout;
        DAWN_TRY(texture->ExportExternalTexture(desiredLayout, &signalSemaphore, &releasedOldLayout,
                                                &releasedNewLayout));

        ExternalSemaphoreHandle semaphoreHandle;
        DAWN_TRY_ASSIGN(semaphoreHandle,
                        mExternalSemaphoreService->ExportSemaphore(signalSemaphore));
        semaphoreHandles->push_back(semaphoreHandle);
        info->releasedOldLayout = releasedOldLayout;
        info->releasedNewLayout = releasedNewLayout;
        info->isInitialized =
            texture->IsSubresourceContentInitialized(texture->GetAllSubresources());

        return {};
    }());
}

TextureBase* Device::CreateTextureWrappingVulkanImage(
    const ExternalImageDescriptorVk* descriptor,
    ExternalMemoryHandle memoryHandle,
    const std::vector<ExternalSemaphoreHandle>& waitHandles) {
    const TextureDescriptor* textureDescriptor = FromAPI(descriptor->cTextureDescriptor);

    // Initial validation
    if (ConsumedError(ValidateTextureDescriptor(this, textureDescriptor))) {
        return nullptr;
    }
    if (ConsumedError(ValidateVulkanImageCanBeWrapped(this, textureDescriptor),
                      "validating that a Vulkan image can be wrapped with %s.",
                      textureDescriptor)) {
        return nullptr;
    }

    VkSemaphore signalSemaphore = VK_NULL_HANDLE;
    VkDeviceMemory allocation = VK_NULL_HANDLE;
    std::vector<VkSemaphore> waitSemaphores;
    waitSemaphores.reserve(waitHandles.size());

    // Cleanup in case of a failure, the image creation doesn't acquire the external objects
    // if a failure happems.
    Texture* result = nullptr;
    // TODO(crbug.com/1026480): Consolidate this into a single CreateFromExternal call.
    if (ConsumedError(Texture::CreateFromExternal(this, descriptor, textureDescriptor,
                                                  mExternalMemoryService.get()),
                      &result) ||
        ConsumedError(ImportExternalImage(descriptor, memoryHandle, result->GetHandle(),
                                          waitHandles, &signalSemaphore, &allocation,
                                          &waitSemaphores)) ||
        ConsumedError(
            result->BindExternalMemory(descriptor, signalSemaphore, allocation, waitSemaphores))) {
        // Delete the Texture if it was created
        if (result != nullptr) {
            result->Release();
        }

        // Clear the signal semaphore
        fn.DestroySemaphore(GetVkDevice(), signalSemaphore, nullptr);

        // Clear image memory
        fn.FreeMemory(GetVkDevice(), allocation, nullptr);

        // Clear any wait semaphores we were able to import
        for (VkSemaphore semaphore : waitSemaphores) {
            fn.DestroySemaphore(GetVkDevice(), semaphore, nullptr);
        }
        return nullptr;
    }

    return result;
}

uint32_t Device::GetComputeSubgroupSize() const {
    return mComputeSubgroupSize;
}

void Device::OnDebugMessage(std::string message) {
    mDebugMessages.push_back(std::move(message));
}

MaybeError Device::CheckDebugLayerAndGenerateErrors() {
    if (!GetAdapter()->GetInstance()->IsBackendValidationEnabled() || mDebugMessages.empty()) {
        return {};
    }

    auto error = DAWN_INTERNAL_ERROR("The Vulkan validation layer reported uncaught errors.");

    AppendDebugLayerMessages(error.get());

    return error;
}

void Device::AppendDebugLayerMessages(ErrorData* error) {
    if (!GetAdapter()->GetInstance()->IsBackendValidationEnabled()) {
        return;
    }

    while (!mDebugMessages.empty()) {
        error->AppendBackendMessage(std::move(mDebugMessages.back()));
        mDebugMessages.pop_back();
    }
}

MaybeError Device::WaitForIdleForDestruction() {
    // Immediately tag the recording context as unused so we don't try to submit it in Tick.
    // Move the mRecordingContext.used to mUnusedCommands so it can be cleaned up in
    // ShutDownImpl
    if (mRecordingContext.used) {
        CommandPoolAndBuffer commands = {mRecordingContext.commandPool,
                                         mRecordingContext.commandBuffer};
        mUnusedCommands.push_back(commands);
        mRecordingContext = CommandRecordingContext();
    }

    VkResult waitIdleResult = VkResult::WrapUnsafe(fn.QueueWaitIdle(mQueue));
    // Ignore the result of QueueWaitIdle: it can return OOM which we can't really do anything
    // about, Device lost, which means workloads running on the GPU are no longer accessible
    // (so they are as good as waited on) or success.
    DAWN_UNUSED(waitIdleResult);

    // Make sure all fences are complete by explicitly waiting on them all
    while (!mFencesInFlight.empty()) {
        VkFence fence = mFencesInFlight.front().first;
        ExecutionSerial fenceSerial = mFencesInFlight.front().second;
        ASSERT(fenceSerial > GetCompletedCommandSerial());

        VkResult result = VkResult::WrapUnsafe(VK_TIMEOUT);
        do {
            // If WaitForIdleForDesctruction is called while we are Disconnected, it means that
            // the device lost came from the ErrorInjector and we need to wait without allowing
            // any more error to be injected. This is because the device lost was "fake" and
            // commands might still be running.
            if (GetState() == State::Disconnected) {
                result =
                    VkResult::WrapUnsafe(fn.WaitForFences(mVkDevice, 1, &*fence, true, UINT64_MAX));
                continue;
            }

            result = VkResult::WrapUnsafe(INJECT_ERROR_OR_RUN(
                fn.WaitForFences(mVkDevice, 1, &*fence, true, UINT64_MAX), VK_ERROR_DEVICE_LOST));
        } while (result == VK_TIMEOUT);
        // Ignore errors from vkWaitForFences: it can be either OOM which we can't do anything
        // about (and we need to keep going with the destruction of all fences), or device
        // loss, which means the workload on the GPU is no longer accessible and we can
        // safely destroy the fence.

        fn.DestroyFence(mVkDevice, fence, nullptr);
        mFencesInFlight.pop();
    }
    return {};
}

void Device::DestroyImpl() {
    ASSERT(GetState() == State::Disconnected);

    // We failed during initialization so early that we don't even have a VkDevice. There is
    // nothing to do.
    if (mVkDevice == VK_NULL_HANDLE) {
        return;
    }

    // The deleter is the second thing we initialize. If it is not present, it means that
    // only the VkDevice was created and nothing else. Destroy the device and do nothing else
    // because the function pointers might not have been loaded (and there is nothing to
    // destroy anyway).
    if (mDeleter == nullptr) {
        fn.DestroyDevice(mVkDevice, nullptr);
        mVkDevice = VK_NULL_HANDLE;
        return;
    }

    // Enough of the Device's initialization happened that we can now do regular robust
    // deinitialization.

    ToBackend(GetAdapter())->GetVulkanInstance()->StopListeningForDeviceMessages(this);

    // Immediately tag the recording context as unused so we don't try to submit it in Tick.
    mRecordingContext.used = false;
    if (mRecordingContext.commandPool != VK_NULL_HANDLE) {
        // The VkCommandBuffer memory should be wholly owned by the pool and freed when it is
        // destroyed, but that's not the case in some drivers and the leak memory.
        // So we call FreeCommandBuffers before DestroyCommandPool to be safe.
        // TODO(enga): Only do this on a known list of bad drivers.
        fn.FreeCommandBuffers(mVkDevice, mRecordingContext.commandPool, 1,
                              &mRecordingContext.commandBuffer);
        fn.DestroyCommandPool(mVkDevice, mRecordingContext.commandPool, nullptr);
    }

    for (VkSemaphore semaphore : mRecordingContext.waitSemaphores) {
        fn.DestroySemaphore(mVkDevice, semaphore, nullptr);
    }
    mRecordingContext.waitSemaphores.clear();

    for (VkSemaphore semaphore : mRecordingContext.signalSemaphores) {
        fn.DestroySemaphore(mVkDevice, semaphore, nullptr);
    }
    mRecordingContext.signalSemaphores.clear();

    // Some commands might still be marked as in-flight if we shut down because of a device
    // loss. Recycle them as unused so that we free them below.
    RecycleCompletedCommands();
    ASSERT(mCommandsInFlight.Empty());

    for (const CommandPoolAndBuffer& commands : mUnusedCommands) {
        // The VkCommandBuffer memory should be wholly owned by the pool and freed when it is
        // destroyed, but that's not the case in some drivers and the leak memory.
        // So we call FreeCommandBuffers before DestroyCommandPool to be safe.
        // TODO(enga): Only do this on a known list of bad drivers.
        fn.FreeCommandBuffers(mVkDevice, commands.pool, 1, &commands.commandBuffer);
        fn.DestroyCommandPool(mVkDevice, commands.pool, nullptr);
    }
    mUnusedCommands.clear();

    // Some fences might still be marked as in-flight if we shut down because of a device loss.
    // Delete them since at this point all commands are complete.
    while (!mFencesInFlight.empty()) {
        fn.DestroyFence(mVkDevice, *mFencesInFlight.front().first, nullptr);
        mFencesInFlight.pop();
    }

    for (VkFence fence : mUnusedFences) {
        fn.DestroyFence(mVkDevice, fence, nullptr);
    }
    mUnusedFences.clear();

    ExecutionSerial completedSerial = GetCompletedCommandSerial();
    for (Ref<DescriptorSetAllocator>& allocator :
         mDescriptorAllocatorsPendingDeallocation.IterateUpTo(completedSerial)) {
        allocator->FinishDeallocation(completedSerial);
    }

    // Releasing the uploader enqueues buffers to be released.
    // Call Tick() again to clear them before releasing the deleter.
    mResourceMemoryAllocator->Tick(completedSerial);
    mDeleter->Tick(completedSerial);
    mDescriptorAllocatorsPendingDeallocation.ClearUpTo(completedSerial);

    // Allow recycled memory to be deleted.
    mResourceMemoryAllocator->DestroyPool();

    // The VkRenderPasses in the cache can be destroyed immediately since all commands referring
    // to them are guaranteed to be finished executing.
    mRenderPassCache = nullptr;

    // We need handle deleting all child objects by calling Tick() again with a large serial to
    // force all operations to look as if they were completed, and delete all objects before
    // destroying the Deleter and vkDevice.
    ASSERT(mDeleter != nullptr);
    mDeleter->Tick(kMaxExecutionSerial);
    mDeleter = nullptr;

    // VkQueues are destroyed when the VkDevice is destroyed
    // The VkDevice is needed to destroy child objects, so it must be destroyed last after all
    // child objects have been deleted.
    ASSERT(mVkDevice != VK_NULL_HANDLE);
    fn.DestroyDevice(mVkDevice, nullptr);
    mVkDevice = VK_NULL_HANDLE;
}

uint32_t Device::GetOptimalBytesPerRowAlignment() const {
    return mDeviceInfo.properties.limits.optimalBufferCopyRowPitchAlignment;
}

uint64_t Device::GetOptimalBufferToTextureCopyOffsetAlignment() const {
    return mDeviceInfo.properties.limits.optimalBufferCopyOffsetAlignment;
}

float Device::GetTimestampPeriodInNS() const {
    return mDeviceInfo.properties.limits.timestampPeriod;
}

void Device::SetLabelImpl() {
    SetDebugName(this, VK_OBJECT_TYPE_DEVICE, mVkDevice, "Dawn_Device", GetLabel());
}

}  // namespace dawn::native::vulkan
