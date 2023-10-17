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

#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/EventManager.h"

namespace dawn::wire::client {

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

Instance::~Instance() {
    mRequestAdapterRequests.CloseAll([](RequestAdapterData* request) {
        request->callback(WGPURequestAdapterStatus_Unknown, nullptr,
                          "Instance destroyed before callback", request->userdata);
    });
}

void Instance::CancelCallbacksForDisconnect() {
    mRequestAdapterRequests.CloseAll([](RequestAdapterData* request) {
        request->callback(WGPURequestAdapterStatus_Unknown, nullptr, "GPU connection lost",
                          request->userdata);
    });
}

void Instance::RequestAdapter(const WGPURequestAdapterOptions* options,
                              WGPURequestAdapterCallback callback,
                              void* userdata) {
    Client* client = GetClient();
    if (client->IsDisconnected()) {
        callback(WGPURequestAdapterStatus_Error, nullptr, "GPU connection lost", userdata);
        return;
    }

    Adapter* adapter = client->Make<Adapter>();
    uint64_t serial = mRequestAdapterRequests.Add({callback, adapter->GetWireId(), userdata});

    InstanceRequestAdapterCmd cmd;
    cmd.instanceId = GetWireId();
    cmd.requestSerial = serial;
    cmd.adapterObjectHandle = adapter->GetWireHandle();
    cmd.options = options;

    client->SerializeCommand(cmd);
}

bool Client::DoInstanceRequestAdapterCallback(Instance* instance,
                                              uint64_t requestSerial,
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
    return instance->OnRequestAdapterCallback(requestSerial, status, message, properties, limits,
                                              featuresCount, features);
}

bool Instance::OnRequestAdapterCallback(uint64_t requestSerial,
                                        WGPURequestAdapterStatus status,
                                        const char* message,
                                        const WGPUAdapterProperties* properties,
                                        const WGPUSupportedLimits* limits,
                                        uint32_t featuresCount,
                                        const WGPUFeatureName* features) {
    RequestAdapterData request;
    if (!mRequestAdapterRequests.Acquire(requestSerial, &request)) {
        return false;
    }

    Client* client = GetClient();
    Adapter* adapter = client->Get<Adapter>(request.adapterObjectId);

    // If the return status is a failure we should give a null adapter to the callback and
    // free the allocation.
    if (status != WGPURequestAdapterStatus_Success) {
        client->Free(adapter);
        request.callback(status, nullptr, message, request.userdata);
        return true;
    }

    adapter->SetProperties(properties);
    adapter->SetLimits(limits);
    adapter->SetFeatures(features, featuresCount);

    request.callback(status, ToAPI(adapter), message, request.userdata);
    return true;
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

}  // namespace dawn::wire::client
