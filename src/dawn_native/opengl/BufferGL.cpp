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
        glGenBuffers(1, &mBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glBufferData(GL_ARRAY_BUFFER, GetSize(), nullptr, GL_STATIC_DRAW);
    }

    Buffer::~Buffer() {
        DestroyImpl();
    }

    GLuint Buffer::GetHandle() const {
        return mBuffer;
    }

    MaybeError Buffer::SetSubDataImpl(uint32_t start, uint32_t count, const uint8_t* data) {
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, start, count, data);
        return {};
    }

    void Buffer::MapReadAsyncImpl(uint32_t serial) {
        // TODO(cwallez@chromium.org): this does GPU->CPU synchronization, we could require a high
        // version of OpenGL that would let us map the buffer unsynchronized.
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        void* data = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
        CallMapReadCallback(serial, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, data, GetSize());
    }

    void Buffer::MapWriteAsyncImpl(uint32_t serial) {
        // TODO(cwallez@chromium.org): this does GPU->CPU synchronization, we could require a high
        // version of OpenGL that would let us map the buffer unsynchronized.
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        void* data = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        CallMapWriteCallback(serial, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, data, GetSize());
    }

    void Buffer::UnmapImpl() {
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    void Buffer::DestroyImpl() {
        glDeleteBuffers(1, &mBuffer);
        mBuffer = 0;
    }

}}  // namespace dawn_native::opengl
