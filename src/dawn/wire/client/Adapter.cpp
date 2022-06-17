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

#include "dawn/wire/client/Adapter.h"

#include "dawn/common/Log.h"
#include "dawn/wire/client/Client.h"

namespace dawn::wire::client {

Adapter::~Adapter() {
    mRequestDeviceRequests.CloseAll([](RequestDeviceData* request) {
        request->callback(WGPURequestDeviceStatus_Unknown, nullptr,
                          "Adapter destroyed before callback", request->userdata);
    });
}

void Adapter::CancelCallbacksForDisconnect() {
    mRequestDeviceRequests.CloseAll([](RequestDeviceData* request) {
        request->callback(WGPURequestDeviceStatus_Unknown, nullptr, "GPU connection lost",
                          request->userdata);
    });
}

bool Adapter::GetLimits(WGPUSupportedLimits* limits) const {
    return mLimitsAndFeatures.GetLimits(limits);
}

bool Adapter::HasFeature(WGPUFeatureName feature) const {
    return mLimitsAndFeatures.HasFeature(feature);
}

size_t Adapter::EnumerateFeatures(WGPUFeatureName* features) const {
    return mLimitsAndFeatures.EnumerateFeatures(features);
}

void Adapter::SetLimits(const WGPUSupportedLimits* limits) {
    return mLimitsAndFeatures.SetLimits(limits);
}

void Adapter::SetFeatures(const WGPUFeatureName* features, uint32_t featuresCount) {
    return mLimitsAndFeatures.SetFeatures(features, featuresCount);
}

void Adapter::SetProperties(const WGPUAdapterProperties* properties) {
    mProperties = *properties;
    mProperties.nextInChain = nullptr;
}

void Adapter::GetProperties(WGPUAdapterProperties* properties) const {
    *properties = mProperties;
}

void Adapter::RequestDevice(const WGPUDeviceDescriptor* descriptor,
                            WGPURequestDeviceCallback callback,
                            void* userdata) {
    Client* client = GetClient();
    if (client->IsDisconnected()) {
        callback(WGPURequestDeviceStatus_Error, nullptr, "GPU connection lost", userdata);
        return;
    }

    Device* device = client->Make<Device>();
    uint64_t serial = mRequestDeviceRequests.Add({callback, device->GetWireId(), userdata});

    AdapterRequestDeviceCmd cmd;
    cmd.adapterId = GetWireId();
    cmd.requestSerial = serial;
    cmd.deviceObjectHandle = device->GetWireHandle();
    cmd.descriptor = descriptor;

    client->SerializeCommand(cmd);
}

bool Client::DoAdapterRequestDeviceCallback(Adapter* adapter,
                                            uint64_t requestSerial,
                                            WGPURequestDeviceStatus status,
                                            const char* message,
                                            const WGPUSupportedLimits* limits,
                                            uint32_t featuresCount,
                                            const WGPUFeatureName* features) {
    // May have been deleted or recreated so this isn't an error.
    if (adapter == nullptr) {
        return true;
    }
    return adapter->OnRequestDeviceCallback(requestSerial, status, message, limits, featuresCount,
                                            features);
}

bool Adapter::OnRequestDeviceCallback(uint64_t requestSerial,
                                      WGPURequestDeviceStatus status,
                                      const char* message,
                                      const WGPUSupportedLimits* limits,
                                      uint32_t featuresCount,
                                      const WGPUFeatureName* features) {
    RequestDeviceData request;
    if (!mRequestDeviceRequests.Acquire(requestSerial, &request)) {
        return false;
    }

    Client* client = GetClient();
    Device* device = client->Get<Device>(request.deviceObjectId);

    // If the return status is a failure we should give a null device to the callback and
    // free the allocation.
    if (status != WGPURequestDeviceStatus_Success) {
        client->Free(device);
        request.callback(status, nullptr, message, request.userdata);
        return true;
    }

    device->SetLimits(limits);
    device->SetFeatures(features, featuresCount);

    request.callback(status, ToAPI(device), message, request.userdata);
    return true;
}

WGPUDevice Adapter::CreateDevice(const WGPUDeviceDescriptor*) {
    dawn::ErrorLog() << "adapter.CreateDevice not supported with dawn_wire.";
    return nullptr;
}

}  // namespace dawn::wire::client
