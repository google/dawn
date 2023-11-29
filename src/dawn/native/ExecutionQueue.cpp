// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/ExecutionQueue.h"

namespace dawn::native {

ExecutionSerial ExecutionQueueBase::GetPendingCommandSerial() const {
    return ExecutionSerial(mLastSubmittedSerial.load(std::memory_order_acquire) + 1);
}

ExecutionSerial ExecutionQueueBase::GetLastSubmittedCommandSerial() const {
    return ExecutionSerial(mLastSubmittedSerial.load(std::memory_order_acquire));
}

ExecutionSerial ExecutionQueueBase::GetCompletedCommandSerial() const {
    return mCompletedSerial;
}

MaybeError ExecutionQueueBase::CheckPassedSerials() {
    ExecutionSerial completedSerial;
    DAWN_TRY_ASSIGN(completedSerial, CheckAndUpdateCompletedSerials());

    DAWN_ASSERT(completedSerial <=
                ExecutionSerial(mLastSubmittedSerial.load(std::memory_order_acquire)));
    // completedSerial should not be less than mCompletedSerial unless it is 0.
    // It can be 0 when there's no fences to check.
    DAWN_ASSERT(completedSerial >= mCompletedSerial || completedSerial == ExecutionSerial(0));

    if (completedSerial > mCompletedSerial) {
        mCompletedSerial = completedSerial;
    }

    return {};
}

void ExecutionQueueBase::AssumeCommandsComplete() {
    // Bump serials so any pending callbacks can be fired.
    uint64_t prev = mLastSubmittedSerial.fetch_add(1u, std::memory_order_release);
    mCompletedSerial = ExecutionSerial(prev + 1);
}

void ExecutionQueueBase::IncrementLastSubmittedCommandSerial() {
    mLastSubmittedSerial.fetch_add(1u, std::memory_order_release);
}

bool ExecutionQueueBase::HasScheduledCommands() const {
    return ExecutionSerial(mLastSubmittedSerial.load(std::memory_order_acquire)) >
               mCompletedSerial ||
           HasPendingCommands();
}

// All prevously submitted works at the moment will supposedly complete at this serial.
// Internally the serial is computed according to whether frontend and backend have pending
// commands. There are 4 cases of combination:
//   1) Frontend(No), Backend(No)
//   2) Frontend(No), Backend(Yes)
//   3) Frontend(Yes), Backend(No)
//   4) Frontend(Yes), Backend(Yes)
// For case 1, we don't need the serial to track the task as we can ack it right now.
// For case 2 and 4, there will be at least an eventual submission, so we can use
// 'GetPendingCommandSerial' as the serial.
// For case 3, we can't use 'GetPendingCommandSerial' as it won't be submitted surely. Instead we
// use 'GetLastSubmittedCommandSerial', which must be fired eventually.
ExecutionSerial ExecutionQueueBase::GetScheduledWorkDoneSerial() const {
    return HasPendingCommands() ? GetPendingCommandSerial() : GetLastSubmittedCommandSerial();
}

}  // namespace dawn::native
