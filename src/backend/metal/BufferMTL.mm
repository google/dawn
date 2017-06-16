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

        MTLResourceOptions storageMode;
        if (GetAllowedUsage() & (nxt::BufferUsageBit::MapRead | nxt::BufferUsageBit::MapWrite)) {
            storageMode = MTLResourceStorageModeShared;
        } else {
            storageMode = MTLResourceStorageModePrivate;
        }

        mtlBuffer = [ToBackend(GetDevice())->GetMTLDevice() newBufferWithLength:GetSize()
            options:storageMode];
    }

    Buffer::~Buffer() {
        [mtlBuffer release];
        mtlBuffer = nil;
    }

    id<MTLBuffer> Buffer::GetMTLBuffer() {
        return mtlBuffer;
    }

    void Buffer::OnMapReadCommandSerialFinished(uint32_t mapSerial, uint32_t offset) {
        const char* data = reinterpret_cast<const char*>([mtlBuffer contents]);
        CallMapReadCallback(mapSerial, NXT_BUFFER_MAP_READ_STATUS_SUCCESS, data + offset);
    }

    void Buffer::SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) {
        auto* uploader = ToBackend(GetDevice())->GetResourceUploader();
        uploader->BufferSubData(mtlBuffer, start * sizeof(uint32_t), count * sizeof(uint32_t), data);
    }

    void Buffer::MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) {
        MapReadRequestTracker* tracker = ToBackend(GetDevice())->GetMapReadTracker();
        tracker->Track(this, serial, start);
    }

    void Buffer::UnmapImpl() {
        // Nothing to do, Metal StorageModeShared buffers are always mapped.
    }

    void Buffer::TransitionUsageImpl(nxt::BufferUsageBit currentUsage, nxt::BufferUsageBit targetUsage) {
    }

    BufferView::BufferView(BufferViewBuilder* builder)
        : BufferViewBase(builder) {
    }

    MapReadRequestTracker::MapReadRequestTracker(Device* device)
        : device(device) {
    }

    MapReadRequestTracker::~MapReadRequestTracker() {
        ASSERT(inflightRequests.Empty());
    }

    void MapReadRequestTracker::Track(Buffer* buffer, uint32_t mapSerial, uint32_t offset) {
        Request request;
        request.buffer = buffer;
        request.mapSerial = mapSerial;
        request.offset = offset;

        inflightRequests.Enqueue(std::move(request), device->GetPendingCommandSerial());
    }

    void MapReadRequestTracker::Tick(Serial finishedSerial) {
        for (auto& request : inflightRequests.IterateUpTo(finishedSerial)) {
            request.buffer->OnMapReadCommandSerialFinished(request.mapSerial, request.offset);
        }
        inflightRequests.ClearUpTo(finishedSerial);
    }
}
}
