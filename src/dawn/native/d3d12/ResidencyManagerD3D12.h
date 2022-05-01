// Copyright 2020 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D12_RESIDENCYMANAGERD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_RESIDENCYMANAGERD3D12_H_

#include "dawn/common/LinkedList.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/Error.h"
#include "dawn/native/dawn_platform.h"

#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class Device;
class Heap;
class Pageable;

class ResidencyManager {
  public:
    explicit ResidencyManager(Device* device);

    MaybeError LockAllocation(Pageable* pageable);
    void UnlockAllocation(Pageable* pageable);

    MaybeError EnsureCanAllocate(uint64_t allocationSize, MemorySegment memorySegment);
    MaybeError EnsureHeapsAreResident(Heap** heaps, size_t heapCount);

    uint64_t SetExternalMemoryReservation(MemorySegment segment, uint64_t requestedReservationSize);

    void TrackResidentAllocation(Pageable* pageable);

    void RestrictBudgetForTesting(uint64_t artificialBudgetCap);

  private:
    struct MemorySegmentInfo {
        const DXGI_MEMORY_SEGMENT_GROUP dxgiSegment;
        LinkedList<Pageable> lruCache = {};
        uint64_t budget = 0;
        uint64_t usage = 0;
        uint64_t externalReservation = 0;
        uint64_t externalRequest = 0;
    };

    struct VideoMemoryInfo {
        MemorySegmentInfo local = {DXGI_MEMORY_SEGMENT_GROUP_LOCAL};
        MemorySegmentInfo nonLocal = {DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL};
    };

    MemorySegmentInfo* GetMemorySegmentInfo(MemorySegment memorySegment);
    ResultOrError<uint64_t> EnsureCanMakeResident(uint64_t allocationSize,
                                                  MemorySegmentInfo* memorySegment);
    ResultOrError<Pageable*> RemoveSingleEntryFromLRU(MemorySegmentInfo* memorySegment);
    MaybeError MakeAllocationsResident(MemorySegmentInfo* segment,
                                       uint64_t sizeToMakeResident,
                                       uint64_t numberOfObjectsToMakeResident,
                                       ID3D12Pageable** allocations);
    void UpdateVideoMemoryInfo();
    void UpdateMemorySegmentInfo(MemorySegmentInfo* segmentInfo);

    Device* mDevice;
    bool mResidencyManagementEnabled = false;
    bool mRestrictBudgetForTesting = false;
    VideoMemoryInfo mVideoMemoryInfo = {};
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_RESIDENCYMANAGERD3D12_H_
