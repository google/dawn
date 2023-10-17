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
