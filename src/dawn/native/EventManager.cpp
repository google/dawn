// Copyright 2023 The Dawn Authors
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
    mTrackers.emplace();  // Construct the non-movable inner struct.
}

EventManager::~EventManager() {
    DAWN_ASSERT(!mTrackers.has_value());
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
    mTrackers.reset();
}

FutureID EventManager::TrackEvent(wgpu::CallbackMode mode, Ref<TrackedEvent>&& future) {
    // TODO(crbug.com/dawn/2052) Can remove the validation on the mode once it's an enum.
    switch (ValidateAndFlattenCallbackMode(static_cast<WGPUCallbackModeFlags>(mode))) {
        case CallbackMode::Spontaneous:
            // We don't need to track the future because some other code is responsible for
            // completing it, and we aren't returning an ID so we don't need to be able to query it.
            return kNullFutureID;
        case CallbackMode::Future:
        case CallbackMode::FutureOrSpontaneous: {
            FutureID futureID = mNextFutureID++;
            if (mTrackers.has_value()) {
                mTrackers->futures->emplace(futureID, std::move(future));
            }
            return futureID;
        }
        case CallbackMode::ProcessEvents:
        case CallbackMode::ProcessEventsOrSpontaneous: {
            FutureID futureID = mNextFutureID++;
            if (mTrackers.has_value()) {
                mTrackers->pollEvents->emplace(futureID, std::move(future));
            }
            // Return null future, because the user didn't actually ask for a future.
            return kNullFutureID;
        }
    }
}

void EventManager::ProcessPollEvents() {
    DAWN_ASSERT(mTrackers.has_value());

    std::vector<TrackedFutureWaitInfo> futures;
    mTrackers->pollEvents.Use([&](auto trackedPollEvents) {
        futures.reserve(trackedPollEvents->size());

        for (auto& [futureID, event] : *trackedPollEvents) {
            futures.push_back(
                TrackedFutureWaitInfo{futureID, TrackedEvent::WaitRef{event.Get()}, 0, false});
        }

        // The WaitImpl is inside of the lock to prevent any two ProcessEvents calls from
        // calling competing OS wait syscalls at the same time.
        wgpu::WaitStatus waitStatus = WaitImpl(futures, Nanoseconds(0));
        if (waitStatus == wgpu::WaitStatus::TimedOut) {
            return;
        }
        DAWN_ASSERT(waitStatus == wgpu::WaitStatus::Success);

        for (TrackedFutureWaitInfo& future : futures) {
            if (future.ready) {
                trackedPollEvents->erase(future.futureID);
            }
        }
    });

    for (TrackedFutureWaitInfo& future : futures) {
        if (future.ready) {
            DAWN_ASSERT(future.event->mCallbackMode & wgpu::CallbackMode::ProcessEvents);
            future.event->EnsureComplete(EventCompletionType::Ready);
        }
    }
}

wgpu::WaitStatus EventManager::WaitAny(size_t count, FutureWaitInfo* infos, Nanoseconds timeout) {
    DAWN_ASSERT(mTrackers.has_value());

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
    mTrackers->futures.Use([&](auto trackedFutures) {
        FutureID firstInvalidFutureID = mNextFutureID;
        for (size_t i = 0; i < count; ++i) {
            FutureID futureID = infos[i].future.id;

            // Check for cases that are undefined behavior in the API contract.
            DAWN_ASSERT(futureID != 0);
            DAWN_ASSERT(futureID < firstInvalidFutureID);
            // TakeWaitRef below will catch if the future is waited twice at the
            // same time (unless it's already completed).

            auto it = trackedFutures->find(futureID);
            if (it == trackedFutures->end()) {
                infos[i].completed = true;
                anyCompleted = true;
            } else {
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
    mTrackers->futures.Use([&](auto trackedFutures) {
        for (const TrackedFutureWaitInfo& future : futures) {
            if (future.ready) {
                trackedFutures->erase(future.futureID);
            }
        }
    });

    // Finally, call callbacks and update return values.
    for (TrackedFutureWaitInfo& future : futures) {
        if (future.ready) {
            // Set completed before calling the callback.
            infos[future.indexInInfos].completed = true;
            // TODO(crbug.com/dawn/2066): Guarantee the event ordering from the JS spec.
            DAWN_ASSERT(future.event->mCallbackMode & wgpu::CallbackMode::Future);
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
    if (mCallbackMode & wgpu::CallbackMode::Spontaneous) {
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
