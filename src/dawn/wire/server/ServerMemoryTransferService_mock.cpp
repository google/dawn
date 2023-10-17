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

#include "dawn/wire/server/ServerMemoryTransferService_mock.h"

#include "dawn/common/Assert.h"

namespace dawn::wire::server {

MockMemoryTransferService::MockReadHandle::MockReadHandle(MockMemoryTransferService* service)
    : ReadHandle(), mService(service) {}

MockMemoryTransferService::MockReadHandle::~MockReadHandle() {
    mService->OnReadHandleDestroy(this);
}

size_t MockMemoryTransferService::MockReadHandle::SizeOfSerializeDataUpdate(size_t offset,
                                                                            size_t size) {
    return mService->OnReadHandleSizeOfSerializeDataUpdate(this, offset, size);
}

void MockMemoryTransferService::MockReadHandle::SerializeDataUpdate(const void* data,
                                                                    size_t offset,
                                                                    size_t size,
                                                                    void* serializePointer) {
    mService->OnReadHandleSerializeDataUpdate(this, data, offset, size, serializePointer);
}

MockMemoryTransferService::MockWriteHandle::MockWriteHandle(MockMemoryTransferService* service)
    : WriteHandle(), mService(service) {}

MockMemoryTransferService::MockWriteHandle::~MockWriteHandle() {
    mService->OnWriteHandleDestroy(this);
}

const uint32_t* MockMemoryTransferService::MockWriteHandle::GetData() const {
    return reinterpret_cast<const uint32_t*>(mTargetData);
}

bool MockMemoryTransferService::MockWriteHandle::DeserializeDataUpdate(
    const void* deserializePointer,
    size_t deserializeSize,
    size_t offset,
    size_t size) {
    DAWN_ASSERT(deserializeSize % sizeof(uint32_t) == 0);
    return mService->OnWriteHandleDeserializeDataUpdate(
        this, reinterpret_cast<const uint32_t*>(deserializePointer), deserializeSize, offset, size);
}

MockMemoryTransferService::MockMemoryTransferService() = default;
MockMemoryTransferService::~MockMemoryTransferService() = default;

bool MockMemoryTransferService::DeserializeReadHandle(const void* deserializePointer,
                                                      size_t deserializeSize,
                                                      ReadHandle** readHandle) {
    DAWN_ASSERT(deserializeSize % sizeof(uint32_t) == 0);
    return OnDeserializeReadHandle(reinterpret_cast<const uint32_t*>(deserializePointer),
                                   deserializeSize, readHandle);
}

bool MockMemoryTransferService::DeserializeWriteHandle(const void* deserializePointer,
                                                       size_t deserializeSize,
                                                       WriteHandle** writeHandle) {
    DAWN_ASSERT(deserializeSize % sizeof(uint32_t) == 0);
    return OnDeserializeWriteHandle(reinterpret_cast<const uint32_t*>(deserializePointer),
                                    deserializeSize, writeHandle);
}

MockMemoryTransferService::MockReadHandle* MockMemoryTransferService::NewReadHandle() {
    return new MockReadHandle(this);
}

MockMemoryTransferService::MockWriteHandle* MockMemoryTransferService::NewWriteHandle() {
    return new MockWriteHandle(this);
}

}  // namespace dawn::wire::server
