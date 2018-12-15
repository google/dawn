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

#include "dawn_native/opengl/DepthStencilStateGL.h"

#include "common/Assert.h"
#include "dawn_native/opengl/PersistentPipelineStateGL.h"

namespace dawn_native { namespace opengl {

    namespace {
        GLuint OpenGLCompareFunction(dawn::CompareFunction compareFunction) {
            switch (compareFunction) {
                case dawn::CompareFunction::Never:
                    return GL_NEVER;
                case dawn::CompareFunction::Less:
                    return GL_LESS;
                case dawn::CompareFunction::LessEqual:
                    return GL_LEQUAL;
                case dawn::CompareFunction::Greater:
                    return GL_GREATER;
                case dawn::CompareFunction::GreaterEqual:
                    return GL_GEQUAL;
                case dawn::CompareFunction::NotEqual:
                    return GL_NOTEQUAL;
                case dawn::CompareFunction::Equal:
                    return GL_EQUAL;
                case dawn::CompareFunction::Always:
                    return GL_ALWAYS;
                default:
                    UNREACHABLE();
            }
        }

        GLuint OpenGLStencilOperation(dawn::StencilOperation stencilOperation) {
            switch (stencilOperation) {
                case dawn::StencilOperation::Keep:
                    return GL_KEEP;
                case dawn::StencilOperation::Zero:
                    return GL_ZERO;
                case dawn::StencilOperation::Replace:
                    return GL_REPLACE;
                case dawn::StencilOperation::Invert:
                    return GL_INVERT;
                case dawn::StencilOperation::IncrementClamp:
                    return GL_INCR;
                case dawn::StencilOperation::DecrementClamp:
                    return GL_DECR;
                case dawn::StencilOperation::IncrementWrap:
                    return GL_INCR_WRAP;
                case dawn::StencilOperation::DecrementWrap:
                    return GL_DECR_WRAP;
                default:
                    UNREACHABLE();
            }
        }
    }  // namespace

    DepthStencilState::DepthStencilState(DepthStencilStateBuilder* builder)
        : DepthStencilStateBase(builder) {
    }

    void DepthStencilState::ApplyNow(PersistentPipelineState& persistentPipelineState) const {
        auto& depthInfo = GetDepth();

        // Depth writes only occur if depth is enabled
        if (depthInfo.compareFunction == dawn::CompareFunction::Always &&
            !depthInfo.depthWriteEnabled) {
            glDisable(GL_DEPTH_TEST);
        } else {
            glEnable(GL_DEPTH_TEST);
        }

        if (depthInfo.depthWriteEnabled) {
            glDepthMask(GL_TRUE);
        } else {
            glDepthMask(GL_FALSE);
        }

        glDepthFunc(OpenGLCompareFunction(depthInfo.compareFunction));

        if (StencilTestEnabled()) {
            glEnable(GL_STENCIL_TEST);
        } else {
            glDisable(GL_STENCIL_TEST);
        }

        auto& stencilInfo = GetStencil();

        GLenum backCompareFunction = OpenGLCompareFunction(stencilInfo.back.compare);
        GLenum frontCompareFunction = OpenGLCompareFunction(stencilInfo.front.compare);
        persistentPipelineState.SetStencilFuncsAndMask(backCompareFunction, frontCompareFunction,
                                                       stencilInfo.readMask);

        glStencilOpSeparate(GL_BACK, OpenGLStencilOperation(stencilInfo.back.stencilFailOp),
                            OpenGLStencilOperation(stencilInfo.back.depthFailOp),
                            OpenGLStencilOperation(stencilInfo.back.passOp));
        glStencilOpSeparate(GL_FRONT, OpenGLStencilOperation(stencilInfo.front.stencilFailOp),
                            OpenGLStencilOperation(stencilInfo.front.depthFailOp),
                            OpenGLStencilOperation(stencilInfo.front.passOp));

        glStencilMask(stencilInfo.writeMask);
    }

}}  // namespace dawn_native::opengl
