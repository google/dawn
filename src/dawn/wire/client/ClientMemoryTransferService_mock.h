// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_WIRE_CLIENT_CLIENTMEMORYTRANSFERSERVICE_MOCK_H_
#define SRC_DAWN_WIRE_CLIENT_CLIENTMEMORYTRANSFERSERVICE_MOCK_H_

#include <gmock/gmock.h>

#include "dawn/wire/WireClient.h"
#include "dawn/wire/client/Client.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire::client {

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
        raw_ptr<MockMemoryTransferService> mService;
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
        raw_ptr<MockMemoryTransferService> mService;
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

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_CLIENTMEMORYTRANSFERSERVICE_MOCK_H_
