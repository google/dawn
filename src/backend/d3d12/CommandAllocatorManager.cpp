// Copyright 2017 The NXT Authors
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

#include "CommandAllocatorManager.h"

#include "D3D12Backend.h"

#include "common/BitSetIterator.h"

namespace backend {
namespace d3d12 {

    CommandAllocatorManager::CommandAllocatorManager(Device* device) : device(device), allocatorCount(0) {
        freeAllocators.set();
    }

    ComPtr<ID3D12CommandAllocator> CommandAllocatorManager::ReserveCommandAllocator() {
        // If there are no free allocators, get the oldest serial in flight and wait on it
        if (freeAllocators.none()) {
            const uint64_t firstSerial = inFlightCommandAllocators.FirstSerial();
            device->WaitForSerial(firstSerial);
            Tick(firstSerial);
        }

        ASSERT(freeAllocators.any());

        // Get the index of the first free allocator from the bitset
        unsigned int firstFreeIndex = *(IterateBitSet(freeAllocators).begin());

        if (firstFreeIndex >= allocatorCount) {
            ASSERT(firstFreeIndex == allocatorCount);
            allocatorCount++;
            ASSERT_SUCCESS(device->GetD3D12Device()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[firstFreeIndex])));
        }

        // Mark the command allocator as used
        freeAllocators.reset(firstFreeIndex);

        // Enqueue the command allocator. It will be scheduled for reset after the next ExecuteCommandLists
        inFlightCommandAllocators.Enqueue({commandAllocators[firstFreeIndex], firstFreeIndex}, device->GetSerial());

        return commandAllocators[firstFreeIndex];
    }

    void CommandAllocatorManager::Tick(uint64_t lastCompletedSerial) {
        // Reset all command allocators that are no longer in flight
        for (auto it : inFlightCommandAllocators.IterateUpTo(lastCompletedSerial)) {
            ASSERT_SUCCESS(it.commandAllocator->Reset());
            freeAllocators.set(it.index);
        }
        inFlightCommandAllocators.ClearUpTo(lastCompletedSerial);
    }

}
}
