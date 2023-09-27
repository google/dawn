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

#include <map>
#include <optional>
#include <utility>
#include <vector>

#include "dawn/wire/ObjectHandle.h"
#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/EventManager.h"

namespace dawn::wire::client {

// EventManager

EventManager::EventManager(Client* client) : mClient(client) {}

std::pair<FutureID, bool> EventManager::TrackEvent(TrackedEvent* event) {
    FutureID futureID = mNextFutureID++;
    std::unique_ptr<TrackedEvent> ptr(event);

    if (mClient->IsDisconnected()) {
        std::move(ptr)->Complete(EventCompletionType::Shutdown);
        return {futureID, false};
    }

    mTrackedEvents.Use([&](auto trackedEvents) {
        auto [it, inserted] = trackedEvents->emplace(futureID, std::move(ptr));
        DAWN_ASSERT(inserted);
    });

    return {futureID, true};
}

void EventManager::ShutDown() {
    // Call any outstanding callbacks before destruction.
    while (true) {
        std::map<FutureID, std::unique_ptr<TrackedEvent>> movedEvents;
        mTrackedEvents.Use([&](auto trackedEvents) { movedEvents = std::move(*trackedEvents); });

        if (movedEvents.empty()) {
            break;
        }

        // Ordering guaranteed because we are using a sorted map.
        for (auto& [futureID, trackedEvent] : movedEvents) {
            std::move(trackedEvent)->Complete(EventCompletionType::Shutdown);
        }
    }
}

void EventManager::SetFutureReady(FutureID futureID, std::function<void(TrackedEvent&)>&& ready) {
    DAWN_ASSERT(futureID > 0);
    // If the client was already disconnected, then all the callbacks should already have fired so
    // we don't need to fire the callback anymore.
    if (mClient->IsDisconnected()) {
        return;
    }

    std::optional<std::unique_ptr<TrackedEvent>> event;
    mTrackedEvents.Use([&](auto trackedEvents) {
        std::unique_ptr<TrackedEvent>& trackedEvent =
            trackedEvents->at(futureID);  // Asserts futureID is in the map
        trackedEvent->mReady = true;
        if (ready) {
            ready(*trackedEvent);
        }

        // If the event can be spontaneously completed, do so now.
        if (trackedEvent->mMode == WGPUCallbackMode_AllowSpontaneous) {
            event = std::move(trackedEvent);
            trackedEvents->erase(futureID);
        }
    });

    // Handle spontaneous completions.
    if (event.has_value()) {
        std::move(*event)->Complete(EventCompletionType::Ready);
    }
}

void EventManager::ProcessPollEvents() {
    // Since events are already stored in an ordered map, this list must already be ordered.
    std::vector<std::unique_ptr<TrackedEvent>> eventsToCompleteNow;
    mTrackedEvents.Use([&](auto trackedEvents) {
        for (auto it = trackedEvents->begin(); it != trackedEvents->end();) {
            std::unique_ptr<TrackedEvent>& event = it->second;
            bool shouldRemove = (event->mMode == WGPUCallbackMode_AllowProcessEvents ||
                                 event->mMode == WGPUCallbackMode_AllowSpontaneous) &&
                                event->mReady;
            if (!shouldRemove) {
                ++it;
                continue;
            }
            eventsToCompleteNow.emplace_back(std::move(event));
            it = trackedEvents->erase(it);
        }
    });

    for (std::unique_ptr<TrackedEvent>& event : eventsToCompleteNow) {
        std::move(event)->Complete(EventCompletionType::Ready);
    }
}

WGPUWaitStatus EventManager::WaitAny(size_t count, WGPUFutureWaitInfo* infos, uint64_t timeoutNS) {
    // Validate for feature support.
    if (timeoutNS > 0) {
        // Wire doesn't support timedWaitEnable (for now). (There's no UnsupportedCount or
        // UnsupportedMixedSources validation here, because those only apply to timed waits.)
        //
        // TODO(crbug.com/dawn/1987): CreateInstance needs to validate timedWaitEnable was false.
        return WGPUWaitStatus_UnsupportedTimeout;
    }

    if (count == 0) {
        return WGPUWaitStatus_Success;
    }

    // Since the user can specify the FutureIDs in any order, we need to use another ordered map
    // here to ensure that the result is ordered for JS event ordering.
    std::map<FutureID, std::unique_ptr<TrackedEvent>> eventsToCompleteNow;
    bool anyCompleted = false;
    const FutureID firstInvalidFutureID = mNextFutureID;
    mTrackedEvents.Use([&](auto trackedEvents) {
        for (size_t i = 0; i < count; ++i) {
            FutureID futureID = infos[i].future.id;
            DAWN_ASSERT(futureID < firstInvalidFutureID);

            auto it = trackedEvents->find(futureID);
            if (it == trackedEvents->end()) {
                infos[i].completed = true;
                anyCompleted = true;
                continue;
            }

            std::unique_ptr<TrackedEvent>& event = it->second;
            // Early update .completed, in prep to complete the callback if ready.
            infos[i].completed = event->mReady;
            if (event->mReady) {
                anyCompleted = true;
                eventsToCompleteNow.emplace(it->first, std::move(event));
                trackedEvents->erase(it);
            }
        }
    });

    for (auto& [_, event] : eventsToCompleteNow) {
        // .completed has already been set to true (before the callback, per API contract).
        std::move(event)->Complete(EventCompletionType::Ready);
    }

    return anyCompleted ? WGPUWaitStatus_Success : WGPUWaitStatus_TimedOut;
}

// TrackedEvent

TrackedEvent::TrackedEvent(WGPUCallbackMode mode, void* userdata)
    : mMode(mode), mUserdata(userdata) {}

TrackedEvent::~TrackedEvent() = default;

void TrackedEvent::Complete(EventCompletionType type) {
    DAWN_ASSERT(type == EventCompletionType::Shutdown || mReady);
    CompleteImpl(type);
}

}  // namespace dawn::wire::client
