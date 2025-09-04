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

#include "dawn/native/vulkan/FencedDeleter.h"

#include <algorithm>

#include "dawn/native/IntegerTypes.h"
#include "dawn/native/Queue.h"
#include "dawn/native/vulkan/DeviceVk.h"

namespace dawn::native::vulkan {

FencedDeleter::FencedDeleter(Device* device) : mDevice(device) {}

FencedDeleter::~FencedDeleter() {
    // Flush any pending deletions.
    DeleteUpTo(kMaxExecutionSerial);
}

#define X(Type, ...)                                                 \
    void FencedDeleter::DeleteWhenUnused(Type handle) {              \
        ExecutionSerial deletionSerial = GetCurrentDeletionSerial(); \
        mPendingDeletions->m##Type.Enqueue(handle, deletionSerial);  \
        EnsureTask(deletionSerial);                                  \
    }
FENCED_DELETER_TYPES(X)
#undef X

ExecutionSerial FencedDeleter::GetLastPendingDeletionSerial() {
    ExecutionSerial lastSerial = kBeginningOfGPUTime;
    mPendingDeletions.Use([&](auto pending) {
#define X(Type, ...)                                                      \
    if (!pending->m##Type.Empty()) {                                      \
        lastSerial = std::max(lastSerial, pending->m##Type.LastSerial()); \
    }
        FENCED_DELETER_TYPES(X)
#undef X
    });
    return lastSerial;
}

ExecutionSerial FencedDeleter::GetCurrentDeletionSerial() {
    return mDevice->GetQueue()->GetPendingCommandSerial();
}

void FencedDeleter::DeleteUpTo(ExecutionSerial completedSerial) {
    VkDevice device = mDevice->GetVkDevice();
    VkInstance instance = mDevice->GetVkInstance();

    mPendingDeletions.Use([&](auto pending) {
#define X(Type, DeleteFn, deleteWith)                                   \
    for (Type handle : pending->m##Type.IterateUpTo(completedSerial)) { \
        mDevice->fn.DeleteFn(deleteWith, handle, nullptr);              \
    }                                                                   \
    pending->m##Type.ClearUpTo(completedSerial);
        FENCED_DELETER_TYPES(X)
#undef X
    });
}

void FencedDeleter::EnsureTask(ExecutionSerial deletionSerial) {
    // Schedules a new task to process deletions if the deletion serial is newer than the one for
    // the last scheduled task. Grouping deletion tasks together by serial like this ensures that
    // the deletion order encoded at the top of the file is preserved for all deletions within a
    // given serial. Dependenant deletions (such deleting memories before the buffers that use them)
    // must not span multiple serials.

    // The deletion serials used are always from GetPendingCommandSerial()
    // and as such will never go backwards or be an already completed serial, so checking for a
    // newer serial should be sufficient criteria for new task creation.
    if (mLastTaskSerial.exchange(deletionSerial) != deletionSerial) {
        // The this pointer here should remain valid for the lifetime of the tasks because the
        // deleter is owned by the device and will only be destroyed once the queue's serial tasks
        // have been flushed.
        mDevice->GetQueue()->TrackSerialTask(
            deletionSerial, [this, deletionSerial]() { DeleteUpTo(deletionSerial); });
    }
}

}  // namespace dawn::native::vulkan
