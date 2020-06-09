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

#include <limits>

namespace dawn_native { namespace metal {
    // The size of uniform buffer and storage buffer need to be aligned to 16 bytes which is the
    // largest alignment of supported data types
    static constexpr uint32_t kMinUniformOrStorageBufferAlignment = 16u;

    // static
    ResultOrError<Buffer*> Buffer::Create(Device* device, const BufferDescriptor* descriptor) {
        Ref<Buffer> buffer = AcquireRef(new Buffer(device, descriptor));
        DAWN_TRY(buffer->Initialize());
        return buffer.Detach();
    }

    MaybeError Buffer::Initialize() {
        MTLResourceOptions storageMode;
        if (GetUsage() & (wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite)) {
            storageMode = MTLResourceStorageModeShared;
        } else {
            storageMode = MTLResourceStorageModePrivate;
        }

        // TODO(cwallez@chromium.org): Have a global "zero" buffer that can do everything instead
        // of creating a new 4-byte buffer?
        if (GetSize() > std::numeric_limits<NSUInteger>::max()) {
            return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
        }
        NSUInteger currentSize = static_cast<NSUInteger>(std::max(GetSize(), uint64_t(4u)));

        // Metal validation layer requires the size of uniform buffer and storage buffer to be no
        // less than the size of the buffer block defined in shader, and the overall size of the
        // buffer must be aligned to the largest alignment of its members.
        if (GetUsage() & (wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage)) {
            if (currentSize >
                std::numeric_limits<NSUInteger>::max() - kMinUniformOrStorageBufferAlignment) {
                // Alignment would overlow.
                return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
            }
            currentSize = Align(currentSize, kMinUniformOrStorageBufferAlignment);
        }

        if (@available(iOS 12, macOS 10.14, *)) {
            NSUInteger maxBufferSize = [ToBackend(GetDevice())->GetMTLDevice() maxBufferLength];
            if (currentSize > maxBufferSize) {
                return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
            }
        }

        mMtlBuffer = [ToBackend(GetDevice())->GetMTLDevice() newBufferWithLength:currentSize
                                                                         options:storageMode];
        if (mMtlBuffer == nil) {
            return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation failed");
        }

        if (GetDevice()->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
            ClearBuffer(BufferBase::ClearValue::NonZero);
        }

        return {};
    }

    Buffer::~Buffer() {
        DestroyInternal();
    }

    id<MTLBuffer> Buffer::GetMTLBuffer() const {
        return mMtlBuffer;
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
        return {};
    }

    MaybeError Buffer::MapWriteAsyncImpl(uint32_t serial) {
        return {};
    }

    void* Buffer::GetMappedPointerImpl() {
        return reinterpret_cast<uint8_t*>([mMtlBuffer contents]);
    }

    void Buffer::UnmapImpl() {
        // Nothing to do, Metal StorageModeShared buffers are always mapped.
    }

    void Buffer::DestroyImpl() {
        [mMtlBuffer release];
        mMtlBuffer = nil;
    }

    void Buffer::ClearBuffer(BufferBase::ClearValue clearValue) {
        // TODO(jiawei.shao@intel.com): support buffer lazy-initialization to 0.
        ASSERT(clearValue == BufferBase::ClearValue::NonZero);
        const uint8_t clearBufferValue = 1;

        Device* device = ToBackend(GetDevice());
        CommandRecordingContext* commandContext = device->GetPendingCommandContext();
        [commandContext->EnsureBlit() fillBuffer:mMtlBuffer
                                           range:NSMakeRange(0, GetSize())
                                           value:clearBufferValue];
    }

}}  // namespace dawn_native::metal
