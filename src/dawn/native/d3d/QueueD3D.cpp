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

#include "dawn/native/d3d/QueueD3D.h"

#include <utility>

#include "dawn/native/WaitAnySystemEvent.h"

namespace dawn::native::d3d {

Queue::~Queue() = default;

ResultOrError<bool> Queue::WaitForQueueSerial(ExecutionSerial serial, Nanoseconds timeout) {
    ExecutionSerial completedSerial = GetCompletedCommandSerial();
    if (serial <= completedSerial) {
        return true;
    }

    auto sharedReceiver = GetOrCreateSystemEventReceiver(serial);

    bool ready = false;
    std::array<std::pair<const dawn::native::SystemEventReceiver&, bool*>, 1> events{
        {{sharedReceiver->receiver, &ready}}};
    DAWN_ASSERT(serial <= GetLastSubmittedCommandSerial());
    bool didComplete = WaitAnySystemEvent(events.begin(), events.end(), timeout);
    if (didComplete) {
        // Clear out everything up to this serial since they are all complete now.
        mSystemEventReceivers->ClearUpTo(serial);
    }
    return didComplete;
}

Ref<SharedSystemEventReceiver> Queue::GetOrCreateSystemEventReceiver(
    ExecutionSerial completionSerial) {
    return mSystemEventReceivers.Use([&](auto receivers) {
        if (auto* receiver = receivers->FindOne(completionSerial)) {
            return *receiver;
        }

        if (completionSerial <= GetCompletedCommandSerial()) {
            return AcquireRef(
                new SharedSystemEventReceiver(SystemEventReceiver::CreateAlreadySignaled()));
        }

        HANDLE fenceEvent =
            ::CreateEvent(nullptr, /*bManualReset=*/true, /*bInitialState=*/false, nullptr);
        DAWN_CHECK(fenceEvent != nullptr);
        SetEventOnCompletion(completionSerial, fenceEvent);

        // Make a boxed SystemEventReceiver and enqueue it into the list of receivers.
        auto sharedReceiver = AcquireRef(
            new SharedSystemEventReceiver(SystemEventReceiver(SystemHandle::Acquire(fenceEvent))));
        receivers->Enqueue(sharedReceiver, completionSerial);
        return sharedReceiver;
    });
}

}  // namespace dawn::native::d3d
