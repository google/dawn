// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/native/vulkan/DescriptorSetAllocator.h"

#include <algorithm>
#include <utility>

#include "dawn/native/Queue.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

// DescriptorSetAllocator

// TODO(https://crbug.com/439522242): Consider adding some better heuristic, like an exponential
// increase up to a larger constant.
static constexpr uint32_t kMaxDescriptorsPerPool = 512;

// static
Ref<DescriptorSetAllocator> DescriptorSetAllocator::Create(
    Device* device,
    absl::flat_hash_map<VkDescriptorType, uint32_t> descriptorCountPerType) {
    return AcquireRef(new DescriptorSetAllocator(device, descriptorCountPerType));
}

DescriptorSetAllocator::DescriptorSetAllocator(
    Device* device,
    absl::flat_hash_map<VkDescriptorType, uint32_t> descriptorCountPerType)
    : mDevice(device) {
    // Compute the total number of descriptors for this layout.
    uint32_t totalDescriptorCount = 0;
    mPoolSizes.reserve(descriptorCountPerType.size());
    for (const auto& [type, count] : descriptorCountPerType) {
        DAWN_ASSERT(count > 0);
        totalDescriptorCount += count;
        mPoolSizes.push_back(VkDescriptorPoolSize{type, count});
    }

    // Always assume there is one descriptor requested to avoid a division by 0 below.
    totalDescriptorCount = std::max(1u, totalDescriptorCount);

    DAWN_ASSERT(totalDescriptorCount <= kMaxBindingsPerPipelineLayout);
    static_assert(kMaxBindingsPerPipelineLayout <= kMaxDescriptorsPerPool);

    // Compute the total number of descriptors sets that fits given the max.
    mMaxSets = kMaxDescriptorsPerPool / totalDescriptorCount;
    DAWN_ASSERT(mMaxSets > 0);

    // Grow the number of descriptors in the pool to fit the computed |mMaxSets|.
    for (auto& poolSize : mPoolSizes) {
        poolSize.descriptorCount *= mMaxSets;
    }
}

DescriptorSetAllocator::~DescriptorSetAllocator() {
    for (auto& pool : mDescriptorPools) {
        DAWN_ASSERT(pool.freeSetIndices.size() == mMaxSets);
        if (pool.vkPool != VK_NULL_HANDLE) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(pool.vkPool);
        }
    }
}

ResultOrError<DescriptorSetAllocation> DescriptorSetAllocator::Allocate(
    VkDescriptorSetLayout dsLayout) {
    Mutex::AutoLock lock(&mMutex);

    if (mAvailableDescriptorPoolIndices.empty()) {
        DAWN_TRY(AllocateDescriptorPool(dsLayout));
    }

    DAWN_ASSERT(!mAvailableDescriptorPoolIndices.empty());

    const PoolIndex poolIndex = mAvailableDescriptorPoolIndices.back();
    DescriptorPool* pool = &mDescriptorPools[poolIndex];

    DAWN_ASSERT(!pool->freeSetIndices.empty());

    SetIndex setIndex = pool->freeSetIndices.back();
    pool->freeSetIndices.pop_back();

    if (pool->freeSetIndices.empty()) {
        mAvailableDescriptorPoolIndices.pop_back();
    }

    return DescriptorSetAllocation{pool->sets[setIndex], poolIndex, setIndex};
}

void DescriptorSetAllocator::Deallocate(DescriptorSetAllocation* allocationInfo) {
    bool enqueueDeferredDeallocation = false;

    {
        Mutex::AutoLock lock(&mMutex);

        DAWN_ASSERT(allocationInfo != nullptr);
        DAWN_ASSERT(allocationInfo->set != VK_NULL_HANDLE);

        // We can't reuse the descriptor set right away because the Vulkan spec says in the
        // documentation for vkCmdBindDescriptorSets that the set may be consumed any time between
        // host execution (on "bindful" GPU that inline the descriptors in the command stream) of
        // the command and the end of the draw/dispatch (for "bindless" GPUs where shaders read
        // directly from the VkDescriptorPool).
        const ExecutionSerial serial = mDevice->GetQueue()->GetPendingCommandSerial();
        mPendingDeallocations.Enqueue({allocationInfo->poolIndex, allocationInfo->setIndex},
                                      serial);

        if (mLastDeallocationSerial != serial) {
            enqueueDeferredDeallocation = true;
            mLastDeallocationSerial = serial;
        }

        // Clear the content of the allocation so that use after frees are more visible.
        *allocationInfo = {};
    }

    if (enqueueDeferredDeallocation) {
        // Release lock before calling EnqueueDeferredDeallocation() to avoid lock acquisition
        // order inversion with lock used there.
        mDevice->EnqueueDeferredDeallocation(this);
    }
}

void DescriptorSetAllocator::FinishDeallocation(ExecutionSerial completedSerial) {
    Mutex::AutoLock lock(&mMutex);

    for (const Deallocation& dealloc : mPendingDeallocations.IterateUpTo(completedSerial)) {
        DAWN_ASSERT(dealloc.poolIndex < mDescriptorPools.size());

        auto& freeSetIndices = mDescriptorPools[dealloc.poolIndex].freeSetIndices;
        if (freeSetIndices.empty()) {
            mAvailableDescriptorPoolIndices.emplace_back(dealloc.poolIndex);
        }
        freeSetIndices.emplace_back(dealloc.setIndex);
    }
    mPendingDeallocations.ClearUpTo(completedSerial);
}

MaybeError DescriptorSetAllocator::AllocateDescriptorPool(VkDescriptorSetLayout dsLayout) {
    VkDescriptorPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.maxSets = mMaxSets;
    createInfo.poolSizeCount = mPoolSizes.size();
    createInfo.pPoolSizes = mPoolSizes.data();

    VkDescriptorPool descriptorPool;
    DAWN_TRY(CheckVkSuccess(mDevice->fn.CreateDescriptorPool(mDevice->GetVkDevice(), &createInfo,
                                                             nullptr, &*descriptorPool),
                            "CreateDescriptorPool"));

    std::vector<VkDescriptorSetLayout> layouts(mMaxSets, dsLayout);

    VkDescriptorSetAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.descriptorPool = descriptorPool;
    allocateInfo.descriptorSetCount = mMaxSets;
    allocateInfo.pSetLayouts = AsVkArray(layouts.data());

    std::vector<VkDescriptorSet> sets(mMaxSets);
    MaybeError result =
        CheckVkSuccess(mDevice->fn.AllocateDescriptorSets(mDevice->GetVkDevice(), &allocateInfo,
                                                          AsVkArray(sets.data())),
                       "AllocateDescriptorSets");
    if (result.IsError()) {
        // On an error we can destroy the pool immediately because no command references it.
        mDevice->fn.DestroyDescriptorPool(mDevice->GetVkDevice(), descriptorPool, nullptr);
        DAWN_TRY(std::move(result));
    }

    std::vector<SetIndex> freeSetIndices;
    freeSetIndices.reserve(mMaxSets);

    for (SetIndex i = 0; i < mMaxSets; ++i) {
        freeSetIndices.push_back(i);
    }

    mAvailableDescriptorPoolIndices.push_back(mDescriptorPools.size());
    mDescriptorPools.emplace_back(
        DescriptorPool{descriptorPool, std::move(sets), std::move(freeSetIndices)});

    return {};
}

// DescriptorSetAllocatorDynamicArray

// static
std::unique_ptr<DescriptorSetAllocatorDynamicArray> DescriptorSetAllocatorDynamicArray::Create(
    Device* device) {
    return std::make_unique<DescriptorSetAllocatorDynamicArray>(device);
}

DescriptorSetAllocatorDynamicArray::DescriptorSetAllocatorDynamicArray(Device* device)
    : mDevice(device) {}

DescriptorSetAllocatorDynamicArray::~DescriptorSetAllocatorDynamicArray() {
    for (VkDescriptorPool pool : mPools) {
        if (pool != VK_NULL_HANDLE) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(pool);
        }
    }
}

ResultOrError<DescriptorSetAllocation> DescriptorSetAllocatorDynamicArray::Allocate(
    VkDescriptorSetLayout dsLayout,
    const absl::flat_hash_map<VkDescriptorType, uint32_t>& descriptorCountPerType,
    VkDescriptorType dynamicVariableType,
    uint32_t dynamicVariableCount) {
    // Create the pool (for a single set), accounting for the extra bindings necessary for the
    // dynamic array.
    std::vector<VkDescriptorPoolSize> sizes;
    sizes.reserve(descriptorCountPerType.size());
    for (auto [type, count] : descriptorCountPerType) {
        if (type == dynamicVariableType) {
            count += dynamicVariableCount;
        }
        sizes.push_back(VkDescriptorPoolSize{type, count});
    }
    if (!descriptorCountPerType.contains(dynamicVariableType)) {
        // Vulkan requires that all specified pool sizes contain at least one descriptor and the
        // VVLs complain if the dynamic array (of size 0) uses a VkDescriptorType that doesn't have
        // an associated VkDescriptorPoolSize. For that reason always create a pool with at least
        // one descriptor in it.
        sizes.push_back(
            VkDescriptorPoolSize{dynamicVariableType, std::max(dynamicVariableCount, 1u)});
    }

    VkDescriptorPoolCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = 1,
        .poolSizeCount = uint32_t(sizes.size()),
        .pPoolSizes = sizes.data(),
    };

    VkDescriptorPool pool;
    DAWN_TRY(CheckVkSuccess(
        mDevice->fn.CreateDescriptorPool(mDevice->GetVkDevice(), &createInfo, nullptr, &*pool),
        "CreateDescriptorPool"));

    // Immediately track the pool to make sure it is eventually destroyed if we fail descriptor set
    // allocation.
    uint32_t slot;
    {
        Mutex::AutoLock lock(&mMutex);

        if (mAvailableSlots.empty()) {
            slot = static_cast<uint32_t>(mPools.size());
            mPools.push_back(pool);
        } else {
            slot = mAvailableSlots.back();
            mAvailableSlots.pop_back();

            DAWN_ASSERT(mPools[slot] == VK_NULL_HANDLE);
            mPools[slot] = pool;
        }
    }

    // Create the descriptor set, sized to account for the dynamic array.
    // Force the count to be at least one as some Vulkan drivers mishandle 0.
    uint32_t descriptorCount = std::max(1u, dynamicVariableCount);
    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorSetCount = 1,
        .pDescriptorCounts = &descriptorCount,
    };

    VkDescriptorSetAllocateInfo allocateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = &variableCountInfo,
        .descriptorPool = pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &*dsLayout,
    };

    VkDescriptorSet set;
    DAWN_TRY(CheckVkSuccess(
        mDevice->fn.AllocateDescriptorSets(mDevice->GetVkDevice(), &allocateInfo, &*set),
        "AllocateDescriptorSets"));

    return DescriptorSetAllocation{set, slot, 0};
}

void DescriptorSetAllocatorDynamicArray::Deallocate(DescriptorSetAllocation* allocationInfo) {
    Mutex::AutoLock lock(&mMutex);

    DAWN_ASSERT(allocationInfo->setIndex == 0);
    DAWN_ASSERT(allocationInfo->poolIndex < mPools.size());
    DAWN_ASSERT(mPools[allocationInfo->poolIndex] != VK_NULL_HANDLE);
    uint32_t slot = allocationInfo->poolIndex;

    mDevice->GetFencedDeleter()->DeleteWhenUnused(mPools[slot]);
    mPools[slot] = VK_NULL_HANDLE;
    mAvailableSlots.push_back(slot);

    // Clear the content of the allocation so that use after frees are more visible.
    *allocationInfo = {};
}

}  // namespace dawn::native::vulkan
