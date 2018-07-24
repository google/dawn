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

#ifndef BACKEND_QUEUE_H_
#define BACKEND_QUEUE_H_

#include "dawn_native/Builder.h"
#include "dawn_native/Error.h"
#include "dawn_native/Forward.h"
#include "dawn_native/RefCounted.h"

#include "dawn/dawncpp.h"

namespace backend {

    class QueueBase : public RefCounted {
      public:
        QueueBase(DeviceBase* device);

        DeviceBase* GetDevice();

        template <typename T>
        MaybeError ValidateSubmit(uint32_t numCommands, T* const* commands) {
            static_assert(std::is_base_of<CommandBufferBase, T>::value,
                          "invalid command buffer type");

            for (uint32_t i = 0; i < numCommands; ++i) {
                DAWN_TRY(ValidateSubmitCommand(commands[i]));
            }
            return {};
        }

      private:
        MaybeError ValidateSubmitCommand(CommandBufferBase* command);

        DeviceBase* mDevice;
    };

}  // namespace backend

#endif  // BACKEND_QUEUE_H_
