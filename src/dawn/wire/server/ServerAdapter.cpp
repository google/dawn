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

#include <vector>

#include "dawn/wire/SupportedFeatures.h"
#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {

bool Server::DoAdapterRequestDevice(ObjectId adapterId,
                                    uint64_t requestSerial,
                                    ObjectHandle deviceHandle,
                                    const WGPUDeviceDescriptor* descriptor) {
    auto* adapter = AdapterObjects().Get(adapterId);
    if (adapter == nullptr) {
        return false;
    }

    auto* resultData = DeviceObjects().Allocate(deviceHandle, AllocationState::Reserved);
    if (resultData == nullptr) {
        return false;
    }

    resultData->generation = deviceHandle.generation;

    auto userdata = MakeUserdata<RequestDeviceUserdata>();
    userdata->adapter = ObjectHandle{adapterId, adapter->generation};
    userdata->requestSerial = requestSerial;
    userdata->deviceObjectId = deviceHandle.id;

    mProcs.adapterRequestDevice(adapter->handle, descriptor,
                                ForwardToServer<&Server::OnRequestDeviceCallback>,
                                userdata.release());
    return true;
}

void Server::OnRequestDeviceCallback(RequestDeviceUserdata* data,
                                     WGPURequestDeviceStatus status,
                                     WGPUDevice device,
                                     const char* message) {
    ReturnAdapterRequestDeviceCallbackCmd cmd = {};
    cmd.adapter = data->adapter;
    cmd.requestSerial = data->requestSerial;
    cmd.status = status;
    cmd.message = message;

    if (status != WGPURequestDeviceStatus_Success) {
        // Free the ObjectId which will make it unusable.
        DeviceObjects().Free(data->deviceObjectId);
        ASSERT(device == nullptr);
        SerializeCommand(cmd);
        return;
    }

    std::vector<WGPUFeatureName> features;

    size_t featuresCount = mProcs.deviceEnumerateFeatures(device, nullptr);
    features.resize(featuresCount);
    mProcs.deviceEnumerateFeatures(device, features.data());

    // The client should only be able to request supported features, so all enumerated
    // features that were enabled must also be supported by the wire.
    // Note: We fail the callback here, instead of immediately upon receiving
    // the request to preserve callback ordering.
    for (WGPUFeatureName f : features) {
        if (!IsFeatureSupported(f)) {
            // Release the device.
            mProcs.deviceRelease(device);
            // Free the ObjectId which will make it unusable.
            DeviceObjects().Free(data->deviceObjectId);

            cmd.status = WGPURequestDeviceStatus_Error;
            cmd.message = "Requested feature not supported.";
            SerializeCommand(cmd);
            return;
        }
    }

    cmd.featuresCount = static_cast<uint32_t>(features.size());
    cmd.features = features.data();

    WGPUSupportedLimits limits = {};
    mProcs.deviceGetLimits(device, &limits);
    cmd.limits = &limits;

    // Assign the handle and allocated status if the device is created successfully.
    auto* deviceObject = DeviceObjects().FillReservation(data->deviceObjectId, device);
    ASSERT(deviceObject != nullptr);
    deviceObject->info->server = this;
    deviceObject->info->self = ObjectHandle{data->deviceObjectId, deviceObject->generation};
    SetForwardingDeviceCallbacks(deviceObject);

    SerializeCommand(cmd);
}

}  // namespace dawn::wire::server
