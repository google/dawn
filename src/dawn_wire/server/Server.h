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

#ifndef DAWNWIRE_SERVER_SERVER_H_
#define DAWNWIRE_SERVER_SERVER_H_

#include "dawn_wire/server/ServerBase_autogen.h"

namespace dawn_wire { namespace server {

    class Server;

    struct MapUserdata {
        Server* server;
        ObjectHandle buffer;
        uint32_t requestSerial;
        uint32_t size;
        bool isWrite;
    };

    struct FenceCompletionUserdata {
        Server* server;
        ObjectHandle fence;
        uint64_t value;
    };

    class Server : public ServerBase {
      public:
        Server(dawnDevice device, const dawnProcTable& procs, CommandSerializer* serializer);
        ~Server();

        const char* HandleCommands(const char* commands, size_t size);

      private:
        void* GetCmdSpace(size_t size);

        // Forwarding callbacks
        static void ForwardDeviceError(const char* message, dawnCallbackUserdata userdata);
        static void ForwardBufferMapReadAsync(dawnBufferMapAsyncStatus status,
                                              const void* ptr,
                                              uint32_t dataLength,
                                              dawnCallbackUserdata userdata);
        static void ForwardBufferMapWriteAsync(dawnBufferMapAsyncStatus status,
                                               void* ptr,
                                               uint32_t dataLength,
                                               dawnCallbackUserdata userdata);
        static void ForwardFenceCompletedValue(dawnFenceCompletionStatus status,
                                               dawnCallbackUserdata userdata);

        // Error callbacks
        void OnDeviceError(const char* message);
        void OnBufferMapReadAsyncCallback(dawnBufferMapAsyncStatus status,
                                          const void* ptr,
                                          uint32_t dataLength,
                                          MapUserdata* userdata);
        void OnBufferMapWriteAsyncCallback(dawnBufferMapAsyncStatus status,
                                           void* ptr,
                                           uint32_t dataLength,
                                           MapUserdata* userdata);
        void OnFenceCompletedValueUpdated(FenceCompletionUserdata* userdata);

#include "dawn_wire/server/ServerPrototypes_autogen.inl"

        CommandSerializer* mSerializer = nullptr;
        WireDeserializeAllocator mAllocator;
        dawnProcTable mProcs;
    };

}}  // namespace dawn_wire::server

#endif  // DAWNWIRE_SERVER_SERVER_H_
