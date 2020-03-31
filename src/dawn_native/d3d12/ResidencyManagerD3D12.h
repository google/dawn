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

#ifndef DAWNNATIVE_D3D12_RESIDENCYMANAGERD3D12_H_
#define DAWNNATIVE_D3D12_RESIDENCYMANAGERD3D12_H_

#include "common/LinkedList.h"
#include "common/Serial.h"
#include "dawn_native/Error.h"
#include "dawn_native/dawn_platform.h"

namespace dawn_native { namespace d3d12 {

    class Device;
    class Heap;

    class ResidencyManager {
      public:
        ResidencyManager(Device* device);

        MaybeError LockMappableHeap(Heap* heap);
        void UnlockMappableHeap(Heap* heap);
        MaybeError EnsureCanMakeResident(uint64_t allocationSize);
        MaybeError EnsureHeapsAreResident(Heap** heaps, size_t heapCount);

        uint64_t SetExternalMemoryReservation(uint64_t requestedReservationSize);

        void TrackResidentAllocation(Heap* heap);

      private:
        struct VideoMemoryInfo {
            uint64_t dawnBudget;
            uint64_t dawnUsage;
            uint64_t externalReservation;
            uint64_t externalRequest;
        };
        ResultOrError<Heap*> RemoveSingleEntryFromLRU();
        bool ShouldTrackHeap(Heap* heap) const;
        void UpdateVideoMemoryInfo();

        Device* mDevice;
        LinkedList<Heap> mLRUCache;
        bool mResidencyManagementEnabled = false;
        VideoMemoryInfo mVideoMemoryInfo = {};
    };

}}  // namespace dawn_native::d3d12

#endif  // DAWNNATIVE_D3D12_RESIDENCYMANAGERD3D12_H_