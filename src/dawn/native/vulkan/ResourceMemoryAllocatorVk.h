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

#ifndef SRC_DAWN_NATIVE_VULKAN_RESOURCEMEMORYALLOCATORVK_H_
#define SRC_DAWN_NATIVE_VULKAN_RESOURCEMEMORYALLOCATORVK_H_

#include <memory>
#include <vector>

#include "dawn/common/SerialQueue.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/PooledResourceMemoryAllocator.h"
#include "dawn/native/ResourceMemoryAllocation.h"

namespace dawn::native::vulkan {

class Device;

// Various kinds of memory that influence the result of the allocation. For example, to take
// into account mappability and Vulkan's bufferImageGranularity.
enum class MemoryKind {
    LazilyAllocated,
    Linear,
    LinearReadMappable,
    LinearWriteMappable,
    Opaque,
};

class ResourceMemoryAllocator {
  public:
    explicit ResourceMemoryAllocator(Device* device);
    ~ResourceMemoryAllocator();

    ResultOrError<ResourceMemoryAllocation> Allocate(const VkMemoryRequirements& requirements,
                                                     MemoryKind kind,
                                                     bool forceDisableSubAllocation = false);
    void Deallocate(ResourceMemoryAllocation* allocation);

    void DestroyPool();

    void Tick(ExecutionSerial completedSerial);

    int FindBestTypeIndex(VkMemoryRequirements requirements, MemoryKind kind);

  private:
    Device* mDevice;

    class SingleTypeAllocator;
    std::vector<std::unique_ptr<SingleTypeAllocator>> mAllocatorsPerType;

    SerialQueue<ExecutionSerial, ResourceMemoryAllocation> mSubAllocationsToDelete;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_RESOURCEMEMORYALLOCATORVK_H_
