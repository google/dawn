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

#include "dawn/native/EventManager.h"

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/FutureUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/SystemEvent.h"

namespace dawn::native {

namespace {

// We can replace the std::vector& when std::span is available via C++20.
wgpu::WaitStatus WaitImpl(std::vector<TrackedFutureWaitInfo>& futures, Nanoseconds timeout) {
    // Sort the futures by how they'll be waited (their GetWaitDevice).
    // This lets us do each wait on a slice of the array.
    std::sort(futures.begin(), futures.end(), [](const auto& a, const auto& b) {
        // operator<() is undefined behavior for arbitrary pointers, but std::less{}() is defined.
        return std::less<DeviceBase*>{}(a.event->GetWaitDevice(), b.event->GetWaitDevice());
    });

    if (timeout > Nanoseconds(0)) {
        DAWN_ASSERT(futures.size() <= kTimedWaitAnyMaxCountDefault);

        // If there's a timeout, check that there isn't a mix of wait devices.
        if (futures.front().event->GetWaitDevice() != futures.back().event->GetWaitDevice()) {
            return wgpu::WaitStatus::UnsupportedMixedSources;
        }
    }

    // Actually do the poll or wait to find out if any of the futures became ready.
    // Here, there's either only one iteration, or timeout is 0, so we know the
    // timeout won't get stacked multiple times.
    bool anySuccess = false;
    // Find each slice of the array (sliced by wait device), and wait on it.
    for (size_t sliceStart = 0; sliceStart < futures.size();) {
        DeviceBase* waitDevice = futures[sliceStart].event->GetWaitDevice();
        size_t sliceLength = 1;
        while (sliceStart + sliceLength < futures.size() &&
               (futures[sliceStart + sliceLength].event->GetWaitDevice()) == waitDevice) {
            sliceLength++;
        }

        {
            bool success;
            if (waitDevice) {
                success = waitDevice->WaitAnyImpl(sliceLength, &futures[sliceStart], timeout);
            } else {
                success = WaitAnySystemEvent(sliceLength, &futures[sliceStart], timeout);
            }
            anySuccess |= success;
        }

        sliceStart += sliceLength;
    }
    if (!anySuccess) {
        return wgpu::WaitStatus::TimedOut;
    }
    return wgpu::WaitStatus::Success;
}

}  // namespace

// EventManager

EventManager::EventManager() {
    mEvents.emplace();  // Construct the non-movable inner struct.
}

EventManager::~EventManager() {
    DAWN_ASSERT(!mEvents.has_value());
}

MaybeError EventManager::Initialize(const InstanceDescriptor* descriptor) {
    if (descriptor) {
        if (descriptor->features.timedWaitAnyMaxCount > kTimedWaitAnyMaxCountDefault) {
            // We don't yet support a higher timedWaitAnyMaxCount because it would be complicated
            // to implement on Windows, and it isn't that useful to implement only on non-Windows.
            return DAWN_VALIDATION_ERROR("Requested timedWaitAnyMaxCount is not supported");
        }
        mTimedWaitAnyEnable = descriptor->features.timedWaitAnyEnable;
        mTimedWaitAnyMaxCount =
            std::max(kTimedWaitAnyMaxCountDefault, descriptor->features.timedWaitAnyMaxCount);
    }

    return {};
}

void EventManager::ShutDown() {
    mEvents.reset();
}

FutureID EventManager::TrackEvent(wgpu::CallbackMode mode, Ref<TrackedEvent>&& future) {
    FutureID futureID = mNextFutureID++;
    if (!mEvents.has_value()) {
        return futureID;
    }

    mEvents->Use([&](auto events) { events->emplace(futureID, std::move(future)); });
    return futureID;
}

void EventManager::ProcessPollEvents() {
    DAWN_ASSERT(mEvents.has_value());

    std::vector<TrackedFutureWaitInfo> futures;
    mEvents->Use([&](auto events) {
        // Iterate all events and record poll events and spontaneous events since they are both
        // allowed to be completed in the ProcessPoll call. Note that spontaneous events are allowed
        // to trigger anywhere which is why we include them in the call.
        futures.reserve(events->size());
        for (auto& [futureID, event] : *events) {
            if (event->mCallbackMode != wgpu::CallbackMode::WaitAnyOnly) {
                futures.push_back(
                    TrackedFutureWaitInfo{futureID, TrackedEvent::WaitRef{event.Get()}, 0, false});
            }
        }
    });

    {
        // There cannot be two competing ProcessEvent calls, so we use a lock to prevent it.
        std::lock_guard<std::mutex> lock(mProcessEventLock);
        wgpu::WaitStatus waitStatus = WaitImpl(futures, Nanoseconds(0));
        if (waitStatus == wgpu::WaitStatus::TimedOut) {
            return;
        }
        DAWN_ASSERT(waitStatus == wgpu::WaitStatus::Success);
    }

    // For all the futures we are about to complete, first ensure they're untracked. It's OK if
    // something actually isn't tracked anymore (because it completed elsewhere while waiting.)
    mEvents->Use([&](auto events) {
        for (TrackedFutureWaitInfo& future : futures) {
            if (future.ready) {
                events->erase(future.futureID);
            }
        }
    });

    // Finally, call callbacks.
    for (TrackedFutureWaitInfo& future : futures) {
        if (future.ready) {
            future.event->EnsureComplete(EventCompletionType::Ready);
        }
    }
}

wgpu::WaitStatus EventManager::WaitAny(size_t count, FutureWaitInfo* infos, Nanoseconds timeout) {
    DAWN_ASSERT(mEvents.has_value());

    // Validate for feature support.
    if (timeout > Nanoseconds(0)) {
        if (!mTimedWaitAnyEnable) {
            return wgpu::WaitStatus::UnsupportedTimeout;
        }
        if (count > mTimedWaitAnyMaxCount) {
            return wgpu::WaitStatus::UnsupportedCount;
        }
        // UnsupportedMixedSources is validated later, in WaitImpl.
    }

    if (count == 0) {
        return wgpu::WaitStatus::Success;
    }

    // Look up all of the futures and build a list of `TrackedFutureWaitInfo`s.
    std::vector<TrackedFutureWaitInfo> futures;
    futures.reserve(count);
    bool anyCompleted = false;
    mEvents->Use([&](auto events) {
        FutureID firstInvalidFutureID = mNextFutureID;
        for (size_t i = 0; i < count; ++i) {
            FutureID futureID = infos[i].future.id;

            // Check for cases that are undefined behavior in the API contract.
            DAWN_ASSERT(futureID != 0);
            DAWN_ASSERT(futureID < firstInvalidFutureID);
            // TakeWaitRef below will catch if the future is waited twice at the
            // same time (unless it's already completed).

            // Try to find the event.
            auto it = events->find(futureID);
            if (it == events->end()) {
                infos[i].completed = true;
                anyCompleted = true;
            } else {
                // TakeWaitRef below will catch if the future is waited twice at the same time
                // (unless it's already completed).
                infos[i].completed = false;
                TrackedEvent* event = it->second.Get();
                futures.push_back(
                    TrackedFutureWaitInfo{futureID, TrackedEvent::WaitRef{event}, i, false});
            }
        }
    });
    // If any completed, return immediately.
    if (anyCompleted) {
        return wgpu::WaitStatus::Success;
    }
    // Otherwise, we should have successfully looked up all of them.
    DAWN_ASSERT(futures.size() == count);

    wgpu::WaitStatus waitStatus = WaitImpl(futures, timeout);
    if (waitStatus != wgpu::WaitStatus::Success) {
        return waitStatus;
    }

    // For any futures that we're about to complete, first ensure they're untracked. It's OK if
    // something actually isn't tracked anymore (because it completed elsewhere while waiting.)
    mEvents->Use([&](auto events) {
        for (const TrackedFutureWaitInfo& future : futures) {
            if (future.ready) {
                events->erase(future.futureID);
            }
        }
    });

    // Finally, call callbacks and update return values.
    for (TrackedFutureWaitInfo& future : futures) {
        if (future.ready) {
            // Set completed before calling the callback.
            infos[future.indexInInfos].completed = true;
            // TODO(crbug.com/dawn/2066): Guarantee the event ordering from the JS spec.
            future.event->EnsureComplete(EventCompletionType::Ready);
        }
    }

    return wgpu::WaitStatus::Success;
}

// EventManager::TrackedEvent

EventManager::TrackedEvent::TrackedEvent(DeviceBase* device,
                                         wgpu::CallbackMode callbackMode,
                                         SystemEventReceiver&& receiver)
    : mDevice(device), mCallbackMode(callbackMode), mReceiver(std::move(receiver)) {}

EventManager::TrackedEvent::~TrackedEvent() {
    DAWN_ASSERT(mCompleted);
}

const SystemEventReceiver& EventManager::TrackedEvent::GetReceiver() const {
    return mReceiver;
}

DeviceBase* EventManager::TrackedEvent::GetWaitDevice() const {
    return MustWaitUsingDevice() ? mDevice.Get() : nullptr;
}

void EventManager::TrackedEvent::EnsureComplete(EventCompletionType completionType) {
    bool alreadyComplete = mCompleted.exchange(true);
    if (!alreadyComplete) {
        Complete(completionType);
    }
}

void EventManager::TrackedEvent::CompleteIfSpontaneous() {
    if (mCallbackMode == wgpu::CallbackMode::AllowSpontaneous) {
        bool alreadyComplete = mCompleted.exchange(true);
        // If it was already complete, but there was an error, we have no place
        // to report it, so DAWN_ASSERT. This shouldn't happen.
        DAWN_ASSERT(!alreadyComplete);
        Complete(EventCompletionType::Ready);
    }
}

// EventManager::TrackedEvent::WaitRef

EventManager::TrackedEvent::WaitRef::WaitRef(TrackedEvent* event) : mRef(event) {
#if DAWN_ENABLE_ASSERTS
    bool wasAlreadyWaited = mRef->mCurrentlyBeingWaited.exchange(true);
    DAWN_ASSERT(!wasAlreadyWaited);
#endif
}

EventManager::TrackedEvent::WaitRef::~WaitRef() {
#if DAWN_ENABLE_ASSERTS
    if (mRef.Get() != nullptr) {
        bool wasAlreadyWaited = mRef->mCurrentlyBeingWaited.exchange(false);
        DAWN_ASSERT(wasAlreadyWaited);
    }
#endif
}

EventManager::TrackedEvent* EventManager::TrackedEvent::WaitRef::operator->() {
    return mRef.Get();
}

const EventManager::TrackedEvent* EventManager::TrackedEvent::WaitRef::operator->() const {
    return mRef.Get();
}

}  // namespace dawn::native
