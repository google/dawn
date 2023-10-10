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

#include "dawn/wire/client/EventManager.h"

#include "dawn/wire/client/Client.h"

namespace dawn::wire::client {

// TrackedEvent

TrackedEvent::TrackedEvent(WGPUCallbackMode mode) : mMode(mode) {}

TrackedEvent::~TrackedEvent() {
    DAWN_ASSERT(mEventState == EventState::Complete);
}

WGPUCallbackMode TrackedEvent::GetCallbackMode() const {
    return mMode;
}

bool TrackedEvent::IsReady() const {
    return mEventState == EventState::Ready;
}

void TrackedEvent::SetReady() {
    DAWN_ASSERT(mEventState == EventState::Pending);
    mEventState = EventState::Ready;
}

void TrackedEvent::Complete(EventCompletionType type) {
    DAWN_ASSERT(mEventState != EventState::Complete);
    CompleteImpl(type);
    mEventState = EventState::Complete;
}

// EventManager

EventManager::EventManager(Client* client) : mClient(client) {}

std::pair<FutureID, bool> EventManager::TrackEvent(std::unique_ptr<TrackedEvent> event) {
    FutureID futureID = mNextFutureID++;

    if (mClient->IsDisconnected()) {
        event->Complete(EventCompletionType::Shutdown);
        return {futureID, false};
    }

    mTrackedEvents.Use([&](auto trackedEvents) {
        auto [it, inserted] = trackedEvents->emplace(futureID, std::move(event));
        DAWN_ASSERT(inserted);
    });

    return {futureID, true};
}

void EventManager::ShutDown() {
    // Call any outstanding callbacks before destruction.
    while (true) {
        std::map<FutureID, std::unique_ptr<TrackedEvent>> events;
        mTrackedEvents.Use([&](auto trackedEvents) { events = std::move(*trackedEvents); });

        if (events.empty()) {
            break;
        }

        // Ordering guaranteed because we are using a sorted map.
        for (auto& [futureID, event] : events) {
            event->Complete(EventCompletionType::Shutdown);
        }
    }
    mIsShutdown = true;
}

void EventManager::ProcessPollEvents() {
    // Since events are already stored in an ordered map, this list must already be ordered.
    std::vector<std::unique_ptr<TrackedEvent>> eventsToCompleteNow;
    mTrackedEvents.Use([&](auto trackedEvents) {
        for (auto it = trackedEvents->begin(); it != trackedEvents->end();) {
            auto& event = it->second;
            WGPUCallbackMode callbackMode = event->GetCallbackMode();
            bool shouldRemove = (callbackMode == WGPUCallbackMode_AllowProcessEvents ||
                                 callbackMode == WGPUCallbackMode_AllowSpontaneous) &&
                                event->IsReady();
            if (!shouldRemove) {
                ++it;
                continue;
            }
            eventsToCompleteNow.emplace_back(std::move(event));
            it = trackedEvents->erase(it);
        }
    });

    for (auto& event : eventsToCompleteNow) {
        event->Complete(EventCompletionType::Ready);
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

            auto& event = it->second;
            // Early update .completed, in prep to complete the callback if ready.
            infos[i].completed = event->IsReady();
            if (event->IsReady()) {
                anyCompleted = true;
                eventsToCompleteNow.emplace(it->first, std::move(event));
                trackedEvents->erase(it);
            }
        }
    });

    for (auto& [_, event] : eventsToCompleteNow) {
        // .completed has already been set to true (before the callback, per API contract).
        event->Complete(EventCompletionType::Ready);
    }

    return anyCompleted ? WGPUWaitStatus_Success : WGPUWaitStatus_TimedOut;
}

}  // namespace dawn::wire::client
