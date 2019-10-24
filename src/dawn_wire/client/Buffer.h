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

#ifndef DAWNWIRE_CLIENT_BUFFER_H_
#define DAWNWIRE_CLIENT_BUFFER_H_

#include <dawn/webgpu.h>

#include "dawn_wire/WireClient.h"
#include "dawn_wire/client/ObjectBase.h"

#include <map>

namespace dawn_wire { namespace client {

    struct Buffer : ObjectBase {
        using ObjectBase::ObjectBase;

        ~Buffer();
        void ClearMapRequests(WGPUBufferMapAsyncStatus status);

        // We want to defer all the validation to the server, which means we could have multiple
        // map request in flight at a single time and need to track them separately.
        // On well-behaved applications, only one request should exist at a single time.
        struct MapRequestData {
            // TODO(enga): Use a tagged pointer to save space.
            WGPUBufferMapReadCallback readCallback = nullptr;
            WGPUBufferMapWriteCallback writeCallback = nullptr;
            void* userdata = nullptr;
            // TODO(enga): Use a tagged pointer to save space.
            std::unique_ptr<MemoryTransferService::ReadHandle> readHandle = nullptr;
            std::unique_ptr<MemoryTransferService::WriteHandle> writeHandle = nullptr;
        };
        std::map<uint32_t, MapRequestData> requests;
        uint32_t requestSerial = 0;
        uint64_t size = 0;

        // Only one mapped pointer can be active at a time because Unmap clears all the in-flight
        // requests.
        // TODO(enga): Use a tagged pointer to save space.
        std::unique_ptr<MemoryTransferService::ReadHandle> readHandle = nullptr;
        std::unique_ptr<MemoryTransferService::WriteHandle> writeHandle = nullptr;
    };

}}  // namespace dawn_wire::client

#endif  // DAWNWIRE_CLIENT_BUFFER_H_
