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
#include "dawn/wire/WireResult.h"
#include "dawn/wire/server/ObjectStorage.h"
#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {

WireResult Server::DoAdapterRequestDevice(Known<WGPUAdapter> adapter,
                                          ObjectHandle eventManager,
                                          WGPUFuture future,
                                          ObjectHandle deviceHandle,
                                          WGPUFuture deviceLostFuture,
                                          const WGPUDeviceDescriptor* descriptor,
                                          uint8_t userdataCount) {
    Reserved<WGPUDevice> device;
    WIRE_TRY(Objects<WGPUDevice>().Allocate(&device, deviceHandle, AllocationState::Reserved));

    auto userdata = MakeUserdata<RequestDeviceUserdata>();
    userdata->eventManager = eventManager;
    userdata->future = future;
    userdata->deviceObjectId = device.id;
    userdata->deviceLostFuture = deviceLostFuture;

    // Update the descriptor with the device lost callback associated with this request.
    auto deviceLostUserdata = MakeUserdata<DeviceLostUserdata>();
    deviceLostUserdata->eventManager = eventManager;
    deviceLostUserdata->future = deviceLostFuture;

    WGPUDeviceDescriptor desc = *descriptor;
    desc.deviceLostCallbackInfo2 = {nullptr, WGPUCallbackMode_AllowProcessEvents,
                                    ForwardToServer2<&Server::OnDeviceLost>,
                                    deviceLostUserdata.release(), nullptr};
    desc.uncapturedErrorCallbackInfo2 = {
        nullptr,
        [](WGPUDevice const*, WGPUErrorType type, const char* message, void*, void* userdata) {
            DeviceInfo* info = static_cast<DeviceInfo*>(userdata);
            info->server->OnUncapturedError(info->self, type, message);
        },
        nullptr, device->info.get()};

    if (userdataCount == 1) {
        mProcs.adapterRequestDevice(adapter->handle, &desc,
                                    ForwardToServer<&Server::OnRequestDeviceCallback>,
                                    userdata.release());
    } else {
        mProcs.adapterRequestDevice2(
            adapter->handle, &desc,
            {nullptr, WGPUCallbackMode_AllowSpontaneous,
             ForwardToServer2<&Server::OnRequestDeviceCallback>, userdata.release(), nullptr});
    }
    return WireResult::Success;
}

void Server::OnRequestDeviceCallback(RequestDeviceUserdata* data,
                                     WGPURequestDeviceStatus status,
                                     WGPUDevice device,
                                     const char* message) {
    ReturnAdapterRequestDeviceCallbackCmd cmd = {};
    cmd.eventManager = data->eventManager;
    cmd.future = data->future;
    cmd.status = status;
    cmd.message = message;

    if (status != WGPURequestDeviceStatus_Success) {
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
            device = nullptr;

            cmd.status = WGPURequestDeviceStatus_Error;
            cmd.message = "Requested feature not supported.";
            SerializeCommand(cmd);
            return;
        }
    }

    cmd.featuresCount = features.size();
    cmd.features = features.data();

    // Query and report the adapter limits, including DawnExperimentalSubgroupLimits and
    // DawnExperimentalImmediateDataLimits. Reporting to client.
    WGPUSupportedLimits limits = {};

    // Chained DawnExperimentalSubgroupLimits.
    WGPUDawnExperimentalSubgroupLimits experimentalSubgroupLimits = {};
    experimentalSubgroupLimits.chain.sType = WGPUSType_DawnExperimentalSubgroupLimits;
    limits.nextInChain = &experimentalSubgroupLimits.chain;

    // Chained DawnExperimentalImmediateDataLimits.
    WGPUDawnExperimentalImmediateDataLimits experimentalImmediateDataLimits = {};
    experimentalImmediateDataLimits.chain.sType = WGPUSType_DawnExperimentalImmediateDataLimits;
    experimentalSubgroupLimits.chain.next = &experimentalImmediateDataLimits.chain;

    mProcs.deviceGetLimits(device, &limits);
    cmd.limits = &limits;

    // Assign the handle and allocated status if the device is created successfully.
    Known<WGPUDevice> reservation;
    if (FillReservation(data->deviceObjectId, device, &reservation) == WireResult::FatalError) {
        cmd.status = WGPURequestDeviceStatus_Unknown;
        cmd.message = "Destroyed before request was fulfilled.";
        SerializeCommand(cmd);
        return;
    }
    DAWN_ASSERT(reservation.data != nullptr);
    reservation->info->server = this;
    reservation->info->self = reservation.AsHandle();
    SetForwardingDeviceCallbacks(reservation);
    SerializeCommand(cmd);
}

}  // namespace dawn::wire::server
