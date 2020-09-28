// Copyright 2020 The Dawn Authors
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

#include "dawn_native/MapRequestTracker.h"

#include "dawn_native/Buffer.h"
#include "dawn_native/Device.h"

namespace dawn_native {

    MapRequestTracker::MapRequestTracker(DeviceBase* device) : mDevice(device) {
    }

    MapRequestTracker::~MapRequestTracker() {
        ASSERT(mInflightRequests.Empty());
    }

    void MapRequestTracker::Track(BufferBase* buffer, MapRequestID mapID) {
        Request request;
        request.buffer = buffer;
        request.id = mapID;

        mInflightRequests.Enqueue(std::move(request), mDevice->GetPendingCommandSerial());
        mDevice->AddFutureCallbackSerial(mDevice->GetPendingCommandSerial());
    }

    void MapRequestTracker::Tick(ExecutionSerial finishedSerial) {
        for (auto& request : mInflightRequests.IterateUpTo(finishedSerial)) {
            request.buffer->OnMapRequestCompleted(request.id);
        }
        mInflightRequests.ClearUpTo(finishedSerial);
    }
}  // namespace dawn_native
