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

#include "dawn/wire/client/ClientMemoryTransferService_mock.h"

#include <cstdio>
#include "dawn/common/Assert.h"

namespace dawn::wire::client {

MockMemoryTransferService::MockReadHandle::MockReadHandle(MockMemoryTransferService* service)
    : ReadHandle(), mService(service) {}

MockMemoryTransferService::MockReadHandle::~MockReadHandle() {
    mService->OnReadHandleDestroy(this);
}

size_t MockMemoryTransferService::MockReadHandle::SerializeCreateSize() {
    return mService->OnReadHandleSerializeCreateSize(this);
}

void MockMemoryTransferService::MockReadHandle::SerializeCreate(void* serializePointer) {
    mService->OnReadHandleSerializeCreate(this, serializePointer);
}

const void* MockMemoryTransferService::MockReadHandle::GetData() {
    return mService->OnReadHandleGetData(this);
}

bool MockMemoryTransferService::MockReadHandle::DeserializeDataUpdate(
    const void* deserializePointer,
    size_t deserializeSize,
    size_t offset,
    size_t size) {
    ASSERT(deserializeSize % sizeof(uint32_t) == 0);
    return mService->OnReadHandleDeserializeDataUpdate(
        this, reinterpret_cast<const uint32_t*>(deserializePointer), deserializeSize, offset, size);
}

MockMemoryTransferService::MockWriteHandle::MockWriteHandle(MockMemoryTransferService* service)
    : WriteHandle(), mService(service) {}

MockMemoryTransferService::MockWriteHandle::~MockWriteHandle() {
    mService->OnWriteHandleDestroy(this);
}

size_t MockMemoryTransferService::MockWriteHandle::SerializeCreateSize() {
    return mService->OnWriteHandleSerializeCreateSize(this);
}

void MockMemoryTransferService::MockWriteHandle::SerializeCreate(void* serializePointer) {
    mService->OnWriteHandleSerializeCreate(this, serializePointer);
}

void* MockMemoryTransferService::MockWriteHandle::GetData() {
    return mService->OnWriteHandleGetData(this);
}

size_t MockMemoryTransferService::MockWriteHandle::SizeOfSerializeDataUpdate(size_t offset,
                                                                             size_t size) {
    return mService->OnWriteHandleSizeOfSerializeDataUpdate(this, offset, size);
}

void MockMemoryTransferService::MockWriteHandle::SerializeDataUpdate(void* serializePointer,
                                                                     size_t offset,
                                                                     size_t size) {
    mService->OnWriteHandleSerializeDataUpdate(this, serializePointer, offset, size);
}

MockMemoryTransferService::MockMemoryTransferService() = default;
MockMemoryTransferService::~MockMemoryTransferService() = default;

MockMemoryTransferService::ReadHandle* MockMemoryTransferService::CreateReadHandle(size_t size) {
    return OnCreateReadHandle(size);
}

MockMemoryTransferService::WriteHandle* MockMemoryTransferService::CreateWriteHandle(size_t size) {
    return OnCreateWriteHandle(size);
}

MockMemoryTransferService::MockReadHandle* MockMemoryTransferService::NewReadHandle() {
    return new MockReadHandle(this);
}

MockMemoryTransferService::MockWriteHandle* MockMemoryTransferService::NewWriteHandle() {
    return new MockWriteHandle(this);
}

}  // namespace dawn::wire::client
