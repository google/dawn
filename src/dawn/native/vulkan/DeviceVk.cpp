// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/vulkan/DeviceVk.h"

#include "dawn/common/Log.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/common/Platform.h"
#include "dawn/native/BackendConnection.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Error.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/Instance.h"
#include "dawn/native/VulkanBackend.h"
#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/BindGroupLayoutVk.h"
#include "dawn/native/vulkan/BindGroupVk.h"
#include "dawn/native/vulkan/BufferVk.h"
#include "dawn/native/vulkan/CommandBufferVk.h"
#include "dawn/native/vulkan/ComputePipelineVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/PipelineCacheVk.h"
#include "dawn/native/vulkan/PipelineLayoutVk.h"
#include "dawn/native/vulkan/QuerySetVk.h"
#include "dawn/native/vulkan/QueueVk.h"
#include "dawn/native/vulkan/RenderPassCache.h"
#include "dawn/native/vulkan/RenderPipelineVk.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/native/vulkan/SamplerVk.h"
#include "dawn/native/vulkan/ShaderModuleVk.h"
#include "dawn/native/vulkan/SwapChainVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

// static
ResultOrError<Ref<Device>> Device::Create(AdapterBase* adapter,
                                          const DeviceDescriptor* descriptor,
                                          const TogglesState& deviceToggles) {
    Ref<Device> device = AcquireRef(new Device(adapter, descriptor, deviceToggles));
    DAWN_TRY(device->Initialize(descriptor));
    return device;
}

Device::Device(AdapterBase* adapter,
               const DeviceDescriptor* descriptor,
               const TogglesState& deviceToggles)
    : DeviceBase(adapter, descriptor, deviceToggles), mDebugPrefix(GetNextDeviceDebugPrefix()) {}

MaybeError Device::Initialize(const DeviceDescriptor* descriptor) {
    // Copy the adapter's device info to the device so that we can change the "knobs"
    mDeviceInfo = ToBackend(GetPhysicalDevice())->GetDeviceInfo();

    // Initialize the "instance" procs of our local function table.
    VulkanFunctions* functions = GetMutableFunctions();
    *functions = ToBackend(GetPhysicalDevice())->GetVulkanInstance()->GetFunctions();

    // Two things are crucial if device initialization fails: the function pointers to destroy
    // objects, and the fence deleter that calls these functions. Do not do anything before
    // these two are set up, so that a failed initialization doesn't cause a crash in
    // DestroyImpl()
    {
        VkPhysicalDevice vkPhysicalDevice = ToBackend(GetPhysicalDevice())->GetVkPhysicalDevice();

        VulkanDeviceKnobs usedDeviceKnobs = {};
        DAWN_TRY_ASSIGN(usedDeviceKnobs, CreateDevice(vkPhysicalDevice));
        *static_cast<VulkanDeviceKnobs*>(&mDeviceInfo) = usedDeviceKnobs;

        DAWN_TRY(functions->LoadDeviceProcs(mVkDevice, mDeviceInfo));

        mDeleter = std::make_unique<MutexProtected<FencedDeleter>>(this);
    }

    mRenderPassCache = std::make_unique<RenderPassCache>(this);
    mResourceMemoryAllocator = std::make_unique<MutexProtected<ResourceMemoryAllocator>>(this);

    mExternalMemoryService = std::make_unique<external_memory::Service>(this);
    mExternalSemaphoreService = std::make_unique<external_semaphore::Service>(this);

    SetLabelImpl();

    ToBackend(GetPhysicalDevice())->GetVulkanInstance()->StartListeningForDeviceMessages(this);

    Ref<Queue> queue;
    DAWN_TRY_ASSIGN(queue, Queue::Create(this, &descriptor->defaultQueue, mMainQueueFamily));

    return DeviceBase::Initialize(std::move(queue));
}

Device::~Device() {
    Destroy();
}

ResultOrError<Ref<BindGroupBase>> Device::CreateBindGroupImpl(
    const BindGroupDescriptor* descriptor) {
    return BindGroup::Create(this, descriptor);
}
ResultOrError<Ref<BindGroupLayoutInternalBase>> Device::CreateBindGroupLayoutImpl(
    const BindGroupLayoutDescriptor* descriptor) {
    return BindGroupLayout::Create(this, descriptor);
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
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* compilationMessages) {
    return ShaderModule::Create(this, descriptor, parseResult, compilationMessages);
}
ResultOrError<Ref<SwapChainBase>> Device::CreateSwapChainImpl(
    Surface* surface,
    SwapChainBase* previousSwapChain,
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

ResultOrError<wgpu::TextureUsage> Device::GetSupportedSurfaceUsageImpl(
    const Surface* surface) const {
    return SwapChain::GetSupportedSurfaceUsage(this, surface);
}

MaybeError Device::TickImpl() {
    Queue* queue = ToBackend(GetQueue());

    ExecutionSerial completedSerial = queue->GetCompletedCommandSerial();
    queue->RecycleCompletedCommands(completedSerial);

    for (Ref<DescriptorSetAllocator>& allocator :
         mDescriptorAllocatorsPendingDeallocation.IterateUpTo(completedSerial)) {
        allocator->FinishDeallocation(completedSerial);
    }

    GetResourceMemoryAllocator()->Tick(completedSerial);
    GetFencedDeleter()->Tick(completedSerial);
    mDescriptorAllocatorsPendingDeallocation.ClearUpTo(completedSerial);

    DAWN_TRY(queue->SubmitPendingCommands());
    DAWN_TRY(CheckDebugLayerAndGenerateErrors());

    return {};
}

VkInstance Device::GetVkInstance() const {
    return ToBackend(GetPhysicalDevice())->GetVulkanInstance()->GetVkInstance();
}
const VulkanDeviceInfo& Device::GetDeviceInfo() const {
    return mDeviceInfo;
}

const VulkanGlobalInfo& Device::GetGlobalInfo() const {
    return ToBackend(GetPhysicalDevice())->GetVulkanInstance()->GetGlobalInfo();
}

VkDevice Device::GetVkDevice() const {
    return mVkDevice;
}

uint32_t Device::GetGraphicsQueueFamily() const {
    return mMainQueueFamily;
}

MutexProtected<FencedDeleter>& Device::GetFencedDeleter() const {
    return *mDeleter;
}

RenderPassCache* Device::GetRenderPassCache() const {
    return mRenderPassCache.get();
}

MutexProtected<ResourceMemoryAllocator>& Device::GetResourceMemoryAllocator() const {
    return *mResourceMemoryAllocator;
}

external_semaphore::Service* Device::GetExternalSemaphoreService() const {
    return mExternalSemaphoreService.get();
}

void Device::EnqueueDeferredDeallocation(DescriptorSetAllocator* allocator) {
    mDescriptorAllocatorsPendingDeallocation.Enqueue(allocator, GetPendingCommandSerial());
}

ResultOrError<VulkanDeviceKnobs> Device::CreateDevice(VkPhysicalDevice vkPhysicalDevice) {
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
        DAWN_ASSERT(usedKnobs.HasExt(DeviceExt::SubgroupSizeControl));

        // Always request all the features from VK_EXT_subgroup_size_control when available.
        usedKnobs.subgroupSizeControlFeatures = mDeviceInfo.subgroupSizeControlFeatures;
        featuresChain.Add(&usedKnobs.subgroupSizeControlFeatures);
    }

    if (mDeviceInfo.HasExt(DeviceExt::ZeroInitializeWorkgroupMemory)) {
        DAWN_ASSERT(usedKnobs.HasExt(DeviceExt::ZeroInitializeWorkgroupMemory));

        // Always allow initializing workgroup memory with OpConstantNull when available.
        // Note that the driver still won't initialize workgroup memory unless the workgroup
        // variable is explicitly initialized with OpConstantNull.
        usedKnobs.zeroInitializeWorkgroupMemoryFeatures =
            mDeviceInfo.zeroInitializeWorkgroupMemoryFeatures;
        featuresChain.Add(&usedKnobs.zeroInitializeWorkgroupMemoryFeatures);
    }

    if (mDeviceInfo.HasExt(DeviceExt::ShaderIntegerDotProduct)) {
        DAWN_ASSERT(usedKnobs.HasExt(DeviceExt::ShaderIntegerDotProduct));

        usedKnobs.shaderIntegerDotProductFeatures = mDeviceInfo.shaderIntegerDotProductFeatures;
        featuresChain.Add(&usedKnobs.shaderIntegerDotProductFeatures);
    }

    if (mDeviceInfo.features.samplerAnisotropy == VK_TRUE) {
        usedKnobs.features.samplerAnisotropy = VK_TRUE;
    }

    if (HasFeature(Feature::TextureCompressionBC)) {
        DAWN_ASSERT(ToBackend(GetPhysicalDevice())->GetDeviceInfo().features.textureCompressionBC ==
                    VK_TRUE);
        usedKnobs.features.textureCompressionBC = VK_TRUE;
    }

    if (HasFeature(Feature::TextureCompressionETC2)) {
        DAWN_ASSERT(
            ToBackend(GetPhysicalDevice())->GetDeviceInfo().features.textureCompressionETC2 ==
            VK_TRUE);
        usedKnobs.features.textureCompressionETC2 = VK_TRUE;
    }

    if (HasFeature(Feature::TextureCompressionASTC)) {
        DAWN_ASSERT(
            ToBackend(GetPhysicalDevice())->GetDeviceInfo().features.textureCompressionASTC_LDR ==
            VK_TRUE);
        usedKnobs.features.textureCompressionASTC_LDR = VK_TRUE;
    }

    if (HasFeature(Feature::DepthClipControl)) {
        usedKnobs.features.depthClamp = VK_TRUE;
    }

    // TODO(dawn:1510, tint:1473): After implementing a transform to handle the pipeline input /
    // output if necessary, relax the requirement of storageInputOutput16.
    if (HasFeature(Feature::ShaderF16)) {
        const VulkanDeviceInfo& deviceInfo = ToBackend(GetPhysicalDevice())->GetDeviceInfo();
        DAWN_ASSERT(deviceInfo.HasExt(DeviceExt::ShaderFloat16Int8) &&
                    deviceInfo.shaderFloat16Int8Features.shaderFloat16 == VK_TRUE &&
                    deviceInfo.HasExt(DeviceExt::_16BitStorage) &&
                    deviceInfo._16BitStorageFeatures.storageBuffer16BitAccess == VK_TRUE &&
                    deviceInfo._16BitStorageFeatures.storageInputOutput16 == VK_TRUE &&
                    deviceInfo._16BitStorageFeatures.uniformAndStorageBuffer16BitAccess == VK_TRUE);

        usedKnobs.shaderFloat16Int8Features.shaderFloat16 = VK_TRUE;
        usedKnobs._16BitStorageFeatures.storageBuffer16BitAccess = VK_TRUE;
        usedKnobs._16BitStorageFeatures.storageInputOutput16 = VK_TRUE;
        usedKnobs._16BitStorageFeatures.uniformAndStorageBuffer16BitAccess = VK_TRUE;

        featuresChain.Add(&usedKnobs.shaderFloat16Int8Features,
                          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR);
        featuresChain.Add(&usedKnobs._16BitStorageFeatures,
                          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES);
    }

    if (HasFeature(Feature::DualSourceBlending)) {
        usedKnobs.features.dualSrcBlend = VK_TRUE;
    }

    if (IsRobustnessEnabled() && mDeviceInfo.HasExt(DeviceExt::Robustness2)) {
        DAWN_ASSERT(usedKnobs.HasExt(DeviceExt::Robustness2));

        usedKnobs.robustness2Features = mDeviceInfo.robustness2Features;
        featuresChain.Add(&usedKnobs.robustness2Features);
    }

    if (HasFeature(Feature::ChromiumExperimentalSubgroupUniformControlFlow)) {
        DAWN_ASSERT(
            usedKnobs.HasExt(DeviceExt::ShaderSubgroupUniformControlFlow) &&
            mDeviceInfo.shaderSubgroupUniformControlFlowFeatures.shaderSubgroupUniformControlFlow ==
                VK_TRUE);

        usedKnobs.shaderSubgroupUniformControlFlowFeatures =
            mDeviceInfo.shaderSubgroupUniformControlFlowFeatures;
        featuresChain.Add(&usedKnobs.shaderSubgroupUniformControlFlowFeatures);
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
        mMainQueueFamily = static_cast<uint32_t>(universalQueueFamily);
    }

    // Choose to create a single universal queue
    std::vector<VkDeviceQueueCreateInfo> queuesToRequest;
    float zero = 0.0f;
    {
        VkDeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = nullptr;
        queueCreateInfo.flags = 0;
        queueCreateInfo.queueFamilyIndex = static_cast<uint32_t>(mMainQueueFamily);
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
        DAWN_ASSERT(features2.pNext == nullptr);
        createInfo.pEnabledFeatures = &usedKnobs.features;
    }

    DAWN_TRY(CheckVkSuccess(fn.CreateDevice(vkPhysicalDevice, &createInfo, nullptr, &mVkDevice),
                            "vkCreateDevice"));

    return usedKnobs;
}

VulkanFunctions* Device::GetMutableFunctions() {
    return const_cast<VulkanFunctions*>(&fn);
}

CommandRecordingContext* Device::GetPendingRecordingContext(Device::SubmitMode submitMode) {
    return ToBackend(GetQueue())->GetPendingRecordingContext(submitMode);
}

MaybeError Device::SubmitPendingCommands() {
    return ToBackend(GetQueue())->SubmitPendingCommands();
}

MaybeError Device::CopyFromStagingToBufferImpl(BufferBase* source,
                                               uint64_t sourceOffset,
                                               BufferBase* destination,
                                               uint64_t destinationOffset,
                                               uint64_t size) {
    // It is a validation error to do a 0-sized copy in Vulkan, check it is skipped prior to
    // calling this function.
    DAWN_ASSERT(size != 0);

    CommandRecordingContext* recordingContext =
        GetPendingRecordingContext(DeviceBase::SubmitMode::Passive);

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

    this->fn.CmdCopyBuffer(recordingContext->commandBuffer, ToBackend(source)->GetHandle(),
                           ToBackend(destination)->GetHandle(), 1, &copy);

    return {};
}

MaybeError Device::CopyFromStagingToTextureImpl(const BufferBase* source,
                                                const TextureDataLayout& src,
                                                const TextureCopy& dst,
                                                const Extent3D& copySizePixels) {
    // There is no need of a barrier to make host writes available and visible to the copy
    // operation for HOST_COHERENT memory. The Vulkan spec for vkQueueSubmit describes that it
    // does an implicit availability, visibility and domain operation.

    CommandRecordingContext* recordingContext =
        GetPendingRecordingContext(DeviceBase::SubmitMode::Passive);

    VkBufferImageCopy region = ComputeBufferImageCopyRegion(src, dst, copySizePixels);
    VkImageSubresourceLayers subresource = region.imageSubresource;

    SubresourceRange range = GetSubresourcesAffectedByCopy(dst, copySizePixels);

    if (IsCompleteSubresourceCopiedTo(dst.texture.Get(), copySizePixels, subresource.mipLevel,
                                      dst.aspect)) {
        // Since texture has been overwritten, it has been "initialized"
        dst.texture->SetIsSubresourceContentInitialized(true, range);
    } else {
        DAWN_TRY(
            ToBackend(dst.texture)->EnsureSubresourceContentInitialized(recordingContext, range));
    }
    // Insert pipeline barrier to ensure correct ordering with previous memory operations on the
    // texture.
    ToBackend(dst.texture)
        ->TransitionUsageNow(recordingContext, wgpu::TextureUsage::CopyDst, range);
    VkImage dstImage = ToBackend(dst.texture)->GetHandle();

    // Dawn guarantees dstImage be in the TRANSFER_DST_OPTIMAL layout after the
    // copy command.
    this->fn.CmdCopyBufferToImage(recordingContext->commandBuffer, ToBackend(source)->GetHandle(),
                                  dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    return {};
}

MaybeError Device::ImportExternalImage(const ExternalImageDescriptorVk* descriptor,
                                       ExternalMemoryHandle memoryHandle,
                                       VkImage image,
                                       const std::vector<ExternalSemaphoreHandle>& waitHandles,
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
                        descriptor->GetType(), VulkanImageFormat(this, textureDescriptor->format),
                        VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                        VulkanImageUsage(usage, GetValidInternalFormat(textureDescriptor->format)),
                        VK_IMAGE_CREATE_ALIAS_BIT_KHR),
                    "External memory usage not supported");

    // Import the external image's memory
    external_memory::MemoryImportParams importParams;
    DAWN_TRY_ASSIGN(importParams, mExternalMemoryService->GetMemoryImportParams(descriptor, image));
    DAWN_TRY_ASSIGN(*outAllocation, mExternalMemoryService->ImportMemory(
                                        descriptor->GetType(), memoryHandle, importParams, image));

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

        ExternalSemaphoreHandle semaphoreHandle;
        VkImageLayout releasedOldLayout;
        VkImageLayout releasedNewLayout;
        DAWN_TRY(texture->ExportExternalTexture(desiredLayout, &semaphoreHandle, &releasedOldLayout,
                                                &releasedNewLayout));

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
    if (ConsumedError(ValidateIsAlive())) {
        return nullptr;
    }
    if (ConsumedError(ValidateTextureDescriptor(this, textureDescriptor,
                                                AllowMultiPlanarTextureFormat::Yes))) {
        return nullptr;
    }
    if (ConsumedError(ValidateVulkanImageCanBeWrapped(this, textureDescriptor),
                      "validating that a Vulkan image can be wrapped with %s.",
                      textureDescriptor)) {
        return nullptr;
    }
    if (GetValidInternalFormat(textureDescriptor->format).IsMultiPlanar() &&
        !descriptor->isInitialized) {
        bool consumed = ConsumedError(DAWN_VALIDATION_ERROR(
            "External textures with multiplanar formats must be initialized."));
        DAWN_UNUSED(consumed);
        return nullptr;
    }

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
                                          waitHandles, &allocation, &waitSemaphores)) ||
        ConsumedError(result->BindExternalMemory(descriptor, allocation, waitSemaphores))) {
        // Delete the Texture if it was created
        if (result != nullptr) {
            result->Release();
        }

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
    return ToBackend(GetPhysicalDevice())->GetDefaultComputeSubgroupSize();
}

void Device::OnDebugMessage(std::string message) {
    mDebugMessages.push_back(std::move(message));
}

MaybeError Device::CheckDebugLayerAndGenerateErrors() {
    if (!GetPhysicalDevice()->GetInstance()->IsBackendValidationEnabled() ||
        mDebugMessages.empty()) {
        return {};
    }

    auto error = DAWN_INTERNAL_ERROR("The Vulkan validation layer reported uncaught errors.");

    AppendDebugLayerMessages(error.get());

    return std::move(error);
}

void Device::AppendDebugLayerMessages(ErrorData* error) {
    if (!GetPhysicalDevice()->GetInstance()->IsBackendValidationEnabled()) {
        return;
    }

    while (!mDebugMessages.empty()) {
        error->AppendBackendMessage(std::move(mDebugMessages.back()));
        mDebugMessages.pop_back();
    }
}

void Device::CheckDebugMessagesAfterDestruction() const {
    if (!GetPhysicalDevice()->GetInstance()->IsBackendValidationEnabled() ||
        mDebugMessages.empty()) {
        return;
    }

    dawn::ErrorLog()
        << "Some VVL messages were not handled before dawn::native::vulkan::Device destruction:";
    for (const auto& message : mDebugMessages) {
        dawn::ErrorLog() << " - " << message;
    }

    // Crash in debug
    DAWN_ASSERT(false);
}

void Device::DestroyImpl() {
    DAWN_ASSERT(GetState() == State::Disconnected);

    // We failed during initialization so early that we don't even have a VkDevice. There is
    // nothing to do.
    if (mVkDevice == VK_NULL_HANDLE) {
        return;
    }

    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the device is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the device.
    // - It may be called when the last ref to the device is dropped and the device
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the device since there are no other live refs.

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

    ToBackend(GetPhysicalDevice())->GetVulkanInstance()->StopListeningForDeviceMessages(this);

    if (GetQueue() != nullptr) {
        GetQueue()->Destroy();
    }

    for (Ref<DescriptorSetAllocator>& allocator :
         mDescriptorAllocatorsPendingDeallocation.IterateUpTo(kMaxExecutionSerial)) {
        allocator->FinishDeallocation(kMaxExecutionSerial);
    }

    // Releasing the uploader enqueues buffers to be released.
    // Call Tick() again to clear them before releasing the deleter.
    GetResourceMemoryAllocator()->Tick(kMaxExecutionSerial);
    mDescriptorAllocatorsPendingDeallocation.ClearUpTo(kMaxExecutionSerial);

    // Allow recycled memory to be deleted.
    GetResourceMemoryAllocator()->DestroyPool();

    // The VkRenderPasses in the cache can be destroyed immediately since all commands referring
    // to them are guaranteed to be finished executing.
    mRenderPassCache = nullptr;

    // Delete all the remaining VkDevice child objects immediately since the GPU timeline is
    // finished.
    GetFencedDeleter()->Tick(kMaxExecutionSerial);
    mDeleter = nullptr;

    // VkQueues are destroyed when the VkDevice is destroyed
    // The VkDevice is needed to destroy child objects, so it must be destroyed last after all
    // child objects have been deleted.
    DAWN_ASSERT(mVkDevice != VK_NULL_HANDLE);
    fn.DestroyDevice(mVkDevice, nullptr);
    mVkDevice = VK_NULL_HANDLE;

    // No additonal Vulkan commands should be done by this device after this function. Check for any
    // remaining Vulkan Validation Layer messages that may have been added during destruction or not
    // handled prior to destruction.
    CheckDebugMessagesAfterDestruction();
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
