// Copyright 2017 The Dawn Authors
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

#include "dawn_native/metal/BufferMTL.h"

#include "dawn_native/metal/DeviceMTL.h"
#include "dawn_native/metal/ResourceUploader.h"

namespace dawn_native { namespace metal {

    Buffer::Buffer(Device* device, const BufferDescriptor* descriptor)
        : BufferBase(device, descriptor) {
        MTLResourceOptions storageMode;
        if (GetUsage() & (dawn::BufferUsageBit::MapRead | dawn::BufferUsageBit::MapWrite)) {
            storageMode = MTLResourceStorageModeShared;
        } else {
            storageMode = MTLResourceStorageModePrivate;
        }

        mMtlBuffer = [device->GetMTLDevice() newBufferWithLength:GetSize() options:storageMode];
    }

    Buffer::~Buffer() {
        [mMtlBuffer release];
        mMtlBuffer = nil;
    }

    id<MTLBuffer> Buffer::GetMTLBuffer() {
        return mMtlBuffer;
    }

    void Buffer::OnMapCommandSerialFinished(uint32_t mapSerial, uint32_t offset, bool isWrite) {
        char* data = reinterpret_cast<char*>([mMtlBuffer contents]);
        if (isWrite) {
            CallMapWriteCallback(mapSerial, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, data + offset);
        } else {
            CallMapReadCallback(mapSerial, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, data + offset);
        }
    }

    void Buffer::SetSubDataImpl(uint32_t start, uint32_t count, const uint8_t* data) {
        auto* uploader = ToBackend(GetDevice())->GetResourceUploader();
        uploader->BufferSubData(mMtlBuffer, start, count, data);
    }

    void Buffer::MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t) {
        MapRequestTracker* tracker = ToBackend(GetDevice())->GetMapTracker();
        tracker->Track(this, serial, start, false);
    }

    void Buffer::MapWriteAsyncImpl(uint32_t serial, uint32_t start, uint32_t) {
        MapRequestTracker* tracker = ToBackend(GetDevice())->GetMapTracker();
        tracker->Track(this, serial, start, true);
    }

    void Buffer::UnmapImpl() {
        // Nothing to do, Metal StorageModeShared buffers are always mapped.
    }

    MapRequestTracker::MapRequestTracker(Device* device) : mDevice(device) {
    }

    MapRequestTracker::~MapRequestTracker() {
        ASSERT(mInflightRequests.Empty());
    }

    void MapRequestTracker::Track(Buffer* buffer,
                                  uint32_t mapSerial,
                                  uint32_t offset,
                                  bool isWrite) {
        Request request;
        request.buffer = buffer;
        request.mapSerial = mapSerial;
        request.offset = offset;
        request.isWrite = isWrite;

        mInflightRequests.Enqueue(std::move(request), mDevice->GetPendingCommandSerial());
    }

    void MapRequestTracker::Tick(Serial finishedSerial) {
        for (auto& request : mInflightRequests.IterateUpTo(finishedSerial)) {
            request.buffer->OnMapCommandSerialFinished(request.mapSerial, request.offset,
                                                       request.isWrite);
        }
        mInflightRequests.ClearUpTo(finishedSerial);
    }

}}  // namespace dawn_native::metal
