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

#include <vector>

#include "dawn/wire/SupportedFeatures.h"
#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {

WireResult Server::DoAdapterRequestDevice(Known<WGPUAdapter> adapter,
                                          uint64_t requestSerial,
                                          ObjectHandle deviceHandle,
                                          const WGPUDeviceDescriptor* descriptor) {
    Known<WGPUDevice> device;
    WIRE_TRY(DeviceObjects().Allocate(&device, deviceHandle, AllocationState::Reserved));

    auto userdata = MakeUserdata<RequestDeviceUserdata>();
    userdata->adapter = adapter.AsHandle();
    userdata->requestSerial = requestSerial;
    userdata->deviceObjectId = device.id;

    mProcs.adapterRequestDevice(adapter->handle, descriptor,
                                ForwardToServer<&Server::OnRequestDeviceCallback>,
                                userdata.release());
    return WireResult::Success;
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
        DAWN_ASSERT(device == nullptr);
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

    cmd.featuresCount = features.size();
    cmd.features = features.data();

    WGPUSupportedLimits limits = {};
    // Also query the DawnExperimentalSubgroupLimits and report to client.
    WGPUDawnExperimentalSubgroupLimits experimentalSubgroupLimits = {};
    experimentalSubgroupLimits.chain.sType = WGPUSType_DawnExperimentalSubgroupLimits;
    limits.nextInChain = &experimentalSubgroupLimits.chain;
    mProcs.deviceGetLimits(device, &limits);
    cmd.limits = &limits;

    // Assign the handle and allocated status if the device is created successfully.
    Known<WGPUDevice> reservation = DeviceObjects().FillReservation(data->deviceObjectId, device);
    DAWN_ASSERT(reservation.data != nullptr);
    reservation->info->server = this;
    reservation->info->self = reservation.AsHandle();
    SetForwardingDeviceCallbacks(reservation);

    SerializeCommand(cmd);
}

}  // namespace dawn::wire::server
