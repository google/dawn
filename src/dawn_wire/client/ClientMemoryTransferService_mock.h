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

#ifndef DAWNWIRE_CLIENT_CLIENTMEMORYTRANSFERSERVICE_MOCK_H_
#define DAWNWIRE_CLIENT_CLIENTMEMORYTRANSFERSERVICE_MOCK_H_

#include <gmock/gmock.h>

#include "dawn_wire/WireClient.h"
#include "dawn_wire/client/Client.h"

namespace dawn_wire { namespace client {

    class MockMemoryTransferService : public MemoryTransferService {
      public:
        class MockReadHandle : public ReadHandle {
          public:
            explicit MockReadHandle(MockMemoryTransferService* service);
            ~MockReadHandle() override;

            size_t SerializeCreateSize() override;
            void SerializeCreate(void* serializePointer) override;
            const void* GetData() override;
            bool DeserializeDataUpdate(const void* deserializePointer,
                                       size_t deserializeSize,
                                       size_t offset,
                                       size_t size) override;

          private:
            MockMemoryTransferService* mService;
        };

        class MockWriteHandle : public WriteHandle {
          public:
            explicit MockWriteHandle(MockMemoryTransferService* service);
            ~MockWriteHandle() override;

            size_t SerializeCreateSize() override;
            void SerializeCreate(void* serializePointer) override;
            void* GetData() override;
            size_t SizeOfSerializeDataUpdate(size_t offset, size_t size) override;
            void SerializeDataUpdate(void* serializePointer, size_t offset, size_t size) override;

          private:
            MockMemoryTransferService* mService;
        };

        MockMemoryTransferService();
        ~MockMemoryTransferService() override;

        ReadHandle* CreateReadHandle(size_t) override;
        WriteHandle* CreateWriteHandle(size_t) override;

        MockReadHandle* NewReadHandle();
        MockWriteHandle* NewWriteHandle();

        MOCK_METHOD(ReadHandle*, OnCreateReadHandle, (size_t));
        MOCK_METHOD(WriteHandle*, OnCreateWriteHandle, (size_t));

        MOCK_METHOD(size_t, OnReadHandleSerializeCreateSize, (const ReadHandle*));
        MOCK_METHOD(void, OnReadHandleSerializeCreate, (const ReadHandle*, void* serializePointer));
        MOCK_METHOD((const void*), OnReadHandleGetData, (const ReadHandle*));
        MOCK_METHOD(bool,
                    OnReadHandleDeserializeDataUpdate,
                    (const ReadHandle*,
                     const uint32_t* deserializePointer,
                     size_t deserializeSize,
                     size_t offset,
                     size_t size));
        MOCK_METHOD(void, OnReadHandleDestroy, (const ReadHandle*));

        MOCK_METHOD(size_t, OnWriteHandleSerializeCreateSize, (const void* WriteHandle));
        MOCK_METHOD(void,
                    OnWriteHandleSerializeCreate,
                    (const void* WriteHandle, void* serializePointer));
        MOCK_METHOD((void*), OnWriteHandleGetData, (const void* WriteHandle));
        MOCK_METHOD(size_t,
                    OnWriteHandleSizeOfSerializeDataUpdate,
                    (const void* WriteHandle, size_t offset, size_t size));
        MOCK_METHOD(size_t,
                    OnWriteHandleSerializeDataUpdate,
                    (const void* WriteHandle, void* serializePointer, size_t offset, size_t size));
        MOCK_METHOD(void, OnWriteHandleDestroy, (const void* WriteHandle));
    };

}}  //  namespace dawn_wire::client

#endif  // DAWNWIRE_CLIENT_CLIENTMEMORYTRANSFERSERVICE_MOCK_H_
