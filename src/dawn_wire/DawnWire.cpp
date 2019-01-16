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

#include "dawn_wire/client/Client.h"
#include "dawn_wire/client/Device_autogen.h"

namespace dawn_wire {
    CommandHandler* NewClientDevice(dawnProcTable* procs,
                                    dawnDevice* device,
                                    CommandSerializer* serializer) {
        auto clientDevice = new client::Device(serializer);

        *device = reinterpret_cast<dawnDeviceImpl*>(clientDevice);
        *procs = client::GetProcs();

        return new client::Client(clientDevice);
    }
}  // namespace dawn_wire
