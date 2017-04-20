// Copyright 2017 The NXT Authors
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

#include "Queue.h"

#include "Device.h"
#include "CommandBuffer.h"

namespace backend {

    // QueueBase

    bool QueueBase::ValidateSubmitCommand(CommandBufferBase* command) {
        return command->ValidateResourceUsagesImmediate();
    }

    // QueueBuilder

    QueueBuilder::QueueBuilder(DeviceBase* device) : device(device) {
    }

    bool QueueBuilder::WasConsumed() const {
        return consumed;
    }

    QueueBase* QueueBuilder::GetResult() {
        consumed = true;
        return device->CreateQueue(this);
    }

}
