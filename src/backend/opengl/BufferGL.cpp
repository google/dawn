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

#include "backend/opengl/BufferGL.h"

#include "backend/opengl/OpenGLBackend.h"

namespace backend { namespace opengl {

    // Buffer

    Buffer::Buffer(BufferBuilder* builder) : BufferBase(builder) {
        glGenBuffers(1, &mBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glBufferData(GL_ARRAY_BUFFER, GetSize(), nullptr, GL_STATIC_DRAW);
    }

    GLuint Buffer::GetHandle() const {
        return mBuffer;
    }

    void Buffer::SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) {
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, start * sizeof(uint32_t), count * sizeof(uint32_t), data);
    }

    void Buffer::MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) {
        // TODO(cwallez@chromium.org): this does GPU->CPU synchronization, we could require a high
        // version of OpenGL that would let us map the buffer unsynchronized.
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        void* data = glMapBufferRange(GL_ARRAY_BUFFER, start, count, GL_MAP_READ_BIT);
        CallMapReadCallback(serial, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, data);
    }

    void Buffer::MapWriteAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) {
        // TODO(cwallez@chromium.org): this does GPU->CPU synchronization, we could require a high
        // version of OpenGL that would let us map the buffer unsynchronized.
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        void* data = glMapBufferRange(GL_ARRAY_BUFFER, start, count, GL_MAP_WRITE_BIT);
        CallMapWriteCallback(serial, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, data);
    }

    void Buffer::UnmapImpl() {
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    void Buffer::TransitionUsageImpl(nxt::BufferUsageBit, nxt::BufferUsageBit) {
    }

    // BufferView

    BufferView::BufferView(BufferViewBuilder* builder) : BufferViewBase(builder) {
    }

}}  // namespace backend::opengl
