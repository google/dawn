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

#include "common/Math.h"
#include "dawn_native/metal/DeviceMTL.h"

namespace dawn_native { namespace metal {
    // The size of uniform buffer and storage buffer need to be aligned to 16 bytes which is the
    // largest alignment of supported data types
    static constexpr uint32_t kMinUniformOrStorageBufferAlignment = 16u;

    Buffer::Buffer(Device* device, const BufferDescriptor* descriptor)
        : BufferBase(device, descriptor) {
        MTLResourceOptions storageMode;
        if (GetUsage() & (wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite)) {
            storageMode = MTLResourceStorageModeShared;
        } else {
            storageMode = MTLResourceStorageModePrivate;
        }

        uint32_t currentSize = GetSize();
        // Metal validation layer requires the size of uniform buffer and storage buffer to be no
        // less than the size of the buffer block defined in shader, and the overall size of the
        // buffer must be aligned to the largest alignment of its members.
        if (GetUsage() & (wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage)) {
            currentSize = Align(currentSize, kMinUniformOrStorageBufferAlignment);
        }

        mMtlBuffer = [device->GetMTLDevice() newBufferWithLength:currentSize options:storageMode];
    }

    Buffer::~Buffer() {
        DestroyInternal();
    }

    id<MTLBuffer> Buffer::GetMTLBuffer() const {
        return mMtlBuffer;
    }

    void Buffer::OnMapCommandSerialFinished(uint32_t mapSerial, bool isWrite) {
        char* data = reinterpret_cast<char*>([mMtlBuffer contents]);
        if (isWrite) {
            CallMapWriteCallback(mapSerial, WGPUBufferMapAsyncStatus_Success, data, GetSize());
        } else {
            CallMapReadCallback(mapSerial, WGPUBufferMapAsyncStatus_Success, data, GetSize());
        }
    }

    bool Buffer::IsMapWritable() const {
        // TODO(enga): Handle CPU-visible memory on UMA
        return (GetUsage() & (wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite)) != 0;
    }

    MaybeError Buffer::MapAtCreationImpl(uint8_t** mappedPointer) {
        *mappedPointer = reinterpret_cast<uint8_t*>([mMtlBuffer contents]);
        return {};
    }

    MaybeError Buffer::MapReadAsyncImpl(uint32_t serial) {
        MapRequestTracker* tracker = ToBackend(GetDevice())->GetMapTracker();
        tracker->Track(this, serial, false);
        return {};
    }

    MaybeError Buffer::MapWriteAsyncImpl(uint32_t serial) {
        MapRequestTracker* tracker = ToBackend(GetDevice())->GetMapTracker();
        tracker->Track(this, serial, true);
        return {};
    }

    void Buffer::UnmapImpl() {
        // Nothing to do, Metal StorageModeShared buffers are always mapped.
    }

    void Buffer::DestroyImpl() {
        [mMtlBuffer release];
        mMtlBuffer = nil;
    }

    MapRequestTracker::MapRequestTracker(Device* device) : mDevice(device) {
    }

    MapRequestTracker::~MapRequestTracker() {
        ASSERT(mInflightRequests.Empty());
    }

    void MapRequestTracker::Track(Buffer* buffer,
                                  uint32_t mapSerial,
                                  bool isWrite) {
        Request request;
        request.buffer = buffer;
        request.mapSerial = mapSerial;
        request.isWrite = isWrite;

        mInflightRequests.Enqueue(std::move(request), mDevice->GetPendingCommandSerial());
    }

    void MapRequestTracker::Tick(Serial finishedSerial) {
        for (auto& request : mInflightRequests.IterateUpTo(finishedSerial)) {
            request.buffer->OnMapCommandSerialFinished(request.mapSerial, request.isWrite);
        }
        mInflightRequests.ClearUpTo(finishedSerial);
    }

}}  // namespace dawn_native::metal
