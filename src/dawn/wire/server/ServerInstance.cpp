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

bool Server::DoInstanceRequestAdapter(ObjectId instanceId,
                                      uint64_t requestSerial,
                                      ObjectHandle adapterHandle,
                                      const WGPURequestAdapterOptions* options) {
    auto* instance = InstanceObjects().Get(instanceId);
    if (instance == nullptr) {
        return false;
    }

    auto* resultData = AdapterObjects().Allocate(adapterHandle, AllocationState::Reserved);
    if (resultData == nullptr) {
        return false;
    }

    resultData->generation = adapterHandle.generation;

    auto userdata = MakeUserdata<RequestAdapterUserdata>();
    userdata->instance = ObjectHandle{instanceId, instance->generation};
    userdata->requestSerial = requestSerial;
    userdata->adapterObjectId = adapterHandle.id;

    mProcs.instanceRequestAdapter(instance->handle, options,
                                  ForwardToServer<&Server::OnRequestAdapterCallback>,
                                  userdata.release());
    return true;
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
    auto* adapterObject = AdapterObjects().FillReservation(data->adapterObjectId, adapter);
    ASSERT(adapterObject != nullptr);

    size_t featuresCount = mProcs.adapterEnumerateFeatures(adapter, nullptr);
    features.resize(featuresCount);
    mProcs.adapterEnumerateFeatures(adapter, features.data());

    // Hide features the wire cannot support.
    auto it = std::partition(features.begin(), features.end(), IsFeatureSupported);

    cmd.featuresCount = static_cast<uint32_t>(std::distance(features.begin(), it));
    cmd.features = features.data();

    mProcs.adapterGetProperties(adapter, &properties);
    mProcs.adapterGetLimits(adapter, &limits);
    cmd.properties = &properties;
    cmd.limits = &limits;

    SerializeCommand(cmd);
}

}  // namespace dawn::wire::server
