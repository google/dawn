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

#ifndef SRC_DAWN_NATIVE_VULKAN_BUFFERVK_H_
#define SRC_DAWN_NATIVE_VULKAN_BUFFERVK_H_

#include <set>

#include "dawn/native/Buffer.h"

#include "dawn/common/SerialQueue.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/ResourceMemoryAllocation.h"

namespace dawn::native::vulkan {

struct CommandRecordingContext;
class Device;
struct VulkanFunctions;

class Buffer final : public BufferBase {
  public:
    static ResultOrError<Ref<Buffer>> Create(Device* device, const BufferDescriptor* descriptor);

    VkBuffer GetHandle() const;

    // Transitions the buffer to be used as `usage`, recording any necessary barrier in
    // `commands`.
    // TODO(crbug.com/dawn/851): coalesce barriers and do them early when possible.
    void TransitionUsageNow(CommandRecordingContext* recordingContext, wgpu::BufferUsage usage);
    bool TrackUsageAndGetResourceBarrier(CommandRecordingContext* recordingContext,
                                         wgpu::BufferUsage usage,
                                         VkBufferMemoryBarrier* barrier,
                                         VkPipelineStageFlags* srcStages,
                                         VkPipelineStageFlags* dstStages);

    // All the Ensure methods return true if the buffer was initialized to zero.
    bool EnsureDataInitialized(CommandRecordingContext* recordingContext);
    bool EnsureDataInitializedAsDestination(CommandRecordingContext* recordingContext,
                                            uint64_t offset,
                                            uint64_t size);
    bool EnsureDataInitializedAsDestination(CommandRecordingContext* recordingContext,
                                            const CopyTextureToBufferCmd* copy);

    // Dawn API
    void SetLabelImpl() override;

    static void TransitionMappableBuffersEagerly(const VulkanFunctions& fn,
                                                 CommandRecordingContext* recordingContext,
                                                 const std::set<Ref<Buffer>>& buffers);

  private:
    ~Buffer() override;
    using BufferBase::BufferBase;

    MaybeError Initialize(bool mappedAtCreation);
    void InitializeToZero(CommandRecordingContext* recordingContext);
    void ClearBuffer(CommandRecordingContext* recordingContext,
                     uint32_t clearValue,
                     uint64_t offset = 0,
                     uint64_t size = 0);

    MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override;
    void UnmapImpl() override;
    void DestroyImpl() override;
    bool IsCPUWritableAtCreation() const override;
    MaybeError MapAtCreationImpl() override;
    void* GetMappedPointer() override;

    VkBuffer mHandle = VK_NULL_HANDLE;
    ResourceMemoryAllocation mMemoryAllocation;

    wgpu::BufferUsage mLastUsage = wgpu::BufferUsage::None;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_BUFFERVK_H_
