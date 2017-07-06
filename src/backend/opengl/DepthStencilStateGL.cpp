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

#include "backend/opengl/DepthStencilStateGL.h"

#include "backend/opengl/OpenGLBackend.h"
#include "backend/opengl/PersistentPipelineStateGL.h"

namespace backend {
namespace opengl {

    namespace {
        GLuint OpenGLCompareFunction(nxt::CompareFunction compareFunction) {
            switch (compareFunction) {
                case nxt::CompareFunction::Never:
                    return GL_NEVER;
                case nxt::CompareFunction::Less:
                    return GL_LESS;
                case nxt::CompareFunction::LessEqual:
                    return GL_LEQUAL;
                case nxt::CompareFunction::Greater:
                    return GL_GREATER;
                case nxt::CompareFunction::GreaterEqual:
                    return GL_GEQUAL;
                case nxt::CompareFunction::NotEqual:
                    return GL_NOTEQUAL;
                case nxt::CompareFunction::Equal:
                    return GL_EQUAL;
                case nxt::CompareFunction::Always:
                    return GL_ALWAYS;
            }
        }

        GLuint OpenGLStencilOperation(nxt::StencilOperation stencilOperation) {
            switch (stencilOperation) {
                case nxt::StencilOperation::Keep:
                    return GL_KEEP;
                case nxt::StencilOperation::Zero:
                    return GL_ZERO;
                case nxt::StencilOperation::Replace:
                    return GL_REPLACE;
                case nxt::StencilOperation::Invert:
                    return GL_INVERT;
                case nxt::StencilOperation::IncrementClamp:
                    return GL_INCR;
                case nxt::StencilOperation::DecrementClamp:
                    return GL_DECR;
                case nxt::StencilOperation::IncrementWrap:
                    return GL_INCR_WRAP;
                case nxt::StencilOperation::DecrementWrap:
                    return GL_DECR_WRAP;
            }
        }
    }

    DepthStencilState::DepthStencilState(DepthStencilStateBuilder* builder)
        : DepthStencilStateBase(builder) {
    }

    void DepthStencilState::ApplyNow(PersistentPipelineState &persistentPipelineState) const {
        if (DepthTestEnabled()) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }

        auto& depthInfo = GetDepth();

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

        GLenum backCompareFunction = OpenGLCompareFunction(stencilInfo.back.compareFunction);
        GLenum frontCompareFunction = OpenGLCompareFunction(stencilInfo.front.compareFunction);
        persistentPipelineState.SetStencilFuncsAndMask(backCompareFunction, frontCompareFunction, stencilInfo.readMask);

        glStencilOpSeparate(GL_BACK,
            OpenGLStencilOperation(stencilInfo.back.stencilFail),
            OpenGLStencilOperation(stencilInfo.back.depthFail),
            OpenGLStencilOperation(stencilInfo.back.depthStencilPass)
        );
        glStencilOpSeparate(GL_FRONT,
            OpenGLStencilOperation(stencilInfo.front.stencilFail),
            OpenGLStencilOperation(stencilInfo.front.depthFail),
            OpenGLStencilOperation(stencilInfo.front.depthStencilPass)
        );

        glStencilMask(stencilInfo.writeMask);

    }

}
}
