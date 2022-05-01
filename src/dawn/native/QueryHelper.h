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

#ifndef SRC_DAWN_NATIVE_QUERYHELPER_H_
#define SRC_DAWN_NATIVE_QUERYHELPER_H_

#include "dawn/native/Error.h"
#include "dawn/native/ObjectBase.h"

namespace dawn::native {

class BufferBase;
class CommandEncoder;

struct TimestampParams {
    TimestampParams(uint32_t first, uint32_t count, uint32_t offset, float period);

    uint32_t first;
    uint32_t count;
    uint32_t offset;
    uint32_t multiplier;
    uint32_t rightShift;
};

MaybeError EncodeConvertTimestampsToNanoseconds(CommandEncoder* encoder,
                                                BufferBase* timestamps,
                                                BufferBase* availability,
                                                BufferBase* params);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_QUERYHELPER_H_
