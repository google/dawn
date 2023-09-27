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

#include <limits>

#include "dawn/common/Assert.h"
#include "dawn/wire/server/Server.h"

namespace dawn::wire::server {

void Server::OnQueueWorkDone(QueueWorkDoneUserdata* data, WGPUQueueWorkDoneStatus status) {
    ReturnQueueWorkDoneCallbackCmd cmd;
    cmd.queue = data->queue;
    cmd.future = data->future;
    cmd.status = status;

    SerializeCommand(cmd);
}

WireResult Server::DoQueueOnSubmittedWorkDone(Known<WGPUQueue> queue,
                                              uint64_t signalValue,
                                              WGPUFuture future) {
    auto userdata = MakeUserdata<QueueWorkDoneUserdata>();
    userdata->queue = queue.AsHandle();
    userdata->future = future;

    mProcs.queueOnSubmittedWorkDone(queue->handle, signalValue,
                                    ForwardToServer<&Server::OnQueueWorkDone>, userdata.release());
    return WireResult::Success;
}

WireResult Server::DoQueueWriteBuffer(Known<WGPUQueue> queue,
                                      Known<WGPUBuffer> buffer,
                                      uint64_t bufferOffset,
                                      const uint8_t* data,
                                      uint64_t size) {
    if (size > std::numeric_limits<size_t>::max()) {
        return WireResult::FatalError;
    }

    mProcs.queueWriteBuffer(queue->handle, buffer->handle, bufferOffset, data,
                            static_cast<size_t>(size));
    return WireResult::Success;
}

WireResult Server::DoQueueWriteTexture(Known<WGPUQueue> queue,
                                       const WGPUImageCopyTexture* destination,
                                       const uint8_t* data,
                                       uint64_t dataSize,
                                       const WGPUTextureDataLayout* dataLayout,
                                       const WGPUExtent3D* writeSize) {
    if (dataSize > std::numeric_limits<size_t>::max()) {
        return WireResult::FatalError;
    }

    mProcs.queueWriteTexture(queue->handle, destination, data, static_cast<size_t>(dataSize),
                             dataLayout, writeSize);
    return WireResult::Success;
}

}  // namespace dawn::wire::server
