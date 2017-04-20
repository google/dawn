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

#ifndef BACKEND_COMMON_QUEUE_H_
#define BACKEND_COMMON_QUEUE_H_

#include "Forward.h"
#include "RefCounted.h"

#include "nxt/nxtcpp.h"

namespace backend {

    class QueueBase : public RefCounted {
        private:
            bool ValidateSubmitCommand(CommandBufferBase* command);

        public:
            template<typename T>
            bool ValidateSubmit(uint32_t numCommands, T* const * commands) {
                static_assert(std::is_base_of<CommandBufferBase, T>::value, "invalid command buffer type");

                for (uint32_t i = 0; i < numCommands; ++i) {
                    if (!ValidateSubmitCommand(commands[i])) {
                        return false;
                    }
                }
                return true;
            }
    };

    class QueueBuilder : public RefCounted {
        public:
            QueueBuilder(DeviceBase* device);

            bool WasConsumed() const;

            // NXT API
            QueueBase* GetResult();

        private:
            DeviceBase* device;
            bool consumed = false;
    };

}

#endif // BACKEND_COMMON_QUEUE_H_
