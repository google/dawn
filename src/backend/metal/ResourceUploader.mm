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

#include "backend/metal/ResourceUploader.h"

#include "backend/metal/MetalBackend.h"

namespace backend { namespace metal {

    ResourceUploader::ResourceUploader(Device* device) : mDevice(device) {
    }

    ResourceUploader::~ResourceUploader() {
        ASSERT(mInflightUploadBuffers.Empty());
    }

    void ResourceUploader::BufferSubData(id<MTLBuffer> buffer,
                                         uint32_t start,
                                         uint32_t size,
                                         const void* data) {
        // TODO(cwallez@chromium.org) use a ringbuffer instead of creating a small buffer for each
        // update
        id<MTLBuffer> uploadBuffer =
            [mDevice->GetMTLDevice() newBufferWithLength:size options:MTLResourceStorageModeShared];
        memcpy([uploadBuffer contents], data, size);

        id<MTLCommandBuffer> commandBuffer = mDevice->GetPendingCommandBuffer();
        id<MTLBlitCommandEncoder> encoder = [commandBuffer blitCommandEncoder];
        [encoder copyFromBuffer:uploadBuffer
                   sourceOffset:0
                       toBuffer:buffer
              destinationOffset:start
                           size:size];
        [encoder endEncoding];

        mInflightUploadBuffers.Enqueue(uploadBuffer, mDevice->GetPendingCommandSerial());
    }

    void ResourceUploader::Tick(Serial finishedSerial) {
        for (id<MTLBuffer> buffer : mInflightUploadBuffers.IterateUpTo(finishedSerial)) {
            [buffer release];
        }
        mInflightUploadBuffers.ClearUpTo(finishedSerial);
    }

}}  // namespace backend::metal
