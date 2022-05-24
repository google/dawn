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
    cmd.requestSerial = data->requestSerial;
    cmd.status = status;

    SerializeCommand(cmd);
}

bool Server::DoQueueOnSubmittedWorkDone(ObjectId queueId,
                                        uint64_t signalValue,
                                        uint64_t requestSerial) {
    auto* queue = QueueObjects().Get(queueId);
    if (queue == nullptr) {
        return false;
    }

    auto userdata = MakeUserdata<QueueWorkDoneUserdata>();
    userdata->queue = ObjectHandle{queueId, queue->generation};
    userdata->requestSerial = requestSerial;

    mProcs.queueOnSubmittedWorkDone(queue->handle, signalValue,
                                    ForwardToServer<&Server::OnQueueWorkDone>, userdata.release());
    return true;
}

bool Server::DoQueueWriteBuffer(ObjectId queueId,
                                ObjectId bufferId,
                                uint64_t bufferOffset,
                                const uint8_t* data,
                                uint64_t size) {
    // The null object isn't valid as `self` or `buffer` so we can combine the check with the
    // check that the ID is valid.
    auto* queue = QueueObjects().Get(queueId);
    auto* buffer = BufferObjects().Get(bufferId);
    if (queue == nullptr || buffer == nullptr) {
        return false;
    }

    if (size > std::numeric_limits<size_t>::max()) {
        return false;
    }

    mProcs.queueWriteBuffer(queue->handle, buffer->handle, bufferOffset, data,
                            static_cast<size_t>(size));
    return true;
}

bool Server::DoQueueWriteTexture(ObjectId queueId,
                                 const WGPUImageCopyTexture* destination,
                                 const uint8_t* data,
                                 uint64_t dataSize,
                                 const WGPUTextureDataLayout* dataLayout,
                                 const WGPUExtent3D* writeSize) {
    // The null object isn't valid as `self` so we can combine the check with the
    // check that the ID is valid.
    auto* queue = QueueObjects().Get(queueId);
    if (queue == nullptr) {
        return false;
    }

    if (dataSize > std::numeric_limits<size_t>::max()) {
        return false;
    }

    mProcs.queueWriteTexture(queue->handle, destination, data, static_cast<size_t>(dataSize),
                             dataLayout, writeSize);
    return true;
}

}  // namespace dawn::wire::server
