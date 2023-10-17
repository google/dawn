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

    // Get lengths, with null terminators.
    size_t vendorNameCLen = strlen(mProperties.vendorName) + 1;
    size_t architectureCLen = strlen(mProperties.architecture) + 1;
    size_t nameCLen = strlen(mProperties.name) + 1;
    size_t driverDescriptionCLen = strlen(mProperties.driverDescription) + 1;

    // Allocate space for all strings.
    char* ptr = new char[vendorNameCLen + architectureCLen + nameCLen + driverDescriptionCLen];

    properties->vendorName = ptr;
    memcpy(ptr, mProperties.vendorName, vendorNameCLen);
    ptr += vendorNameCLen;

    properties->architecture = ptr;
    memcpy(ptr, mProperties.architecture, architectureCLen);
    ptr += architectureCLen;

    properties->name = ptr;
    memcpy(ptr, mProperties.name, nameCLen);
    ptr += nameCLen;

    properties->driverDescription = ptr;
    memcpy(ptr, mProperties.driverDescription, driverDescriptionCLen);
    ptr += driverDescriptionCLen;
}

void ClientAdapterPropertiesFreeMembers(WGPUAdapterProperties properties) {
    // This single delete is enough because everything is a single allocation.
    delete[] properties.vendorName;
}

void Adapter::RequestDevice(const WGPUDeviceDescriptor* descriptor,
                            WGPURequestDeviceCallback callback,
                            void* userdata) {
    Client* client = GetClient();
    if (client->IsDisconnected()) {
        callback(WGPURequestDeviceStatus_Error, nullptr, "GPU connection lost", userdata);
        return;
    }

    // The descriptor is passed so that the deviceLostCallback can be tracked client-side and called
    // when the device is lost.
    Device* device = client->Make<Device>(descriptor);
    uint64_t serial = mRequestDeviceRequests.Add({callback, device->GetWireId(), userdata});

    // Ensure the device lost callback isn't serialized as part of the command, as it cannot be
    // passed between processes.
    WGPUDeviceDescriptor wireDescriptor = {};
    if (descriptor) {
        wireDescriptor = *descriptor;
        wireDescriptor.deviceLostCallback = nullptr;
        wireDescriptor.deviceLostUserdata = nullptr;
    }

    AdapterRequestDeviceCmd cmd;
    cmd.adapterId = GetWireId();
    cmd.requestSerial = serial;
    cmd.deviceObjectHandle = device->GetWireHandle();
    cmd.descriptor = &wireDescriptor;

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

WGPUInstance Adapter::GetInstance() const {
    dawn::ErrorLog() << "adapter.GetInstance not supported with dawn_wire.";
    return nullptr;
}

WGPUDevice Adapter::CreateDevice(const WGPUDeviceDescriptor*) {
    dawn::ErrorLog() << "adapter.CreateDevice not supported with dawn_wire.";
    return nullptr;
}

}  // namespace dawn::wire::client
