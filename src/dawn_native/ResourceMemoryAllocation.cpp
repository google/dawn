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

#include "dawn_native/ResourceMemoryAllocation.h"
#include "common/Assert.h"

#include <limits>

namespace dawn_native {

    static constexpr uint64_t INVALID_OFFSET = std::numeric_limits<uint64_t>::max();

    ResourceMemoryAllocation::ResourceMemoryAllocation()
        : mMethod(AllocationMethod::kInvalid), mOffset(INVALID_OFFSET), mResourceHeap(nullptr) {
    }

    ResourceMemoryAllocation::ResourceMemoryAllocation(uint64_t offset,
                                                       ResourceHeapBase* resourceHeap,
                                                       AllocationMethod method)
        : mMethod(method), mOffset(offset), mResourceHeap(resourceHeap) {
    }

    ResourceHeapBase* ResourceMemoryAllocation::GetResourceHeap() const {
        ASSERT(mMethod != AllocationMethod::kInvalid);
        return mResourceHeap;
    }

    uint64_t ResourceMemoryAllocation::GetOffset() const {
        ASSERT(mMethod != AllocationMethod::kInvalid);
        return mOffset;
    }

    AllocationMethod ResourceMemoryAllocation::GetAllocationMethod() const {
        ASSERT(mMethod != AllocationMethod::kInvalid);
        return mMethod;
    }

    void ResourceMemoryAllocation::Invalidate() {
        mResourceHeap = nullptr;
        mMethod = AllocationMethod::kInvalid;
    }
}  // namespace dawn_native