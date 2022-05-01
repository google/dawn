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
