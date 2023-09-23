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
#include <unordered_map>
#include <utility>

#include "dawn/common/FutureUtils.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/common/Ref.h"
#include "dawn/webgpu.h"
#include "dawn/wire/ObjectHandle.h"

namespace dawn::wire::client {

class Client;

// Code to run to complete the event (after receiving a ready notification from the wire).
using EventCallback = std::function<void(EventCompletionType)>;

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
    std::pair<FutureID, bool> TrackEvent(WGPUCallbackMode mode, EventCallback&& callback);
    void ShutDown();
    void SetFutureReady(FutureID futureID);
    void ProcessPollEvents();
    WGPUWaitStatus WaitAny(size_t count, WGPUFutureWaitInfo* infos, uint64_t timeoutNS);

  private:
    struct TrackedEvent : dawn::NonCopyable {
        TrackedEvent(WGPUCallbackMode mode, EventCallback&& callback);
        ~TrackedEvent();

        TrackedEvent(TrackedEvent&&) = default;
        TrackedEvent& operator=(TrackedEvent&&) = default;

        WGPUCallbackMode mMode;
        // Callback. Falsey if already called.
        EventCallback mCallback;
        // These states don't need to be atomic because they're always protected by
        // mTrackedEventsMutex (or moved out to a local variable).
        bool mReady = false;
    };

    Client* mClient;

    // Tracks all kinds of events (for both WaitAny and ProcessEvents).
    MutexProtected<std::unordered_map<FutureID, TrackedEvent>> mTrackedEvents;
    std::atomic<FutureID> mNextFutureID = 1;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_EVENTMANAGER_H_
