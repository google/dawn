// Copyright 2019 The Dawn Authors
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

#include "dawn_wire/server/Server.h"

namespace dawn_wire { namespace server {

    Server::Server(dawnDevice device, const dawnProcTable& procs, CommandSerializer* serializer)
        : mSerializer(serializer), mProcs(procs) {
        // The client-server knowledge is bootstrapped with device 1.
        auto* deviceData = DeviceObjects().Allocate(1);
        deviceData->handle = device;
        deviceData->valid = true;

        auto userdata = static_cast<dawnCallbackUserdata>(reinterpret_cast<intptr_t>(this));
        procs.deviceSetErrorCallback(device, ForwardDeviceErrorToServer, userdata);
    }

    Server::~Server() {
        DestroyAllObjects(mProcs);
    }

    void* Server::GetCmdSpace(size_t size) {
        return mSerializer->GetCmdSpace(size);
    }

}}  // namespace dawn_wire::server
