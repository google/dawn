// Copyright 2022 The Dawn Authors
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

#include "dawn/wire/client/QuerySet.h"

#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/Device.h"

namespace dawn::wire::client {

// static
WGPUQuerySet QuerySet::Create(Device* device, const WGPUQuerySetDescriptor* descriptor) {
    Client* wireClient = device->GetClient();
    QuerySet* querySet = wireClient->Make<QuerySet>(descriptor);

    // Send the Device::CreateQuerySet command without modifications.
    DeviceCreateQuerySetCmd cmd;
    cmd.self = ToAPI(device);
    cmd.selfId = device->GetWireId();
    cmd.descriptor = descriptor;
    cmd.result = querySet->GetWireHandle();
    wireClient->SerializeCommand(cmd);

    return ToAPI(querySet);
}

QuerySet::QuerySet(const ObjectBaseParams& params, const WGPUQuerySetDescriptor* descriptor)
    : ObjectBase(params), mType(descriptor->type), mCount(descriptor->count) {}

QuerySet::~QuerySet() = default;

WGPUQueryType QuerySet::GetType() const {
    return mType;
}

uint32_t QuerySet::GetCount() const {
    return mCount;
}

}  // namespace dawn::wire::client
