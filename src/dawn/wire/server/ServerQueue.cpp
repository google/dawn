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

#include "dawn/common/Assert.h"
#include "dawn/wire/server/Server.h"

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

    mProcs.queueOnSubmittedWorkDone(
        queue->handle, MakeCallbackInfo<WGPUQueueWorkDoneCallbackInfo, &Server::OnQueueWorkDone>(
                           userdata.release()));
    return WireResult::Success;
}

WireResult Server::DoQueueWriteBuffer(Known<WGPUQueue> queue,
                                      Known<WGPUBuffer> buffer,
                                      uint64_t bufferOffset,
                                      uint64_t size,
                                      uint64_t writeHandleCreateInfoLength,
                                      const uint8_t* writeHandleCreateInfo,
                                      uint64_t writeDataUpdateInfoLength,
                                      const uint8_t* writeDataUpdateInfo) {
    if (size > std::numeric_limits<size_t>::max()) {
        return WireResult::FatalError;
    }

    MemoryTransferService::WriteHandle* writeHandle = nullptr;
    // Deserialize metadata produced from the client to create a companion server handle.
    if (!mMemoryTransferService->DeserializeWriteHandle(
            writeHandleCreateInfo, static_cast<size_t>(writeHandleCreateInfoLength),
            &writeHandle)) {
        return WireResult::FatalError;
    }

    // Try first to use GetSourceData if the memory transfer service implements
    // it. If so, we can avoid a copy.
    uint8_t* sourceData = writeHandle->GetSourceData();
    if (sourceData) {
        mProcs.queueWriteBuffer(queue->handle, buffer->handle, bufferOffset, sourceData,
                                static_cast<size_t>(size));
        return WireResult::Success;
    }

    // Otherwise, fall back to DeserializeDataUpdate.
    auto backingData = std::make_unique<char[]>(size);
    writeHandle->SetTarget(backingData.get());
    writeHandle->SetDataLength(size);

    // Deserialize the flush info and flush updated data from the handle into the target
    // of the handle that's just a temporary allocation from above right now.
    if (!writeHandle->DeserializeDataUpdate(writeDataUpdateInfo,
                                            static_cast<size_t>(writeDataUpdateInfoLength), 0u,
                                            static_cast<size_t>(size))) {
        return WireResult::FatalError;
    }

    mProcs.queueWriteBuffer(queue->handle, buffer->handle, bufferOffset, backingData.get(),
                            static_cast<size_t>(size));
    return WireResult::Success;
}

WireResult Server::DoQueueWriteTexture(Known<WGPUQueue> queue,
                                       const WGPUTexelCopyTextureInfo* destination,
                                       uint64_t dataSize,
                                       const WGPUTexelCopyBufferLayout* dataLayout,
                                       const WGPUExtent3D* writeSize,
                                       uint64_t writeHandleCreateInfoLength,
                                       const uint8_t* writeHandleCreateInfo,
                                       uint64_t writeDataUpdateInfoLength,
                                       const uint8_t* writeDataUpdateInfo) {
    if (dataSize > std::numeric_limits<size_t>::max()) {
        return WireResult::FatalError;
    }

    MemoryTransferService::WriteHandle* writeHandle = nullptr;
    // Deserialize metadata produced from the client to create a companion server handle.
    if (!mMemoryTransferService->DeserializeWriteHandle(
            writeHandleCreateInfo, static_cast<size_t>(writeHandleCreateInfoLength),
            &writeHandle)) {
        return WireResult::FatalError;
    }

    // Try first to use GetSourceData if the memory transfer service implements
    // it. If so, we can avoid a copy.
    uint8_t* sourceData = writeHandle->GetSourceData();
    if (sourceData) {
        mProcs.queueWriteTexture(queue->handle, destination, sourceData,
                                 static_cast<size_t>(dataSize), dataLayout, writeSize);
        return WireResult::Success;
    }

    // Otherwise, fall back to DeserializeDataUpdate.
    auto backingData = std::make_unique<char[]>(dataSize);
    writeHandle->SetTarget(backingData.get());
    writeHandle->SetDataLength(dataSize);

    // Deserialize the flush info and flush updated data from the handle into the target
    // of the handle that's just a temporary allocation from above right now.
    if (!writeHandle->DeserializeDataUpdate(writeDataUpdateInfo,
                                            static_cast<size_t>(writeDataUpdateInfoLength), 0u,
                                            static_cast<size_t>(dataSize))) {
        return WireResult::FatalError;
    }

    mProcs.queueWriteTexture(queue->handle, destination, backingData.get(),
                             static_cast<size_t>(dataSize), dataLayout, writeSize);
    return WireResult::Success;
}

}  // namespace dawn::wire::server
