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

#include "dawn/native/opengl/BufferGL.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "dawn/native/CommandBuffer.h"
#include "dawn/native/opengl/DeviceGL.h"

namespace dawn::native::opengl {

// Buffer

// static
ResultOrError<Ref<Buffer>> Buffer::CreateInternalBuffer(Device* device,
                                                        const BufferDescriptor* descriptor,
                                                        bool shouldLazyClear) {
    Ref<Buffer> buffer = AcquireRef(new Buffer(device, descriptor, shouldLazyClear));
    if (descriptor->mappedAtCreation) {
        DAWN_TRY(buffer->MapAtCreationInternal());
    }

    return std::move(buffer);
}

Buffer::Buffer(Device* device, const BufferDescriptor* descriptor)
    : BufferBase(device, descriptor) {
    const OpenGLFunctions& gl = device->GetGL();
    // Allocate at least 4 bytes so clamped accesses are always in bounds.
    mAllocatedSize = std::max(GetSize(), uint64_t(4u));

    gl.GenBuffers(1, &mBuffer);
    gl.BindBuffer(GL_ARRAY_BUFFER, mBuffer);

    // The buffers with mappedAtCreation == true will be initialized in
    // BufferBase::MapAtCreation().
    if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting) &&
        !descriptor->mappedAtCreation) {
        std::vector<uint8_t> clearValues(mAllocatedSize, 1u);
        gl.BufferData(GL_ARRAY_BUFFER, mAllocatedSize, clearValues.data(), GL_STATIC_DRAW);
    } else {
        // Buffers start uninitialized if you pass nullptr to glBufferData.
        gl.BufferData(GL_ARRAY_BUFFER, mAllocatedSize, nullptr, GL_STATIC_DRAW);
    }
    TrackUsage();
}

Buffer::Buffer(Device* device, const BufferDescriptor* descriptor, bool shouldLazyClear)
    : Buffer(device, descriptor) {
    if (!shouldLazyClear) {
        SetIsDataInitialized();
    }
}

Buffer::~Buffer() = default;

GLuint Buffer::GetHandle() const {
    return mBuffer;
}

bool Buffer::EnsureDataInitialized() {
    if (!NeedsInitialization()) {
        return false;
    }

    InitializeToZero();
    return true;
}

bool Buffer::EnsureDataInitializedAsDestination(uint64_t offset, uint64_t size) {
    if (!NeedsInitialization()) {
        return false;
    }

    if (IsFullBufferRange(offset, size)) {
        SetIsDataInitialized();
        return false;
    }

    InitializeToZero();
    return true;
}

bool Buffer::EnsureDataInitializedAsDestination(const CopyTextureToBufferCmd* copy) {
    if (!NeedsInitialization()) {
        return false;
    }

    if (IsFullBufferOverwrittenInTextureToBufferCopy(copy)) {
        SetIsDataInitialized();
        return false;
    }

    InitializeToZero();
    return true;
}

void Buffer::InitializeToZero() {
    ASSERT(NeedsInitialization());

    const uint64_t size = GetAllocatedSize();
    Device* device = ToBackend(GetDevice());
    const OpenGLFunctions& gl = device->GetGL();

    const std::vector<uint8_t> clearValues(size, 0u);
    gl.BindBuffer(GL_ARRAY_BUFFER, mBuffer);
    gl.BufferSubData(GL_ARRAY_BUFFER, 0, size, clearValues.data());
    device->IncrementLazyClearCountForTesting();

    TrackUsage();
    SetIsDataInitialized();
}

bool Buffer::IsCPUWritableAtCreation() const {
    // TODO(enga): All buffers in GL can be mapped. Investigate if mapping them will cause the
    // driver to migrate it to shared memory.
    return true;
}

MaybeError Buffer::MapAtCreationImpl() {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
    gl.BindBuffer(GL_ARRAY_BUFFER, mBuffer);
    mMappedData = gl.MapBufferRange(GL_ARRAY_BUFFER, 0, GetSize(), GL_MAP_WRITE_BIT);
    return {};
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();

    // It is an error to map an empty range in OpenGL. We always have at least a 4-byte buffer
    // so we extend the range to be 4 bytes.
    if (size == 0) {
        if (offset != 0) {
            offset -= 4;
        }
        size = 4;
    }

    EnsureDataInitialized();

    // This does GPU->CPU synchronization, we could require a high
    // version of OpenGL that would let us map the buffer unsynchronized.
    gl.BindBuffer(GL_ARRAY_BUFFER, mBuffer);
    void* mappedData = nullptr;
    if (mode & wgpu::MapMode::Read) {
        mappedData = gl.MapBufferRange(GL_ARRAY_BUFFER, offset, size, GL_MAP_READ_BIT);
    } else {
        ASSERT(mode & wgpu::MapMode::Write);
        mappedData = gl.MapBufferRange(GL_ARRAY_BUFFER, offset, size,
                                       GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    }

    // The frontend asks that the pointer returned by GetMappedPointer is from the start of
    // the resource but OpenGL gives us the pointer at offset. Remove the offset.
    mMappedData = static_cast<uint8_t*>(mappedData) - offset;
    return {};
}

void* Buffer::GetMappedPointer() {
    // The mapping offset has already been removed.
    return mMappedData;
}

void Buffer::UnmapImpl() {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();

    gl.BindBuffer(GL_ARRAY_BUFFER, mBuffer);
    gl.UnmapBuffer(GL_ARRAY_BUFFER);
    mMappedData = nullptr;
}

void Buffer::DestroyImpl() {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();

    BufferBase::DestroyImpl();
    gl.DeleteBuffers(1, &mBuffer);
    mBuffer = 0;
}

}  // namespace dawn::native::opengl
