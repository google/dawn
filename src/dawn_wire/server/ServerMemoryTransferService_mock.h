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

#ifndef DAWNWIRE_SERVER_SERVERMEMORYTRANSFERSERVICE_MOCK_H_
#define DAWNWIRE_SERVER_SERVERMEMORYTRANSFERSERVICE_MOCK_H_

#include <gmock/gmock.h>

#include "dawn_wire/WireServer.h"
#include "dawn_wire/server/Server.h"

namespace dawn_wire { namespace server {

    class MockMemoryTransferService : public MemoryTransferService {
      public:
        class MockReadHandle : public ReadHandle {
          public:
            MockReadHandle(MockMemoryTransferService* service);
            ~MockReadHandle() override;

            size_t SizeOfSerializeDataUpdate(size_t offset, size_t size) override;
            void SerializeDataUpdate(const void* data,
                                     size_t offset,
                                     size_t size,
                                     void* serializePointer) override;

          private:
            MockMemoryTransferService* mService;
        };

        class MockWriteHandle : public WriteHandle {
          public:
            MockWriteHandle(MockMemoryTransferService* service);
            ~MockWriteHandle() override;

            bool DeserializeDataUpdate(const void* deserializePointer,
                                       size_t deserializeSize,
                                       size_t offset,
                                       size_t size) override;

            const uint32_t* GetData() const;

          private:
            MockMemoryTransferService* mService;
        };

        MockMemoryTransferService();
        ~MockMemoryTransferService() override;

        bool DeserializeReadHandle(const void* deserializePointer,
                                   size_t deserializeSize,
                                   ReadHandle** readHandle) override;

        bool DeserializeWriteHandle(const void* deserializePointer,
                                    size_t deserializeSize,
                                    WriteHandle** writeHandle) override;

        MockReadHandle* NewReadHandle();
        MockWriteHandle* NewWriteHandle();

        MOCK_METHOD(bool,
                    OnDeserializeReadHandle,
                    (const uint32_t* deserializePointer,
                     size_t deserializeSize,
                     ReadHandle** readHandle));

        MOCK_METHOD(bool,
                    OnDeserializeWriteHandle,
                    (const uint32_t* deserializePointer,
                     size_t deserializeSize,
                     WriteHandle** writeHandle));

        MOCK_METHOD(size_t,
                    OnReadHandleSizeOfSerializeDataUpdate,
                    (const ReadHandle* readHandle, size_t offset, size_t size));
        MOCK_METHOD(void,
                    OnReadHandleSerializeDataUpdate,
                    (const ReadHandle* readHandle,
                     const void* data,
                     size_t offset,
                     size_t size,
                     void* serializePointer));
        MOCK_METHOD(void, OnReadHandleDestroy, (const ReadHandle* readHandle));

        MOCK_METHOD(bool,
                    OnWriteHandleDeserializeDataUpdate,
                    (const WriteHandle* writeHandle,
                     const uint32_t* deserializePointer,
                     size_t deserializeSize,
                     size_t offset,
                     size_t size));
        MOCK_METHOD(void, OnWriteHandleDestroy, (const WriteHandle* writeHandle));
    };

}}  //  namespace dawn_wire::server

#endif  // DAWNWIRE_SERVER_SERVERMEMORYTRANSFERSERVICE_MOCK_H_
