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

#ifndef DAWNNATIVE_QUERYHELPER_H_
#define DAWNNATIVE_QUERYHELPER_H_

#include "dawn_native/ObjectBase.h"

namespace dawn_native {

    class BufferBase;
    class DeviceBase;
    class CommandEncoder;

    struct TimestampParams {
        uint32_t count;
        uint32_t offset;
        float period;
    };

    void EncodeConvertTimestampsToNanoseconds(CommandEncoder* encoder,
                                              BufferBase* timestamps,
                                              BufferBase* availability,
                                              BufferBase* params);

}  // namespace dawn_native

#endif  // DAWNNATIVE_QUERYHELPER_H_
