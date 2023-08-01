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

#ifndef SRC_DAWN_NATIVE_VULKAN_DEVICEVK_H_
#define SRC_DAWN_NATIVE_VULKAN_DEVICEVK_H_

#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "dawn/common/SerialQueue.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/native/vulkan/CommandRecordingContext.h"
#include "dawn/native/vulkan/DescriptorSetAllocator.h"
#include "dawn/native/vulkan/Forward.h"
#include "dawn/native/vulkan/VulkanFunctions.h"
#include "dawn/native/vulkan/VulkanInfo.h"

#include "dawn/native/vulkan/external_memory/MemoryService.h"
#include "dawn/native/vulkan/external_semaphore/SemaphoreService.h"

namespace dawn::native::vulkan {

class BufferUploader;
class FencedDeleter;
class RenderPassCache;
class ResourceMemoryAllocator;

class Device final : public DeviceBase {
  public:
    static ResultOrError<Ref<Device>> Create(AdapterBase* adapter,
                                             const DeviceDescriptor* descriptor,
                                             const TogglesState& deviceToggles);
    ~Device() override;

    MaybeError Initialize(const DeviceDescriptor* descriptor);

    // Contains all the Vulkan entry points, vkDoFoo is called via device->fn.DoFoo.
    const VulkanFunctions fn;

    VkInstance GetVkInstance() const;
    const VulkanDeviceInfo& GetDeviceInfo() const;
    const VulkanGlobalInfo& GetGlobalInfo() const;
    VkDevice GetVkDevice() const;
    uint32_t GetGraphicsQueueFamily() const;
    VkQueue GetQueue() const;

    FencedDeleter* GetFencedDeleter() const;
    RenderPassCache* GetRenderPassCache() const;
    ResourceMemoryAllocator* GetResourceMemoryAllocator() const;
    external_semaphore::Service* GetExternalSemaphoreService() const;

    CommandRecordingContext* GetPendingRecordingContext(
        Device::SubmitMode submitMode = Device::SubmitMode::Normal);
    MaybeError SplitRecordingContext(CommandRecordingContext* recordingContext);
    MaybeError SubmitPendingCommands();

    void EnqueueDeferredDeallocation(DescriptorSetAllocator* allocator);

    // Dawn Native API

    TextureBase* CreateTextureWrappingVulkanImage(
        const ExternalImageDescriptorVk* descriptor,
        ExternalMemoryHandle memoryHandle,
        const std::vector<ExternalSemaphoreHandle>& waitHandles);
    bool SignalAndExportExternalTexture(Texture* texture,
                                        VkImageLayout desiredLayout,
                                        ExternalImageExportInfoVk* info,
                                        std::vector<ExternalSemaphoreHandle>* semaphoreHandle);

    ResultOrError<Ref<CommandBufferBase>> CreateCommandBuffer(
        CommandEncoder* encoder,
        const CommandBufferDescriptor* descriptor) override;

    MaybeError TickImpl() override;

    MaybeError CopyFromStagingToBufferImpl(BufferBase* source,
                                           uint64_t sourceOffset,
                                           BufferBase* destination,
                                           uint64_t destinationOffset,
                                           uint64_t size) override;
    MaybeError CopyFromStagingToTextureImpl(const BufferBase* source,
                                            const TextureDataLayout& src,
                                            const TextureCopy& dst,
                                            const Extent3D& copySizePixels) override;

    // Return the fixed subgroup size to use for compute shaders on this device or 0 if none
    // needs to be set.
    uint32_t GetComputeSubgroupSize() const;

    uint32_t GetOptimalBytesPerRowAlignment() const override;
    uint64_t GetOptimalBufferToTextureCopyOffsetAlignment() const override;

    float GetTimestampPeriodInNS() const override;

    void SetLabelImpl() override;

    void OnDebugMessage(std::string message);

    // Used to associate this device with validation layer messages.
    const char* GetDebugPrefix() { return mDebugPrefix.c_str(); }

    void ForceEventualFlushOfCommands() override;

  private:
    Device(AdapterBase* adapter,
           const DeviceDescriptor* descriptor,
           const TogglesState& deviceToggles);

    ResultOrError<Ref<BindGroupBase>> CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) override;
    ResultOrError<Ref<BindGroupLayoutInternalBase>> CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) override;
    ResultOrError<Ref<BufferBase>> CreateBufferImpl(const BufferDescriptor* descriptor) override;
    ResultOrError<Ref<PipelineLayoutBase>> CreatePipelineLayoutImpl(
        const PipelineLayoutDescriptor* descriptor) override;
    ResultOrError<Ref<QuerySetBase>> CreateQuerySetImpl(
        const QuerySetDescriptor* descriptor) override;
    ResultOrError<Ref<SamplerBase>> CreateSamplerImpl(const SamplerDescriptor* descriptor) override;
    ResultOrError<Ref<ShaderModuleBase>> CreateShaderModuleImpl(
        const ShaderModuleDescriptor* descriptor,
        ShaderModuleParseResult* parseResult,
        OwnedCompilationMessages* compilationMessages) override;
    ResultOrError<Ref<SwapChainBase>> CreateSwapChainImpl(
        Surface* surface,
        SwapChainBase* previousSwapChain,
        const SwapChainDescriptor* descriptor) override;
    ResultOrError<Ref<TextureBase>> CreateTextureImpl(const TextureDescriptor* descriptor) override;
    ResultOrError<Ref<TextureViewBase>> CreateTextureViewImpl(
        TextureBase* texture,
        const TextureViewDescriptor* descriptor) override;
    Ref<ComputePipelineBase> CreateUninitializedComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) override;
    Ref<RenderPipelineBase> CreateUninitializedRenderPipelineImpl(
        const RenderPipelineDescriptor* descriptor) override;
    Ref<PipelineCacheBase> GetOrCreatePipelineCacheImpl(const CacheKey& key) override;
    void InitializeComputePipelineAsyncImpl(Ref<ComputePipelineBase> computePipeline,
                                            WGPUCreateComputePipelineAsyncCallback callback,
                                            void* userdata) override;
    void InitializeRenderPipelineAsyncImpl(Ref<RenderPipelineBase> renderPipeline,
                                           WGPUCreateRenderPipelineAsyncCallback callback,
                                           void* userdata) override;

    ResultOrError<wgpu::TextureUsage> GetSupportedSurfaceUsageImpl(
        const Surface* surface) const override;

    ResultOrError<VulkanDeviceKnobs> CreateDevice(VkPhysicalDevice vkPhysicalDevice);
    void GatherQueueFromDevice();

    MaybeError CheckDebugLayerAndGenerateErrors();
    void AppendDebugLayerMessages(ErrorData* error) override;
    void CheckDebugMessagesAfterDestruction() const;

    void DestroyImpl() override;
    MaybeError WaitForIdleForDestruction() override;
    bool HasPendingCommands() const override;

    // To make it easier to use fn it is a public const member. However
    // the Device is allowed to mutate them through these private methods.
    VulkanFunctions* GetMutableFunctions();

    VulkanDeviceInfo mDeviceInfo = {};
    VkDevice mVkDevice = VK_NULL_HANDLE;
    uint32_t mQueueFamily = 0;
    VkQueue mQueue = VK_NULL_HANDLE;

    SerialQueue<ExecutionSerial, Ref<DescriptorSetAllocator>>
        mDescriptorAllocatorsPendingDeallocation;
    std::unique_ptr<FencedDeleter> mDeleter;
    std::unique_ptr<ResourceMemoryAllocator> mResourceMemoryAllocator;
    std::unique_ptr<RenderPassCache> mRenderPassCache;

    std::unique_ptr<external_memory::Service> mExternalMemoryService;
    std::unique_ptr<external_semaphore::Service> mExternalSemaphoreService;

    ResultOrError<VkFence> GetUnusedFence();
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override;

    // We track which operations are in flight on the GPU with an increasing serial.
    // This works only because we have a single queue. Each submit to a queue is associated
    // to a serial and a fence, such that when the fence is "ready" we know the operations
    // have finished.
    std::queue<std::pair<VkFence, ExecutionSerial>> mFencesInFlight;
    // Fences in the unused list aren't reset yet.
    std::vector<VkFence> mUnusedFences;

    // For capturing messages generated by the Vulkan debug layer.
    const std::string mDebugPrefix;
    std::vector<std::string> mDebugMessages;

    MaybeError PrepareRecordingContext();
    ResultOrError<CommandPoolAndBuffer> BeginVkCommandBuffer();
    void RecycleCompletedCommands();

    SerialQueue<ExecutionSerial, CommandPoolAndBuffer> mCommandsInFlight;
    // Command pools in the unused list haven't been reset yet.
    std::vector<CommandPoolAndBuffer> mUnusedCommands;
    // There is always a valid recording context stored in mRecordingContext
    CommandRecordingContext mRecordingContext;

    MaybeError ImportExternalImage(const ExternalImageDescriptorVk* descriptor,
                                   ExternalMemoryHandle memoryHandle,
                                   VkImage image,
                                   const std::vector<ExternalSemaphoreHandle>& waitHandles,
                                   VkDeviceMemory* outAllocation,
                                   std::vector<VkSemaphore>* outWaitSemaphores);
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_DEVICEVK_H_
