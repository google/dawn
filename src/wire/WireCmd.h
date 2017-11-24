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

#ifndef WIRE_WIRECMD_H_
#define WIRE_WIRECMD_H_

#include "wire/WireCmd_autogen.h"

namespace nxt { namespace wire {

    struct ReturnDeviceErrorCallbackCmd {
        wire::ReturnWireCmd commandId = ReturnWireCmd::DeviceErrorCallback;

        size_t messageStrlen;

        size_t GetRequiredSize() const;
        char* GetMessage();
        const char* GetMessage() const;
    };

    struct BufferMapReadAsyncCmd {
        wire::WireCmd commandId = WireCmd::BufferMapReadAsync;

        uint32_t bufferId;
        uint32_t requestSerial;
        uint32_t start;
        uint32_t size;

        size_t GetRequiredSize() const;
    };

    struct ReturnBufferMapReadAsyncCallbackCmd {
        wire::ReturnWireCmd commandId = ReturnWireCmd::BufferMapReadAsyncCallback;

        uint32_t bufferId;
        uint32_t bufferSerial;
        uint32_t requestSerial;
        uint32_t status;
        uint32_t dataLength;

        size_t GetRequiredSize() const;
        void* GetData();
        const void* GetData() const;
    };

}}  // namespace nxt::wire

#endif  // WIRE_WIRECMD_H_
