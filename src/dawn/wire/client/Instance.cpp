// Copyright 2021 The Dawn Authors
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

#include "dawn/wire/client/Instance.h"

#include "dawn/wire/client/Client.h"

namespace dawn::wire::client {

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

}  // namespace dawn::wire::client
