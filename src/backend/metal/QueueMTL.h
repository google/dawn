// Copyright 2018 The Dawn Authors
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

#ifndef BACKEND_METAL_QUEUEMTL_H_
#define BACKEND_METAL_QUEUEMTL_H_

#include "backend/Queue.h"

namespace backend { namespace metal {

    class CommandBuffer;
    class Device;

    class Queue : public QueueBase {
      public:
        Queue(Device* device);

        // Dawn API
        void Submit(uint32_t numCommands, CommandBuffer* const* commands);
    };

}}  // namespace backend::metal

#endif  // BACKEND_METAL_QUEUEMTL_H_
