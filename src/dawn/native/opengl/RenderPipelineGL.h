// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_OPENGL_RENDERPIPELINEGL_H_
#define SRC_DAWN_NATIVE_OPENGL_RENDERPIPELINEGL_H_

#include <vector>

#include "dawn/native/RenderPipeline.h"

#include "dawn/native/opengl/PipelineGL.h"
#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native::opengl {

class Device;
class PersistentPipelineState;

class RenderPipeline final : public RenderPipelineBase, public PipelineGL {
  public:
    static Ref<RenderPipeline> CreateUninitialized(Device* device,
                                                   const RenderPipelineDescriptor* descriptor);

    GLenum GetGLPrimitiveTopology() const;
    ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes> GetAttributesUsingVertexBuffer(
        VertexBufferSlot slot) const;

    void ApplyNow(PersistentPipelineState& persistentPipelineState);

    MaybeError Initialize() override;

  private:
    RenderPipeline(Device* device, const RenderPipelineDescriptor* descriptor);
    ~RenderPipeline() override;
    void DestroyImpl() override;

    void CreateVAOForVertexState();

    // TODO(yunchao.he@intel.com): vao need to be deduplicated between pipelines.
    GLuint mVertexArrayObject;
    GLenum mGlPrimitiveTopology;

    ityp::array<VertexBufferSlot,
                ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes>,
                kMaxVertexBuffers>
        mAttributesUsingVertexBuffer;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_RENDERPIPELINEGL_H_
