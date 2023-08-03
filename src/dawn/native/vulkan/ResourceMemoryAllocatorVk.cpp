// Copyright 2019 The Dawn Authors
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

#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"

#include <algorithm>
#include <utility>

#include "dawn/common/Math.h"
#include "dawn/native/BuddyMemoryAllocator.h"
#include "dawn/native/ResourceHeapAllocator.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/ResourceHeapVk.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

namespace {

// TODO(crbug.com/dawn/849): This is a hardcoded heurstic to choose when to
// suballocate but it should ideally depend on the size of the memory heaps and other
// factors.
constexpr uint64_t kMaxSizeForSubAllocation = 4ull * 1024ull * 1024ull;  // 4MiB

// Have each bucket of the buddy system allocate at least some resource of the maximum
// size
constexpr uint64_t kBuddyHeapsSize = 2 * kMaxSizeForSubAllocation;

bool IsMemoryKindMappable(MemoryKind memoryKind) {
    switch (memoryKind) {
        case MemoryKind::LinearReadMappable:
        case MemoryKind::LinearWriteMappable:
            return true;

        case MemoryKind::LazilyAllocated:
        case MemoryKind::Linear:
        case MemoryKind::Opaque:
            return false;

        default:
            UNREACHABLE();
    }
}

}  // anonymous namespace

// SingleTypeAllocator is a combination of a BuddyMemoryAllocator and its client and can
// service suballocation requests, but for a single Vulkan memory type.

class ResourceMemoryAllocator::SingleTypeAllocator : public ResourceHeapAllocator {
  public:
    SingleTypeAllocator(Device* device, size_t memoryTypeIndex, VkDeviceSize memoryHeapSize)
        : mDevice(device),
          mMemoryTypeIndex(memoryTypeIndex),
          mMemoryHeapSize(memoryHeapSize),
          mPooledMemoryAllocator(this),
          mBuddySystem(
              // Round down to a power of 2 that's <= mMemoryHeapSize. This will always
              // be a multiple of kBuddyHeapsSize because kBuddyHeapsSize is a power of 2.
              uint64_t(1) << Log2(mMemoryHeapSize),
              // Take the min in the very unlikely case the memory heap is tiny.
              std::min(uint64_t(1) << Log2(mMemoryHeapSize), kBuddyHeapsSize),
              &mPooledMemoryAllocator) {
        ASSERT(IsPowerOfTwo(kBuddyHeapsSize));
    }
    ~SingleTypeAllocator() override = default;

    void DestroyPool() { mPooledMemoryAllocator.DestroyPool(); }

    ResultOrError<ResourceMemoryAllocation> AllocateMemory(uint64_t size, uint64_t alignment) {
        return mBuddySystem.Allocate(size, alignment);
    }

    void DeallocateMemory(const ResourceMemoryAllocation& allocation) {
        mBuddySystem.Deallocate(allocation);
    }

    // Implementation of the MemoryAllocator interface to be a client of BuddyMemoryAllocator

    ResultOrError<std::unique_ptr<ResourceHeapBase>> AllocateResourceHeap(uint64_t size) override {
        if (size > mMemoryHeapSize) {
            return DAWN_OUT_OF_MEMORY_ERROR("Allocation size too large");
        }

        VkMemoryAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.allocationSize = size;
        allocateInfo.memoryTypeIndex = mMemoryTypeIndex;

        VkDeviceMemory allocatedMemory = VK_NULL_HANDLE;

        // First check OOM that we want to surface to the application.
        DAWN_TRY(
            CheckVkOOMThenSuccess(mDevice->fn.AllocateMemory(mDevice->GetVkDevice(), &allocateInfo,
                                                             nullptr, &*allocatedMemory),
                                  "vkAllocateMemory"));

        ASSERT(allocatedMemory != VK_NULL_HANDLE);
        return {std::make_unique<ResourceHeap>(allocatedMemory, mMemoryTypeIndex)};
    }

    void DeallocateResourceHeap(std::unique_ptr<ResourceHeapBase> allocation) override {
        mDevice->GetFencedDeleter()->DeleteWhenUnused(ToBackend(allocation.get())->GetMemory());
    }

  private:
    Device* mDevice;
    size_t mMemoryTypeIndex;
    VkDeviceSize mMemoryHeapSize;
    PooledResourceMemoryAllocator mPooledMemoryAllocator;
    BuddyMemoryAllocator mBuddySystem;
};

// Implementation of ResourceMemoryAllocator

ResourceMemoryAllocator::ResourceMemoryAllocator(Device* device) : mDevice(device) {
    const VulkanDeviceInfo& info = mDevice->GetDeviceInfo();
    mAllocatorsPerType.reserve(info.memoryTypes.size());

    for (size_t i = 0; i < info.memoryTypes.size(); i++) {
        mAllocatorsPerType.emplace_back(std::make_unique<SingleTypeAllocator>(
            mDevice, i, info.memoryHeaps[info.memoryTypes[i].heapIndex].size));
    }
}

ResourceMemoryAllocator::~ResourceMemoryAllocator() = default;

ResultOrError<ResourceMemoryAllocation> ResourceMemoryAllocator::Allocate(
    const VkMemoryRequirements& requirements,
    MemoryKind kind,
    bool forceDisableSubAllocation) {
    // The Vulkan spec guarantees at least on memory type is valid.
    int memoryType = FindBestTypeIndex(requirements, kind);
    ASSERT(memoryType >= 0);

    VkDeviceSize size = requirements.size;

    // Sub-allocate non-mappable resources because at the moment the mapped pointer
    // is part of the resource and not the heap, which doesn't match the Vulkan model.
    // TODO(crbug.com/dawn/849): allow sub-allocating mappable resources, maybe.
    if (!forceDisableSubAllocation && requirements.size < kMaxSizeForSubAllocation &&
        !IsMemoryKindMappable(kind) &&
        !mDevice->IsToggleEnabled(Toggle::DisableResourceSuballocation)) {
        // When sub-allocating, Vulkan requires that we respect bufferImageGranularity. Some
        // hardware puts information on the memory's page table entry and allocating a linear
        // resource in the same page as a non-linear (aka opaque) resource can cause issues.
        // Probably because some texture compression flags are stored on the page table entry,
        // and allocating a linear resource removes these flags.
        //
        // Anyway, just to be safe we ask that all sub-allocated resources are allocated with at
        // least this alignment. TODO(crbug.com/dawn/849): this is suboptimal because multiple
        // linear (resp. opaque) resources can coexist in the same page. In particular Nvidia
        // GPUs often use a granularity of 64k which will lead to a lot of wasted spec. Revisit
        // with a more efficient algorithm later.
        uint64_t alignment =
            std::max(requirements.alignment,
                     mDevice->GetDeviceInfo().properties.limits.bufferImageGranularity);

        ResourceMemoryAllocation subAllocation;
        DAWN_TRY_ASSIGN(subAllocation, mAllocatorsPerType[memoryType]->AllocateMemory(
                                           requirements.size, alignment));
        if (subAllocation.GetInfo().mMethod != AllocationMethod::kInvalid) {
            return std::move(subAllocation);
        }
    }

    // If sub-allocation failed, allocate memory just for it.
    std::unique_ptr<ResourceHeapBase> resourceHeap;
    DAWN_TRY_ASSIGN(resourceHeap, mAllocatorsPerType[memoryType]->AllocateResourceHeap(size));

    void* mappedPointer = nullptr;
    if (IsMemoryKindMappable(kind)) {
        DAWN_TRY_WITH_CLEANUP(
            CheckVkSuccess(mDevice->fn.MapMemory(mDevice->GetVkDevice(),
                                                 ToBackend(resourceHeap.get())->GetMemory(), 0,
                                                 size, 0, &mappedPointer),
                           "vkMapMemory"),
            { mAllocatorsPerType[memoryType]->DeallocateResourceHeap(std::move(resourceHeap)); });
    }

    AllocationInfo info;
    info.mMethod = AllocationMethod::kDirect;
    return ResourceMemoryAllocation(info, /*offset*/ 0, resourceHeap.release(),
                                    static_cast<uint8_t*>(mappedPointer));
}

void ResourceMemoryAllocator::Deallocate(ResourceMemoryAllocation* allocation) {
    switch (allocation->GetInfo().mMethod) {
        // Some memory allocation can never be initialized, for example when wrapping
        // swapchain VkImages with a Texture.
        case AllocationMethod::kInvalid:
            break;

        // For direct allocation we can put the memory for deletion immediately and the fence
        // deleter will make sure the resources are freed before the memory.
        case AllocationMethod::kDirect: {
            ResourceHeap* heap = ToBackend(allocation->GetResourceHeap());
            allocation->Invalidate();
            mDevice->GetFencedDeleter()->DeleteWhenUnused(heap->GetMemory());
            delete heap;
            break;
        }

        // Suballocations aren't freed immediately, otherwise another resource allocation could
        // happen just after that aliases the old one and would require a barrier.
        // TODO(crbug.com/dawn/851): Maybe we can produce the correct barriers to reduce the
        // latency to reclaim memory.
        case AllocationMethod::kSubAllocated:
            mSubAllocationsToDelete.Enqueue(*allocation, mDevice->GetPendingCommandSerial());
            break;

        default:
            UNREACHABLE();
            break;
    }

    // Invalidate the underlying resource heap in case the client accidentally
    // calls DeallocateMemory again using the same allocation.
    allocation->Invalidate();
}

void ResourceMemoryAllocator::Tick(ExecutionSerial completedSerial) {
    for (const ResourceMemoryAllocation& allocation :
         mSubAllocationsToDelete.IterateUpTo(completedSerial)) {
        ASSERT(allocation.GetInfo().mMethod == AllocationMethod::kSubAllocated);
        size_t memoryType = ToBackend(allocation.GetResourceHeap())->GetMemoryType();

        mAllocatorsPerType[memoryType]->DeallocateMemory(allocation);
    }

    mSubAllocationsToDelete.ClearUpTo(completedSerial);
}

int ResourceMemoryAllocator::FindBestTypeIndex(VkMemoryRequirements requirements, MemoryKind kind) {
    const VulkanDeviceInfo& info = mDevice->GetDeviceInfo();
    bool mappable = IsMemoryKindMappable(kind);

    // Find a suitable memory type for this allocation
    int bestType = -1;
    for (size_t i = 0; i < info.memoryTypes.size(); ++i) {
        // Resource must support this memory type
        if ((requirements.memoryTypeBits & (1 << i)) == 0) {
            continue;
        }

        // Mappable resource must be host visible
        if (mappable &&
            (info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
            continue;
        }

        // Mappable must also be host coherent.
        if (mappable &&
            (info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            continue;
        }

        // Found the first candidate memory type
        if (bestType == -1) {
            bestType = static_cast<int>(i);
            continue;
        }

        // For non-mappable resources that can be lazily allocated, favor lazy
        // allocation (note: this is a more important property than that of
        // device local memory and hence is checked first).
        bool currentLazilyAllocated =
            info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
        bool bestLazilyAllocated =
            info.memoryTypes[bestType].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
        if ((kind == MemoryKind::LazilyAllocated) &&
            (currentLazilyAllocated != bestLazilyAllocated)) {
            if (currentLazilyAllocated) {
                bestType = static_cast<int>(i);
            }
            continue;
        }

        // For non-mappable, non-lazily-allocated resources, favor device local
        // memory.
        bool currentDeviceLocal =
            info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        bool bestDeviceLocal =
            info.memoryTypes[bestType].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        if (!mappable && (currentDeviceLocal != bestDeviceLocal)) {
            if (currentDeviceLocal) {
                bestType = static_cast<int>(i);
            }
            continue;
        }

        // Cached memory is optimal for read-only access from CPU as host memory accesses to
        // uncached memory are slower than to cached memory.
        bool currentHostCached =
            info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        bool bestHostCached =
            info.memoryTypes[bestType].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        if (kind == MemoryKind::LinearReadMappable && currentHostCached != bestHostCached) {
            if (currentHostCached) {
                bestType = static_cast<int>(i);
            }
            continue;
        }

        // All things equal favor the memory in the biggest heap
        VkDeviceSize bestTypeHeapSize = info.memoryHeaps[info.memoryTypes[bestType].heapIndex].size;
        VkDeviceSize candidateHeapSize = info.memoryHeaps[info.memoryTypes[i].heapIndex].size;
        if (candidateHeapSize > bestTypeHeapSize) {
            bestType = static_cast<int>(i);
            continue;
        }
    }

    return bestType;
}

void ResourceMemoryAllocator::DestroyPool() {
    for (auto& alloc : mAllocatorsPerType) {
        alloc->DestroyPool();
    }
}

}  // namespace dawn::native::vulkan
