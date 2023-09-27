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
#include "dawn/common/Ref.h"
#include "dawn/webgpu.h"
#include "dawn/wire/ObjectHandle.h"

namespace dawn::wire::client {

class Client;

struct TrackedEvent : NonMovable {
    TrackedEvent(WGPUCallbackMode mode, void* userdata);
    virtual ~TrackedEvent();

    void Complete(EventCompletionType type);

    WGPUCallbackMode mMode;
    void* mUserdata = nullptr;
    // These states don't need to be atomic because they're always protected by mTrackedEventsMutex
    // (or moved out to a local variable).
    bool mReady = false;

  protected:
    virtual void CompleteImpl(EventCompletionType type) = 0;
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
    std::pair<FutureID, bool> TrackEvent(TrackedEvent* event);
    void ShutDown();
    void SetFutureReady(FutureID futureID, std::function<void(TrackedEvent&)>&& ready = {});
    void ProcessPollEvents();
    WGPUWaitStatus WaitAny(size_t count, WGPUFutureWaitInfo* infos, uint64_t timeoutNS);

  private:
    Client* mClient;

    // Tracks all kinds of events (for both WaitAny and ProcessEvents). We use an ordered map so
    // that in most cases, event ordering is already implicit when we iterate the map. (Not true for
    // WaitAny though because the user could specify the FutureIDs out of order.)
    MutexProtected<std::map<FutureID, std::unique_ptr<TrackedEvent>>> mTrackedEvents;
    std::atomic<FutureID> mNextFutureID = 1;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_EVENTMANAGER_H_
