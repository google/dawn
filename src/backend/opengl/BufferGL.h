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

#ifndef BACKEND_OPENGL_BUFFERGL_H_
#define BACKEND_OPENGL_BUFFERGL_H_

#include "backend/Buffer.h"

#include "glad/glad.h"

namespace backend {
namespace opengl {

    class Device;

    class Buffer : public BufferBase {
        public:
            Buffer(BufferBuilder* builder);

            GLuint GetHandle() const;

        private:
            void SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) override;
            void MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) override;
            void UnmapImpl() override;
            void TransitionUsageImpl(nxt::BufferUsageBit currentUsage, nxt::BufferUsageBit targetUsage) override;

            GLuint mBuffer = 0;
    };

    class BufferView : public BufferViewBase {
        public:
            BufferView(BufferViewBuilder* builder);
    };

}
}

#endif // BACKEND_OPENGL_BUFFERGL_H_
