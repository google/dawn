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

#ifndef SRC_DAWN_WIRE_SERVER_SERVERMEMORYTRANSFERSERVICE_MOCK_H_
#define SRC_DAWN_WIRE_SERVER_SERVERMEMORYTRANSFERSERVICE_MOCK_H_

#include <gmock/gmock.h>

#include "dawn/wire/WireServer.h"
#include "dawn/wire/server/Server.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire::server {

class MockMemoryTransferService : public MemoryTransferService {
  public:
    class MockReadHandle : public ReadHandle {
      public:
        explicit MockReadHandle(MockMemoryTransferService* service);
        ~MockReadHandle() override;

        size_t SizeOfSerializeDataUpdate(size_t offset, size_t size) override;
        void SerializeDataUpdate(const void* data,
                                 size_t offset,
                                 size_t size,
                                 void* serializePointer) override;

      private:
        raw_ptr<MockMemoryTransferService> mService;
    };

    class MockWriteHandle : public WriteHandle {
      public:
        explicit MockWriteHandle(MockMemoryTransferService* service);
        ~MockWriteHandle() override;

        bool DeserializeDataUpdate(const void* deserializePointer,
                                   size_t deserializeSize,
                                   size_t offset,
                                   size_t size) override;

        const uint32_t* GetData() const;

      private:
        raw_ptr<MockMemoryTransferService> mService;
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

}  // namespace dawn::wire::server

#endif  // SRC_DAWN_WIRE_SERVER_SERVERMEMORYTRANSFERSERVICE_MOCK_H_
