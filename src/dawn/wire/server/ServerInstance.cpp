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
        ASSERT(adapter == nullptr);
        SerializeCommand(cmd);
        return;
    }

    WGPUAdapterProperties properties = {};
    WGPUSupportedLimits limits = {};
    std::vector<WGPUFeatureName> features;

    // Assign the handle and allocated status if the adapter is created successfully.
    AdapterObjects().FillReservation(data->adapterObjectId, adapter);

    size_t featuresCount = mProcs.adapterEnumerateFeatures(adapter, nullptr);
    features.resize(featuresCount);
    mProcs.adapterEnumerateFeatures(adapter, features.data());

    // Hide features the wire cannot support.
    auto it = std::partition(features.begin(), features.end(), IsFeatureSupported);

    cmd.featuresCount = std::distance(features.begin(), it);
    cmd.features = features.data();

    mProcs.adapterGetProperties(adapter, &properties);
    mProcs.adapterGetLimits(adapter, &limits);
    cmd.properties = &properties;
    cmd.limits = &limits;

    SerializeCommand(cmd);
    mProcs.adapterPropertiesFreeMembers(properties);
}

}  // namespace dawn::wire::server
