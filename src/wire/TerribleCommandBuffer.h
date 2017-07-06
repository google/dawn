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

#ifndef WIRE_TERRIBLE_COMMAND_BUFFER_H_
#define WIRE_TERRIBLE_COMMAND_BUFFER_H_

#include <vector>

#include "wire/Wire.h"

namespace nxt {
namespace wire {

class TerribleCommandBuffer : public CommandSerializer {
    public:
        TerribleCommandBuffer();
        TerribleCommandBuffer(CommandHandler* handler);

        void SetHandler(CommandHandler* handler);

        void* GetCmdSpace(size_t size) override;
        void Flush() override;

    private:
        CommandHandler* handler = nullptr;
        size_t offset = 0;
        uint8_t buffer[10000000];
};

}
}

#endif // WIRE_TERRIBLE_COMMAND_BUFFER_H_
