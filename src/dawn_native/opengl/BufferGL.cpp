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

#include "dawn_native/opengl/BufferGL.h"

#include "dawn_native/opengl/DeviceGL.h"

namespace dawn_native { namespace opengl {

    // Buffer

    Buffer::Buffer(Device* device, const BufferDescriptor* descriptor)
        : BufferBase(device, descriptor) {
        // TODO(cwallez@chromium.org): Have a global "zero" buffer instead of creating a new 4-byte
        // buffer?
        uint64_t size = GetAppliedSize();

        device->gl.GenBuffers(1, &mBuffer);
        device->gl.BindBuffer(GL_ARRAY_BUFFER, mBuffer);

        if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
            std::vector<uint8_t> clearValues(size, 1u);
            device->gl.BufferData(GL_ARRAY_BUFFER, size, clearValues.data(), GL_STATIC_DRAW);
        } else {
            device->gl.BufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
        }
    }

    Buffer::~Buffer() {
        DestroyInternal();
    }

    GLuint Buffer::GetHandle() const {
        return mBuffer;
    }

    uint64_t Buffer::GetAppliedSize() const {
        // TODO(cwallez@chromium.org): Have a global "zero" buffer instead of creating a new 4-byte
        // buffer?
        return std::max(GetSize(), uint64_t(4u));
    }

    void Buffer::EnsureDataInitialized() {
        // TODO(jiawei.shao@intel.com): check Toggle::LazyClearResourceOnFirstUse
        // instead when buffer lazy initialization is completely supported.
        if (IsDataInitialized() ||
            !GetDevice()->IsToggleEnabled(Toggle::LazyClearBufferOnFirstUse)) {
            return;
        }

        InitializeToZero();
    }

    void Buffer::EnsureDataInitializedAsDestination(uint64_t offset, uint64_t size) {
        // TODO(jiawei.shao@intel.com): check Toggle::LazyClearResourceOnFirstUse
        // instead when buffer lazy initialization is completely supported.
        if (IsDataInitialized() ||
            !GetDevice()->IsToggleEnabled(Toggle::LazyClearBufferOnFirstUse)) {
            return;
        }

        if (IsFullBufferRange(offset, size)) {
            SetIsDataInitialized();
        } else {
            InitializeToZero();
        }
    }

    void Buffer::InitializeToZero() {
        ASSERT(GetDevice()->IsToggleEnabled(Toggle::LazyClearBufferOnFirstUse));
        ASSERT(!IsDataInitialized());

        const uint64_t size = GetAppliedSize();
        Device* device = ToBackend(GetDevice());

        const std::vector<uint8_t> clearValues(size, 0u);
        device->gl.BindBuffer(GL_ARRAY_BUFFER, mBuffer);
        device->gl.BufferSubData(GL_ARRAY_BUFFER, 0, size, clearValues.data());
        device->IncrementLazyClearCountForTesting();

        SetIsDataInitialized();
    }

    bool Buffer::IsMappableAtCreation() const {
        // TODO(enga): All buffers in GL can be mapped. Investigate if mapping them will cause the
        // driver to migrate it to shared memory.
        return true;
    }

    MaybeError Buffer::MapAtCreationImpl() {
        const OpenGLFunctions& gl = ToBackend(GetDevice())->gl;
        gl.BindBuffer(GL_ARRAY_BUFFER, mBuffer);
        mMappedData = gl.MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        return {};
    }

    MaybeError Buffer::MapReadAsyncImpl() {
        const OpenGLFunctions& gl = ToBackend(GetDevice())->gl;

        // TODO(cwallez@chromium.org): this does GPU->CPU synchronization, we could require a high
        // version of OpenGL that would let us map the buffer unsynchronized.
        gl.BindBuffer(GL_ARRAY_BUFFER, mBuffer);
        mMappedData = gl.MapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
        return {};
    }

    MaybeError Buffer::MapWriteAsyncImpl() {
        const OpenGLFunctions& gl = ToBackend(GetDevice())->gl;

        // TODO(cwallez@chromium.org): this does GPU->CPU synchronization, we could require a high
        // version of OpenGL that would let us map the buffer unsynchronized.
        gl.BindBuffer(GL_ARRAY_BUFFER, mBuffer);
        mMappedData = gl.MapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        return {};
    }

    void* Buffer::GetMappedPointerImpl() {
        return mMappedData;
    }

    void Buffer::UnmapImpl() {
        const OpenGLFunctions& gl = ToBackend(GetDevice())->gl;

        gl.BindBuffer(GL_ARRAY_BUFFER, mBuffer);
        gl.UnmapBuffer(GL_ARRAY_BUFFER);
        mMappedData = nullptr;
    }

    void Buffer::DestroyImpl() {
        ToBackend(GetDevice())->gl.DeleteBuffers(1, &mBuffer);
        mBuffer = 0;
    }

}}  // namespace dawn_native::opengl
