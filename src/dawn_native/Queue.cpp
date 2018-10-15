// Copyright 2017 The Dawn Authors
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

#include "dawn_native/Queue.h"

#include "dawn_native/CommandBuffer.h"
#include "dawn_native/Device.h"

namespace dawn_native {

    // QueueBase

    QueueBase::QueueBase(DeviceBase* device) : ObjectBase(device) {
    }

    void QueueBase::Submit(uint32_t numCommands, CommandBufferBase* const* commands) {
        if (GetDevice()->ConsumedError(ValidateSubmit(numCommands, commands))) {
            return;
        }

        SubmitImpl(numCommands, commands);
    }

    MaybeError QueueBase::ValidateSubmit(uint32_t, CommandBufferBase* const*) {
        return {};
    }

}  // namespace dawn_native
