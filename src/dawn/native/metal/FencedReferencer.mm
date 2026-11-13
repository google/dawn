// Copyright 2026 The Dawn & Tint Authors
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

#include "src/dawn/native/metal/FencedReferencer.h"

#include <algorithm>

#include "src/dawn/native/IntegerTypes.h"
#include "src/dawn/native/Queue.h"
#include "src/dawn/native/metal/DeviceMTL.h"

namespace dawn::native::metal {

FencedReferencer::FencedReferencer(Device* device) : mDevice(device) {}

FencedReferencer::~FencedReferencer() {
    // There should be nothing left to delete by this point.
    DAWN_ASSERT(mPendingDeletions.Use([&](auto pending) { return pending->mObjects.Empty(); }));
}

void FencedReferencer::RefUntilUnused(id handle) {
    mPendingDeletions.Use([&](auto pending) {
        ExecutionSerial deletionSerial = mDevice->GetQueue()->GetPendingCommandSerial();
        if (pending->mCompletedSerial >= deletionSerial) {
            // If the serial has already passed (e.g. queue is shutdown), we can just drop the ref.
            return;
        }
        pending->mObjects.Enqueue(handle, deletionSerial);
    });
}

void FencedReferencer::UpdateCompletedSerialTo(ExecutionSerial completedSerial) {
    mPendingDeletions.Use([&](auto pending) {  //
        pending->mObjects.ClearUpTo(completedSerial);
        pending->mCompletedSerial = std::max(pending->mCompletedSerial, completedSerial);
    });
}

bool FencedReferencer::HasPendingObjectsForTesting() const {
    return mPendingDeletions.Use([](const auto& pending) { return !pending->mObjects.Empty(); });
}

}  // namespace dawn::native::metal
