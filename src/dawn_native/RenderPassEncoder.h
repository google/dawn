// Copyright 2018 The Dawn Authors
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

#ifndef DAWNNATIVE_RENDERPASSENCODER_H_
#define DAWNNATIVE_RENDERPASSENCODER_H_

#include "dawn_native/Error.h"
#include "dawn_native/ProgrammablePassEncoder.h"

namespace dawn_native {

    // This is called RenderPassEncoderBase to match the code generator expectations. Note that it
    // is a pure frontend type to record in its parent CommandBufferBuilder and never has a backend
    // implementation.
    // TODO(cwallez@chromium.org): Remove that generator limitation and rename to ComputePassEncoder
    class RenderPassEncoderBase : public ProgrammablePassEncoder {
      public:
        RenderPassEncoderBase(DeviceBase* device,
                              CommandBufferBuilder* topLevelBuilder,
                              CommandAllocator* allocator);

        void DrawArrays(uint32_t vertexCount,
                        uint32_t instanceCount,
                        uint32_t firstVertex,
                        uint32_t firstInstance);
        void DrawElements(uint32_t vertexCount,
                          uint32_t instanceCount,
                          uint32_t firstIndex,
                          uint32_t firstInstance);

        void SetRenderPipeline(RenderPipelineBase* pipeline);

        void SetStencilReference(uint32_t reference);
        void SetBlendColor(float r, float g, float b, float a);
        void SetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

        template <typename T>
        void SetVertexBuffers(uint32_t startSlot,
                              uint32_t count,
                              T* const* buffers,
                              uint32_t const* offsets) {
            static_assert(std::is_base_of<BufferBase, T>::value, "");
            SetVertexBuffers(startSlot, count, reinterpret_cast<BufferBase* const*>(buffers),
                             offsets);
        }
        void SetVertexBuffers(uint32_t startSlot,
                              uint32_t count,
                              BufferBase* const* buffers,
                              uint32_t const* offsets);
        void SetIndexBuffer(BufferBase* buffer, uint32_t offset);
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_RENDERPASSENCODER_H_
