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

#ifndef SRC_DAWN_WIRE_CLIENT_EVENTMANAGER_H_
#define SRC_DAWN_WIRE_CLIENT_EVENTMANAGER_H_

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <utility>

#include "dawn/common/FutureUtils.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/webgpu.h"
#include "dawn/wire/WireResult.h"

namespace dawn::wire::client {

class Client;

enum class EventType {
    MapAsync,
    WorkDone,
};

// Implementations of TrackedEvents must implement the CompleteImpl and ReadyHook functions. In most
// scenarios, the CompleteImpl function should call the callbacks while the ReadyHook should process
// and copy memory (if necessary) from the wire deserialization buffer into a local copy that can be
// readily used by the user callback.
class TrackedEvent : NonMovable {
  public:
    explicit TrackedEvent(WGPUCallbackMode mode);
    virtual ~TrackedEvent();

    virtual EventType GetType() = 0;

    WGPUCallbackMode GetCallbackMode() const;
    bool IsReady() const;

    void SetReady();
    void Complete(EventCompletionType type);

  protected:
    virtual void CompleteImpl(EventCompletionType type) = 0;

    const WGPUCallbackMode mMode;
    enum class EventState {
        Pending,
        Ready,
        Complete,
    };
    EventState mEventState = EventState::Pending;
};

// Subcomponent which tracks callback events for the Future-based callback
// entrypoints. All events from this instance (regardless of whether from an adapter, device, queue,
// etc.) are tracked here, and used by the instance-wide ProcessEvents and WaitAny entrypoints.
//
// TODO(crbug.com/dawn/2060): This should probably be merged together with RequestTracker.
class EventManager final : NonMovable {
  public:
    explicit EventManager(Client*);
    ~EventManager() = default;

    // Returns a pair of the FutureID and a bool that is true iff the event was successfuly tracked,
    // false otherwise. Events may not be tracked if the client is already disconnected.
    std::pair<FutureID, bool> TrackEvent(std::unique_ptr<TrackedEvent> event);
    void ShutDown();

    template <typename Event, typename... ReadyArgs>
    WireResult SetFutureReady(FutureID futureID, ReadyArgs&&... readyArgs) {
        DAWN_ASSERT(futureID > 0);
        // If already shutdown, then all the callbacks should already have fired so we don't need to
        // fire the callback anymore. This may happen if cleanup/dtor functions try to call this
        // unconditionally on objects.
        if (mIsShutdown) {
#if DAWN_ENABLE_ASSERTS
            // Note we need to use an if clause here because otherwise the DAWN_ASSERT macro will
            // generate code that results in the lambda being in an unevaluated context.
            DAWN_ASSERT(mTrackedEvents.Use([&](auto trackedEvents) {
                return trackedEvents->find(futureID) == trackedEvents->end();
            }));
#endif
            return WireResult::Success;
        }

        std::unique_ptr<TrackedEvent> spontaneousEvent;
        WIRE_TRY(mTrackedEvents.Use([&](auto trackedEvents) {
            auto it = trackedEvents->find(futureID);
            if (it == trackedEvents->end()) {
                return WireResult::FatalError;
            }
            auto& trackedEvent = it->second;

            if (trackedEvent->GetType() != Event::kType) {
                // Assert here for debugging, before returning a fatal error that is handled upwards
                // in production.
                DAWN_ASSERT(trackedEvent->GetType() == Event::kType);
                return WireResult::FatalError;
            }
            static_cast<Event*>(trackedEvent.get())
                ->ReadyHook(std::forward<ReadyArgs>(readyArgs)...);
            trackedEvent->SetReady();

            // If the event can be spontaneously completed, prepare to do so now.
            if (trackedEvent->GetCallbackMode() == WGPUCallbackMode_AllowSpontaneous) {
                spontaneousEvent = std::move(trackedEvent);
                trackedEvents->erase(futureID);
            }
            return WireResult::Success;
        }));

        // Handle spontaneous completions.
        if (spontaneousEvent) {
            spontaneousEvent->Complete(EventCompletionType::Ready);
        }
        return WireResult::Success;
    }

    void ProcessPollEvents();
    WGPUWaitStatus WaitAny(size_t count, WGPUFutureWaitInfo* infos, uint64_t timeoutNS);

  private:
    Client* mClient;
    bool mIsShutdown = false;

    // Tracks all kinds of events (for both WaitAny and ProcessEvents). We use an ordered map so
    // that in most cases, event ordering is already implicit when we iterate the map. (Not true for
    // WaitAny though because the user could specify the FutureIDs out of order.)
    MutexProtected<std::map<FutureID, std::unique_ptr<TrackedEvent>>> mTrackedEvents;
    std::atomic<FutureID> mNextFutureID = 1;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_EVENTMANAGER_H_
