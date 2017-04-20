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

#ifndef WIRE_WIRE_H_
#define WIRE_WIRE_H_

#include <cstdint>

#include "nxt/nxt.h"

namespace nxt {
namespace wire {

    class CommandSerializer {
        public:
            virtual ~CommandSerializer() = default;
            virtual void* GetCmdSpace(size_t size) = 0;
            virtual void Flush() = 0;
    };

    class CommandHandler {
        public:
            virtual ~CommandHandler() = default;
            virtual const uint8_t* HandleCommands(const uint8_t* commands, size_t size) = 0;
    };

    CommandHandler* NewClientDevice(nxtProcTable* procs, nxtDevice* device, CommandSerializer* serializer);
    CommandHandler* NewServerCommandHandler(nxtDevice device, const nxtProcTable& procs, CommandSerializer* serializer);

}
}

#endif // WIRE_WIRE_H_
