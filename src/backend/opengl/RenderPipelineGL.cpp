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

#include "backend/opengl/RenderPipelineGL.h"

#include "backend/opengl/BlendStateGL.h"
#include "backend/opengl/DepthStencilStateGL.h"
#include "backend/opengl/OpenGLBackend.h"
#include "backend/opengl/PersistentPipelineStateGL.h"

namespace backend {
namespace opengl {

    namespace {
        GLenum GLPrimitiveTopology(nxt::PrimitiveTopology primitiveTopology) {
            switch (primitiveTopology) {
                case nxt::PrimitiveTopology::PointList:
                    return GL_POINTS;
                case nxt::PrimitiveTopology::LineList:
                    return GL_LINES;
                case nxt::PrimitiveTopology::LineStrip:
                    return GL_LINE_STRIP;
                case nxt::PrimitiveTopology::TriangleList:
                    return GL_TRIANGLES;
                case nxt::PrimitiveTopology::TriangleStrip:
                    return GL_TRIANGLE_STRIP;
                default:
                    UNREACHABLE();
            }
        }
    }

    RenderPipeline::RenderPipeline(RenderPipelineBuilder* builder)
        : RenderPipelineBase(builder), PipelineGL(this, builder),
          glPrimitiveTopology(GLPrimitiveTopology(GetPrimitiveTopology())) {
    }

    GLenum RenderPipeline::GetGLPrimitiveTopology() const {
        return glPrimitiveTopology;
    }

    void RenderPipeline::ApplyNow(PersistentPipelineState &persistentPipelineState) {
        PipelineGL::ApplyNow();

        auto inputState = ToBackend(GetInputState());
        glBindVertexArray(inputState->GetVAO());

        auto depthStencilState = ToBackend(GetDepthStencilState());
        depthStencilState->ApplyNow(persistentPipelineState);


        RenderPass* renderPass = ToBackend(GetRenderPass());
        auto& subpassInfo = renderPass->GetSubpassInfo(GetSubPass());

        for (uint32_t attachmentSlot : IterateBitSet(subpassInfo.colorAttachmentsSet)) {
            ToBackend(GetBlendState(attachmentSlot))->ApplyNow(attachmentSlot);
        }
    }

}
}
