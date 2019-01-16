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

#ifndef DAWNWIRE_CLIENT_CLIENT_H_
#define DAWNWIRE_CLIENT_CLIENT_H_

#include <dawn_wire/Wire.h>

#include "dawn_wire/WireCmd_autogen.h"
#include "dawn_wire/WireDeserializeAllocator.h"

namespace dawn_wire { namespace client {

    class Device;

    class Client : public CommandHandler {
      public:
        Client(Device* device);
        const char* HandleCommands(const char* commands, size_t size);

      private:
#include "dawn_wire/client/ClientPrototypes_autogen.inl"

        Device* mDevice;
        WireDeserializeAllocator mAllocator;
    };

    dawnProcTable GetProcs();

}}  // namespace dawn_wire::client

#endif  // DAWNWIRE_CLIENT_CLIENT_H_
