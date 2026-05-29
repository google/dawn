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

#include <limits>
#include <memory>

#include "src/dawn/common/Assert.h"
#include "src/dawn/wire/server/Server.h"

namespace dawn::wire::server {

void Server::OnQueueWorkDone(QueueWorkDoneUserdata* data,
                             WGPUQueueWorkDoneStatus status,
                             WGPUStringView message) {
    ReturnQueueWorkDoneCallbackCmd cmd;
    cmd.eventManager = data->eventManager;
    cmd.future = data->future;
    cmd.status = status;
    cmd.message = message;

    SerializeCommand(cmd);
}

WireResult Server::DoQueueOnSubmittedWorkDone(Known<WGPUQueue> queue,
                                              ObjectHandle eventManager,
                                              WGPUFuture future) {
    auto userdata = MakeUserdata<QueueWorkDoneUserdata>();
    userdata->queue = queue.AsHandle();
    userdata->eventManager = eventManager;
    userdata->future = future;

    mProcs->queueOnSubmittedWorkDone(
        queue->handle, MakeCallbackInfo<WGPUQueueWorkDoneCallbackInfo, &Server::OnQueueWorkDone>(
                           userdata.release()));
    return WireResult::Success;
}

WireResult Server::DoQueueWriteBuffer(Known<WGPUQueue> queue,
                                      Known<WGPUBuffer> buffer,
                                      uint64_t bufferOffset,
                                      const uint8_t* data,
                                      size_t size) {
    mProcs->queueWriteBuffer(queue->handle, buffer->handle, bufferOffset, data, size);
    return WireResult::Success;
}

WireResult Server::DoQueueWriteBufferXl(Known<WGPUQueue> queue,
                                        Known<WGPUBuffer> buffer,
                                        uint64_t bufferOffset,
                                        size_t size,
                                        size_t writeHandleCreateInfoLength,
                                        const uint8_t* writeHandleCreateInfo,
                                        size_t writeDataUpdateInfoLength,
                                        const uint8_t* writeDataUpdateInfo) {
    // Deserialize metadata produced from the client to create a companion server handle.
    MemoryTransferService::WriteHandle* writeHandlePtr = nullptr;
    if (!mMemoryTransferService->DeserializeWriteHandle(
            writeHandleCreateInfo, writeHandleCreateInfoLength, &writeHandlePtr)) {
        return WireResult::FatalError;
    }

    // Ensure the WriteHandle gets properly cleaned up when the function returns.
    std::unique_ptr<MemoryTransferService::WriteHandle> writeHandle(writeHandlePtr);

    // Try first to use GetSourceData if the memory transfer service implements
    // it. If so, we can avoid a copy.
    std::span<uint8_t> source = writeHandle->GetSource();
    if (!source.empty()) {
        if (source.size() < size) {
            return WireResult::FatalError;
        }

        mProcs->queueWriteBuffer(queue->handle, buffer->handle, bufferOffset, source.data(), size);
        return WireResult::Success;
    }

    // Otherwise, fall back to DeserializeDataUpdate.
    auto backingData = std::unique_ptr<uint8_t[]>(AllocNoThrow<uint8_t>(size));
    if (!backingData) {
        return WireResult::FatalError;
    }

    // Deserialize the flush info and flush updated data from the handle into the target
    // of the handle that's just a temporary allocation from above right now.
    std::span<const uint8_t> writeDataUpdateInfoSpan(writeDataUpdateInfo,
                                                     writeDataUpdateInfoLength);
    std::span<uint8_t> target(backingData.get(), size);
    if (!writeHandle->DeserializeDataUpdate(writeDataUpdateInfoSpan, target, 0u)) {
        return WireResult::FatalError;
    }

    mProcs->queueWriteBuffer(queue->handle, buffer->handle, bufferOffset, backingData.get(), size);
    return WireResult::Success;
}

WireResult Server::DoQueueWriteTexture(Known<WGPUQueue> queue,
                                       const WGPUTexelCopyTextureInfo* destination,
                                       const uint8_t* data,
                                       size_t dataSize,
                                       const WGPUTexelCopyBufferLayout* dataLayout,
                                       const WGPUExtent3D* writeSize) {
    mProcs->queueWriteTexture(queue->handle, destination, data, dataSize, dataLayout, writeSize);
    return WireResult::Success;
}

WireResult Server::DoQueueWriteTextureXl(Known<WGPUQueue> queue,
                                         const WGPUTexelCopyTextureInfo* destination,
                                         size_t dataSize,
                                         const WGPUTexelCopyBufferLayout* dataLayout,
                                         const WGPUExtent3D* writeSize,
                                         size_t writeHandleCreateInfoLength,
                                         const uint8_t* writeHandleCreateInfo,
                                         size_t writeDataUpdateInfoLength,
                                         const uint8_t* writeDataUpdateInfo) {
    // Deserialize metadata produced from the client to create a companion server handle.
    MemoryTransferService::WriteHandle* writeHandlePtr = nullptr;
    if (!mMemoryTransferService->DeserializeWriteHandle(
            writeHandleCreateInfo, writeHandleCreateInfoLength, &writeHandlePtr)) {
        return WireResult::FatalError;
    }

    // Ensure the WriteHandle gets properly cleaned up when the function returns.
    std::unique_ptr<MemoryTransferService::WriteHandle> writeHandle(writeHandlePtr);

    // Try first to use GetSourceData if the memory transfer service implements
    // it. If so, we can avoid a copy.
    std::span<uint8_t> source = writeHandle->GetSource();
    if (!source.empty()) {
        if (source.size() < dataSize) {
            return WireResult::FatalError;
        }

        mProcs->queueWriteTexture(queue->handle, destination, source.data(), dataSize, dataLayout,
                                  writeSize);
        return WireResult::Success;
    }

    // Otherwise, fall back to DeserializeDataUpdate.
    auto backingData = std::unique_ptr<uint8_t[]>(AllocNoThrow<uint8_t>(dataSize));
    if (!backingData) {
        return WireResult::FatalError;
    }

    // Deserialize the flush info and flush updated data from the handle into the target
    // of the handle that's just a temporary allocation from above right now.
    std::span<const uint8_t> writeDataUpdateInfoSpan(writeDataUpdateInfo,
                                                     writeDataUpdateInfoLength);
    std::span<uint8_t> target(backingData.get(), dataSize);
    if (!writeHandle->DeserializeDataUpdate(writeDataUpdateInfoSpan, target, 0u)) {
        return WireResult::FatalError;
    }

    mProcs->queueWriteTexture(queue->handle, destination, backingData.get(), dataSize, dataLayout,
                              writeSize);
    return WireResult::Success;
}

}  // namespace dawn::wire::server
