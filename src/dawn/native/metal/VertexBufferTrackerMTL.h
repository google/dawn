// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_METAL_VERTEXBUFFERTRACKERMTL_H_
#define SRC_DAWN_NATIVE_METAL_VERTEXBUFFERTRACKERMTL_H_

#include <algorithm>
#include <limits>

#include "src/dawn/native/metal/BufferMTL.h"
#include "src/dawn/native/metal/StorageBufferLengthTrackerMTL.h"

namespace dawn::native::metal {

// Keeps track of the dirty vertex buffer values so they can be lazily applied when we know
// all the relevant state.
class VertexBufferTracker {
  public:
    explicit VertexBufferTracker(StorageBufferLengthTracker* lengthTracker)
        : mLengthTracker(lengthTracker) {}

    void OnSetVertexBuffer(VertexBufferSlot slot, Buffer* buffer, uint64_t offset) {
        id<MTLBuffer> mtlBuffer = buffer->GetMTLBuffer();
        if (mVertexBuffers[slot] == mtlBuffer && mVertexBufferOffsets[slot] == offset) {
            return;
        }
        buffer->TrackUsage();
        mVertexBuffers[slot] = mtlBuffer;
        mVertexBufferOffsets[slot] = offset;

        DAWN_ASSERT(buffer->GetSize() >= offset);
        // The binding size for a vertex buffer must always be at least 4 so we can do clamping.
        uint64_t bindingSize = std::max(4ull, buffer->GetSize() - offset);
        // (BufferMTL reserves an extra 4 bytes for us in case we're at the very end of the buffer.)
        DAWN_ASSERT(offset + bindingSize <= buffer->GetAllocatedSize());

        // Check to make sure sizes will fit into uint32_t for the shader.
        DAWN_CHECK(bindingSize <= std::numeric_limits<uint32_t>::max());
        mVertexBufferBindingSizes[slot] = static_cast<uint32_t>(bindingSize);

        mDirtyVertexBuffers.set(slot);
    }

    void OnSetPipeline(RenderPipeline* lastPipeline, RenderPipeline* pipeline) {
        // When a new pipeline is bound we must set all the vertex buffers again because
        // they might have been offset by the pipeline layout, and they might be packed
        // differently from the previous pipeline.
        mDirtyVertexBuffers |= pipeline->GetVertexBuffersUsed();
    }

    void Apply(id<MTLRenderCommandEncoder> encoder,
               RenderPipeline* pipeline,
               bool enableVertexPulling) {
        const auto& vertexBuffersToApply = mDirtyVertexBuffers & pipeline->GetVertexBuffersUsed();

        for (VertexBufferSlot slot : vertexBuffersToApply) {
            uint32_t metalIndex = pipeline->GetMtlVertexBufferIndex(slot);

            if (enableVertexPulling) {
                // Insert lengths for vertex buffers bound as storage buffers
                mLengthTracker->data[SingleShaderStage::Vertex][metalIndex] =
                    mVertexBufferBindingSizes[slot];
                mLengthTracker->dirtyStages |= wgpu::ShaderStage::Vertex;
            }

            [encoder setVertexBuffers:&mVertexBuffers[slot]
                              offsets:&mVertexBufferOffsets[slot]
                            withRange:NSMakeRange(metalIndex, 1)];
        }

        mDirtyVertexBuffers.reset();
    }

  private:
    // All the indices in these arrays are Dawn vertex buffer indices
    VertexBufferMask mDirtyVertexBuffers;
    PerVertexBuffer<id<MTLBuffer>> mVertexBuffers{};
    PerVertexBuffer<NSUInteger> mVertexBufferOffsets{};
    PerVertexBuffer<uint32_t> mVertexBufferBindingSizes{};

    raw_ptr<StorageBufferLengthTracker> mLengthTracker;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_VERTEXBUFFERTRACKERMTL_H_
