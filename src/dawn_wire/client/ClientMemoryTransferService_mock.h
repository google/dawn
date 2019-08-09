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
            bool DeserializeInitialData(const void* deserializePointer,
                                        size_t deserializeSize,
                                        const void** data,
                                        size_t* dataLength) override;

          private:
            MockMemoryTransferService* mService;
        };

        class MockWriteHandle : public WriteHandle {
          public:
            explicit MockWriteHandle(MockMemoryTransferService* service);
            ~MockWriteHandle() override;

            size_t SerializeCreateSize() override;
            void SerializeCreate(void* serializePointer) override;
            std::pair<void*, size_t> Open() override;
            size_t SerializeFlushSize() override;
            void SerializeFlush(void* serializePointer) override;

          private:
            MockMemoryTransferService* mService;
        };

        MockMemoryTransferService();
        ~MockMemoryTransferService() override;

        ReadHandle* CreateReadHandle(size_t) override;
        WriteHandle* CreateWriteHandle(size_t) override;

        MockReadHandle* NewReadHandle();
        MockWriteHandle* NewWriteHandle();

        MOCK_METHOD1(OnCreateReadHandle, ReadHandle*(size_t));
        MOCK_METHOD1(OnCreateWriteHandle, WriteHandle*(size_t));

        MOCK_METHOD1(OnReadHandleSerializeCreateSize, size_t(const ReadHandle*));
        MOCK_METHOD2(OnReadHandleSerializeCreate, void(const ReadHandle*, void* serializePointer));
        MOCK_METHOD5(OnReadHandleDeserializeInitialData,
                     bool(const ReadHandle*,
                          const uint32_t* deserializePointer,
                          size_t deserializeSize,
                          const void** data,
                          size_t* dataLength));
        MOCK_METHOD1(OnReadHandleDestroy, void(const ReadHandle*));

        MOCK_METHOD1(OnWriteHandleSerializeCreateSize, size_t(const void* WriteHandle));
        MOCK_METHOD2(OnWriteHandleSerializeCreate,
                     void(const void* WriteHandle, void* serializePointer));
        MOCK_METHOD1(OnWriteHandleOpen, std::pair<void*, size_t>(const void* WriteHandle));
        MOCK_METHOD1(OnWriteHandleSerializeFlushSize, size_t(const void* WriteHandle));
        MOCK_METHOD2(OnWriteHandleSerializeFlush,
                     void(const void* WriteHandle, void* serializePointer));
        MOCK_METHOD1(OnWriteHandleDestroy, void(const void* WriteHandle));
    };

}}  //  namespace dawn_wire::client

#endif  // DAWNWIRE_CLIENT_CLIENTMEMORYTRANSFERSERVICE_MOCK_H_
