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

#include <algorithm>
#include <vector>

#include "dawn/wire/SupportedFeatures.h"
#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {

WireResult Server::DoInstanceRequestAdapter(Known<WGPUInstance> instance,
                                            uint64_t requestSerial,
                                            ObjectHandle adapterHandle,
                                            const WGPURequestAdapterOptions* options) {
    Known<WGPUAdapter> adapter;
    WIRE_TRY(AdapterObjects().Allocate(&adapter, adapterHandle, AllocationState::Reserved));

    auto userdata = MakeUserdata<RequestAdapterUserdata>();
    userdata->instance = instance.AsHandle();
    userdata->requestSerial = requestSerial;
    userdata->adapterObjectId = adapter.id;

    mProcs.instanceRequestAdapter(instance->handle, options,
                                  ForwardToServer<&Server::OnRequestAdapterCallback>,
                                  userdata.release());
    return WireResult::Success;
}

void Server::OnRequestAdapterCallback(RequestAdapterUserdata* data,
                                      WGPURequestAdapterStatus status,
                                      WGPUAdapter adapter,
                                      const char* message) {
    ReturnInstanceRequestAdapterCallbackCmd cmd = {};
    cmd.instance = data->instance;
    cmd.requestSerial = data->requestSerial;
    cmd.status = status;
    cmd.message = message;

    if (status != WGPURequestAdapterStatus_Success) {
        // Free the ObjectId which will make it unusable.
        AdapterObjects().Free(data->adapterObjectId);
        DAWN_ASSERT(adapter == nullptr);
        SerializeCommand(cmd);
        return;
    }

    // Assign the handle and allocated status if the adapter is created successfully.
    AdapterObjects().FillReservation(data->adapterObjectId, adapter);

    // Query and report the adapter supported features.
    std::vector<WGPUFeatureName> features;

    size_t featuresCount = mProcs.adapterEnumerateFeatures(adapter, nullptr);
    features.resize(featuresCount);
    mProcs.adapterEnumerateFeatures(adapter, features.data());

    // Hide features the wire cannot support.
    auto it = std::partition(features.begin(), features.end(), IsFeatureSupported);

    cmd.featuresCount = std::distance(features.begin(), it);
    cmd.features = features.data();

    // Query and report the adapter properties.
    WGPUAdapterProperties properties = {};
    mProcs.adapterGetProperties(adapter, &properties);
    cmd.properties = &properties;

    // Query and report the adapter limits, including DawnExperimentalSubgroupLimits.
    WGPUSupportedLimits limits = {};

    WGPUDawnExperimentalSubgroupLimits experimentalSubgroupLimits = {};
    experimentalSubgroupLimits.chain.sType = WGPUSType_DawnExperimentalSubgroupLimits;
    limits.nextInChain = &experimentalSubgroupLimits.chain;

    mProcs.adapterGetLimits(adapter, &limits);
    cmd.limits = &limits;

    SerializeCommand(cmd);
    mProcs.adapterPropertiesFreeMembers(properties);
}

}  // namespace dawn::wire::server
