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

#include "BufferMTL.h"

#include "MetalBackend.h"
#include "ResourceUploader.h"

namespace backend {
namespace metal {

    Buffer::Buffer(BufferBuilder* builder)
        : BufferBase(builder) {
        mtlBuffer = [ToBackend(GetDevice())->GetMTLDevice() newBufferWithLength:GetSize()
            options:MTLResourceStorageModePrivate];
    }

    Buffer::~Buffer() {
        [mtlBuffer release];
        mtlBuffer = nil;
    }

    id<MTLBuffer> Buffer::GetMTLBuffer() {
        return mtlBuffer;
    }

    void Buffer::SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) {
        auto* uploader = ToBackend(GetDevice())->GetResourceUploader();
        uploader->BufferSubData(mtlBuffer, start * sizeof(uint32_t), count * sizeof(uint32_t), data);
    }

    void Buffer::MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) {
        // TODO(cwallez@chromium.org): Implement Map Read for the metal backend
    }

    void Buffer::UnmapImpl() {
        // TODO(cwallez@chromium.org): Implement Map Read for the metal backend
    }

    void Buffer::TransitionUsageImpl(nxt::BufferUsageBit currentUsage, nxt::BufferUsageBit targetUsage) {
    }

    BufferView::BufferView(BufferViewBuilder* builder)
        : BufferViewBase(builder) {
    }

}
}
