// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/native/d3d12/CommandAllocatorManager.h"

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/DeviceD3D12.h"

#include "dawn/common/Assert.h"
#include "dawn/common/BitSetIterator.h"

namespace dawn::native::d3d12 {

CommandAllocatorManager::CommandAllocatorManager(Device* device)
    : device(device), mAllocatorCount(0) {
    mFreeAllocators.set();
}

ResultOrError<ID3D12CommandAllocator*> CommandAllocatorManager::ReserveCommandAllocator() {
    // If there are no free allocators, get the oldest serial in flight and wait on it
    if (mFreeAllocators.none()) {
        const ExecutionSerial firstSerial = mInFlightCommandAllocators.FirstSerial();
        DAWN_TRY(device->WaitForSerial(firstSerial));
        DAWN_TRY(Tick(firstSerial));
    }

    DAWN_ASSERT(mFreeAllocators.any());

    // Get the index of the first free allocator from the bitset
    unsigned int firstFreeIndex = *(IterateBitSet(mFreeAllocators).begin());

    if (firstFreeIndex >= mAllocatorCount) {
        DAWN_ASSERT(firstFreeIndex == mAllocatorCount);
        mAllocatorCount++;
        DAWN_TRY(CheckHRESULT(
            device->GetD3D12Device()->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocators[firstFreeIndex])),
            "D3D12 create command allocator"));
    }

    // Mark the command allocator as used
    mFreeAllocators.reset(firstFreeIndex);

    // Enqueue the command allocator. It will be scheduled for reset after the next
    // ExecuteCommandLists
    mInFlightCommandAllocators.Enqueue({mCommandAllocators[firstFreeIndex], firstFreeIndex},
                                       device->GetPendingCommandSerial());
    return mCommandAllocators[firstFreeIndex].Get();
}

MaybeError CommandAllocatorManager::Tick(ExecutionSerial lastCompletedSerial) {
    // Reset all command allocators that are no longer in flight
    for (auto it : mInFlightCommandAllocators.IterateUpTo(lastCompletedSerial)) {
        DAWN_TRY(CheckHRESULT(it.commandAllocator->Reset(), "D3D12 reset command allocator"));
        mFreeAllocators.set(it.index);
    }
    mInFlightCommandAllocators.ClearUpTo(lastCompletedSerial);
    return {};
}

}  // namespace dawn::native::d3d12
