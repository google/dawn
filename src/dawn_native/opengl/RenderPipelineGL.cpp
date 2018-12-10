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

#include "dawn_native/opengl/RenderPipelineGL.h"

#include "dawn_native/opengl/BlendStateGL.h"
#include "dawn_native/opengl/DepthStencilStateGL.h"
#include "dawn_native/opengl/DeviceGL.h"
#include "dawn_native/opengl/Forward.h"
#include "dawn_native/opengl/InputStateGL.h"
#include "dawn_native/opengl/PersistentPipelineStateGL.h"

namespace dawn_native { namespace opengl {

    namespace {
        GLenum GLPrimitiveTopology(dawn::PrimitiveTopology primitiveTopology) {
            switch (primitiveTopology) {
                case dawn::PrimitiveTopology::PointList:
                    return GL_POINTS;
                case dawn::PrimitiveTopology::LineList:
                    return GL_LINES;
                case dawn::PrimitiveTopology::LineStrip:
                    return GL_LINE_STRIP;
                case dawn::PrimitiveTopology::TriangleList:
                    return GL_TRIANGLES;
                case dawn::PrimitiveTopology::TriangleStrip:
                    return GL_TRIANGLE_STRIP;
                default:
                    UNREACHABLE();
            }
        }
    }  // namespace

    RenderPipeline::RenderPipeline(Device* device, const RenderPipelineDescriptor* descriptor)
        : RenderPipelineBase(device, descriptor),
          mGlPrimitiveTopology(GLPrimitiveTopology(GetPrimitiveTopology())) {
        PerStage<const ShaderModule*> modules(nullptr);
        modules[dawn::ShaderStage::Vertex] = ToBackend(descriptor->vertexStage->module);
        modules[dawn::ShaderStage::Fragment] = ToBackend(descriptor->fragmentStage->module);

        PipelineGL::Initialize(ToBackend(GetLayout()), modules);
    }

    GLenum RenderPipeline::GetGLPrimitiveTopology() const {
        return mGlPrimitiveTopology;
    }

    void RenderPipeline::ApplyNow(PersistentPipelineState& persistentPipelineState) {
        PipelineGL::ApplyNow();

        auto inputState = ToBackend(GetInputState());
        glBindVertexArray(inputState->GetVAO());

        auto depthStencilState = ToBackend(GetDepthStencilState());
        depthStencilState->ApplyNow(persistentPipelineState);

        for (uint32_t attachmentSlot : IterateBitSet(GetColorAttachmentsMask())) {
            ToBackend(GetBlendState(attachmentSlot))->ApplyNow(attachmentSlot);
        }
    }

}}  // namespace dawn_native::opengl
