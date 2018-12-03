// Copyright 2017 The Dawn Authors
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

#ifndef DAWNWIRE_WIRECMD_H_
#define DAWNWIRE_WIRECMD_H_

#include <dawn/dawn.h>

#include "dawn_wire/WireCmd_autogen.h"

namespace dawn_wire {

    struct ReturnDeviceErrorCallbackCmd {
        ReturnWireCmd commandId = ReturnWireCmd::DeviceErrorCallback;

        size_t messageStrlen;
    };

    struct BufferMapAsyncCmd {
        WireCmd commandId = WireCmd::BufferMapAsync;

        ObjectId bufferId;
        ObjectSerial requestSerial;
        uint32_t start;
        uint32_t size;
        bool isWrite;
    };

    struct ReturnBufferMapReadAsyncCallbackCmd {
        ReturnWireCmd commandId = ReturnWireCmd::BufferMapReadAsyncCallback;

        ObjectId bufferId;
        ObjectSerial bufferSerial;
        uint32_t requestSerial;
        uint32_t status;
        uint32_t dataLength;
    };

    struct ReturnBufferMapWriteAsyncCallbackCmd {
        ReturnWireCmd commandId = ReturnWireCmd::BufferMapWriteAsyncCallback;

        ObjectId bufferId;
        ObjectSerial bufferSerial;
        uint32_t requestSerial;
        uint32_t status;
    };

    struct ReturnFenceUpdateCompletedValueCmd {
        ReturnWireCmd commandId = ReturnWireCmd::FenceUpdateCompletedValue;

        ObjectId fenceId;
        ObjectSerial fenceSerial;
        uint64_t value;
    };

    struct BufferUpdateMappedDataCmd {
        WireCmd commandId = WireCmd::BufferUpdateMappedDataCmd;

        ObjectId bufferId;
        uint32_t dataLength;
    };

}  // namespace dawn_wire

#endif  // DAWNWIRE_WIRECMD_H_
