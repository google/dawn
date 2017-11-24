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

#include "wire/WireCmd.h"

namespace nxt { namespace wire {

    size_t ReturnDeviceErrorCallbackCmd::GetRequiredSize() const {
        return sizeof(*this) + messageStrlen + 1;
    }

    char* ReturnDeviceErrorCallbackCmd::GetMessage() {
        return reinterpret_cast<char*>(this + 1);
    }

    const char* ReturnDeviceErrorCallbackCmd::GetMessage() const {
        return reinterpret_cast<const char*>(this + 1);
    }

    size_t BufferMapReadAsyncCmd::GetRequiredSize() const {
        return sizeof(*this);
    }

    size_t ReturnBufferMapReadAsyncCallbackCmd::GetRequiredSize() const {
        return sizeof(*this) + dataLength;
    }

    void* ReturnBufferMapReadAsyncCallbackCmd::GetData() {
        return this + 1;
    }

    const void* ReturnBufferMapReadAsyncCallbackCmd::GetData() const {
        return this + 1;
    }

}}  // namespace nxt::wire
