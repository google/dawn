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

#include "dawn_wire/client/Buffer.h"

namespace dawn_wire { namespace client {

    Buffer::~Buffer() {
        // Callbacks need to be fired in all cases, as they can handle freeing resources
        // so we call them with "Unknown" status.
        ClearMapRequests(WGPUBufferMapAsyncStatus_Unknown);
    }

    void Buffer::ClearMapRequests(WGPUBufferMapAsyncStatus status) {
        for (auto& it : requests) {
            if (it.second.writeHandle) {
                it.second.writeCallback(status, nullptr, 0, it.second.userdata);
            } else {
                it.second.readCallback(status, nullptr, 0, it.second.userdata);
            }
        }
        requests.clear();
    }

}}  // namespace dawn_wire::client
