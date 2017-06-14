// Copyright 2017 The NXT Authors
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

#include "ResourceUploader.h"

#include "MetalBackend.h"

namespace backend {
namespace metal {

    ResourceUploader::ResourceUploader(Device* device)
        : device(device) {
    }

    ResourceUploader::~ResourceUploader() {
        ASSERT(inflightUploadBuffers.Empty());
    }

    void ResourceUploader::BufferSubData(id<MTLBuffer> buffer, uint32_t start, uint32_t size, const void* data) {
        // TODO(cwallez@chromium.org) use a ringbuffer instead of creating a small buffer for each update
        id<MTLBuffer> uploadBuffer = [device->GetMTLDevice() newBufferWithLength:size
            options:MTLResourceStorageModeShared];
        memcpy([uploadBuffer contents], data, size);

        id<MTLCommandBuffer> commandBuffer = device->GetPendingCommandBuffer();
        id<MTLBlitCommandEncoder> encoder = [commandBuffer blitCommandEncoder];
        [encoder copyFromBuffer:uploadBuffer
                sourceOffset:0
                toBuffer:buffer
                destinationOffset:start
                size:size];
        [encoder endEncoding];

        inflightUploadBuffers.Enqueue(uploadBuffer, device->GetPendingCommandSerial());
    }

    void ResourceUploader::Tick(Serial finishedSerial) {
        for (id<MTLBuffer> buffer : inflightUploadBuffers.IterateUpTo(finishedSerial)) {
            [buffer release];
        }
        inflightUploadBuffers.ClearUpTo(finishedSerial);
    }

}
}
