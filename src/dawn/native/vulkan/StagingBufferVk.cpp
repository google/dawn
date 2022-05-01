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

#include "dawn/native/vulkan/StagingBufferVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/ResourceHeapVk.h"
#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

StagingBuffer::StagingBuffer(size_t size, Device* device)
    : StagingBufferBase(size), mDevice(device) {}

MaybeError StagingBuffer::Initialize() {
    VkBufferCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.size = GetSize();
    createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = 0;

    DAWN_TRY(CheckVkSuccess(
        mDevice->fn.CreateBuffer(mDevice->GetVkDevice(), &createInfo, nullptr, &*mBuffer),
        "vkCreateBuffer"));

    VkMemoryRequirements requirements;
    mDevice->fn.GetBufferMemoryRequirements(mDevice->GetVkDevice(), mBuffer, &requirements);

    DAWN_TRY_ASSIGN(mAllocation, mDevice->GetResourceMemoryAllocator()->Allocate(
                                     requirements, MemoryKind::LinearMappable));

    DAWN_TRY(CheckVkSuccess(
        mDevice->fn.BindBufferMemory(mDevice->GetVkDevice(), mBuffer,
                                     ToBackend(mAllocation.GetResourceHeap())->GetMemory(),
                                     mAllocation.GetOffset()),
        "vkBindBufferMemory"));

    mMappedPointer = mAllocation.GetMappedPointer();
    if (mMappedPointer == nullptr) {
        return DAWN_INTERNAL_ERROR("Unable to map staging buffer.");
    }

    SetDebugName(mDevice, mBuffer, "Dawn_StagingBuffer");

    return {};
}

StagingBuffer::~StagingBuffer() {
    mMappedPointer = nullptr;
    mDevice->GetFencedDeleter()->DeleteWhenUnused(mBuffer);
    mDevice->GetResourceMemoryAllocator()->Deallocate(&mAllocation);
}

VkBuffer StagingBuffer::GetBufferHandle() const {
    return mBuffer;
}

}  // namespace dawn::native::vulkan
