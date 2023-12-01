// Copyright 2021 The Dawn & Tint Authors
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

#include "dawn/wire/client/Instance.h"

#include <memory>
#include <string>
#include <utility>

#include "dawn/common/Log.h"
#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/EventManager.h"

namespace dawn::wire::client {
namespace {

class RequestAdapterEvent : public TrackedEvent {
  public:
    static constexpr EventType kType = EventType::RequestAdapter;

    RequestAdapterEvent(const WGPURequestAdapterCallbackInfo& callbackInfo, Adapter* adapter)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata(callbackInfo.userdata),
          mAdapter(adapter) {}

    EventType GetType() override { return kType; }

    void ReadyHook(WGPURequestAdapterStatus status,
                   const char* message,
                   const WGPUAdapterProperties* properties,
                   const WGPUSupportedLimits* limits,
                   uint32_t featuresCount,
                   const WGPUFeatureName* features) {
        DAWN_ASSERT(mAdapter != nullptr);
        mStatus = status;
        if (message != nullptr) {
            mMessage = message;
        }
        if (status == WGPURequestAdapterStatus_Success) {
            mAdapter->SetProperties(properties);
            mAdapter->SetLimits(limits);
            mAdapter->SetFeatures(features, featuresCount);
        }
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPURequestAdapterStatus_Unknown;
            mMessage = "GPU connection lost";
        }
        if (mStatus != WGPURequestAdapterStatus_Success && mAdapter != nullptr) {
            // If there was an error, we may need to reclaim the adapter allocation, otherwise the
            // adapter is returned to the user who owns it.
            mAdapter->GetClient()->Free(mAdapter);
            mAdapter = nullptr;
        }
        if (mCallback) {
            mCallback(mStatus, ToAPI(mAdapter), mMessage ? mMessage->c_str() : nullptr, mUserdata);
        }
    }

    WGPURequestAdapterCallback mCallback;
    void* mUserdata;

    // Note that the message is optional because we want to return nullptr when it wasn't set
    // instead of a pointer to an empty string.
    WGPURequestAdapterStatus mStatus;
    std::optional<std::string> mMessage;

    // The adapter is created when we call RequestAdapter(F). It is guaranteed to be alive
    // throughout the duration of a RequestAdapterEvent because the Event essentially takes
    // ownership of it until either an error occurs at which point the Event cleans it up, or it
    // returns the adapter to the user who then takes ownership as the Event goes away.
    Adapter* mAdapter = nullptr;
};

}  // anonymous namespace

// Free-standing API functions

WGPUBool ClientGetInstanceFeatures(WGPUInstanceFeatures* features) {
    if (features->nextInChain != nullptr) {
        return false;
    }

    features->timedWaitAnyEnable = false;
    features->timedWaitAnyMaxCount = kTimedWaitAnyMaxCountDefault;
    return true;
}

WGPUInstance ClientCreateInstance(WGPUInstanceDescriptor const* descriptor) {
    DAWN_UNREACHABLE();
    return nullptr;
}

// Instance

void Instance::RequestAdapter(const WGPURequestAdapterOptions* options,
                              WGPURequestAdapterCallback callback,
                              void* userdata) {
    WGPURequestAdapterCallbackInfo callbackInfo = {};
    callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
    callbackInfo.callback = callback;
    callbackInfo.userdata = userdata;
    RequestAdapterF(options, callbackInfo);
}

WGPUFuture Instance::RequestAdapterF(const WGPURequestAdapterOptions* options,
                                     const WGPURequestAdapterCallbackInfo& callbackInfo) {
    Client* client = GetClient();
    Adapter* adapter = client->Make<Adapter>();
    auto [futureIDInternal, tracked] = client->GetEventManager()->TrackEvent(
        std::make_unique<RequestAdapterEvent>(callbackInfo, adapter));
    if (!tracked) {
        return {futureIDInternal};
    }

    InstanceRequestAdapterCmd cmd;
    cmd.instanceId = GetWireId();
    cmd.future = {futureIDInternal};
    cmd.adapterObjectHandle = adapter->GetWireHandle();
    cmd.options = options;

    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

bool Client::DoInstanceRequestAdapterCallback(Instance* instance,
                                              WGPUFuture future,
                                              WGPURequestAdapterStatus status,
                                              const char* message,
                                              const WGPUAdapterProperties* properties,
                                              const WGPUSupportedLimits* limits,
                                              uint32_t featuresCount,
                                              const WGPUFeatureName* features) {
    // May have been deleted or recreated so this isn't an error.
    if (instance == nullptr) {
        return true;
    }
    return instance->OnRequestAdapterCallback(future, status, message, properties, limits,
                                              featuresCount, features);
}

bool Instance::OnRequestAdapterCallback(WGPUFuture future,
                                        WGPURequestAdapterStatus status,
                                        const char* message,
                                        const WGPUAdapterProperties* properties,
                                        const WGPUSupportedLimits* limits,
                                        uint32_t featuresCount,
                                        const WGPUFeatureName* features) {
    return GetClient()->GetEventManager()->SetFutureReady<RequestAdapterEvent>(
               future.id, status, message, properties, limits, featuresCount, features) ==
           WireResult::Success;
}

void Instance::ProcessEvents() {
    // TODO(crbug.com/dawn/2061): This should only process events for this Instance, not others
    // on the same client. When EventManager is moved to Instance, this can be fixed.
    GetClient()->GetEventManager()->ProcessPollEvents();

    // TODO(crbug.com/dawn/1987): The responsibility of ProcessEvents here is a bit mixed. It both
    // processes events coming in from the server, and also prompts the server to check for and
    // forward over new events - which won't be received until *after* this client-side
    // ProcessEvents completes.
    //
    // Fixing this nicely probably requires the server to more self-sufficiently
    // forward the events, which is half of making the wire fully invisible to use (which we might
    // like to do, someday, but not soon). This is easy for immediate events (like requestDevice)
    // and thread-driven events (async pipeline creation), but harder for queue fences where we have
    // to wait on the backend and then trigger Dawn code to forward the event.
    //
    // In the meantime, we could maybe do this on client->server flush to keep this concern in the
    // wire instead of in the API itself, but otherwise it's not significantly better so we just
    // keep it here for now for backward compatibility.
    InstanceProcessEventsCmd cmd;
    cmd.self = ToAPI(this);
    GetClient()->SerializeCommand(cmd);
}

WGPUWaitStatus Instance::WaitAny(size_t count, WGPUFutureWaitInfo* infos, uint64_t timeoutNS) {
    // In principle the EventManager should be on the Instance, not the Client.
    // But it's hard to get from an object to its Instance right now, so we can
    // store it on the Client.
    return GetClient()->GetEventManager()->WaitAny(count, infos, timeoutNS);
}

bool Instance::HasWGSLLanguageFeature(WGPUWGSLFeatureName feature) const {
    // TODO(dawn:2260): Implemented wgslLanguageFeatures on the wire.
    return false;
}

size_t Instance::EnumerateWGSLLanguageFeatures(WGPUWGSLFeatureName* features) const {
    // TODO(dawn:2260): Implemented wgslLanguageFeatures on the wire.
    return 0;
}

}  // namespace dawn::wire::client
