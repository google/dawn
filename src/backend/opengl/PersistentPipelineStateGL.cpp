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

#include "PersistentPipelineStateGL.h"

#include "OpenGLBackend.h"

namespace backend {
namespace opengl {

    void PersistentPipelineState::SetDefaultState() {
        stencilBackCompareFunction = GL_ALWAYS;
        stencilFrontCompareFunction = GL_ALWAYS;
        stencilReadMask = 0xff;
        SetStencilReference(0);
    }

    void PersistentPipelineState::CacheStencilFuncsAndMask(GLenum stencilBackCompareFunction, GLenum stencilFrontCompareFunction, uint32_t stencilReadMask) {
        this->stencilBackCompareFunction = stencilBackCompareFunction;
        this->stencilFrontCompareFunction = stencilFrontCompareFunction;
        this->stencilReadMask = stencilReadMask;
    }

    void PersistentPipelineState::SetStencilReference(uint32_t stencilReference) {
        if (this->stencilReference != stencilReference) {
            this->stencilReference = stencilReference;
            glStencilFuncSeparate(GL_BACK,
                stencilBackCompareFunction,
                stencilReference,
                stencilReadMask
            );
            glStencilFuncSeparate(GL_FRONT,
                stencilFrontCompareFunction,
                stencilReference,
                stencilReadMask
            );
        }
    }

    GLuint PersistentPipelineState::GetCachedStencilReference() const {
        return stencilReference;
    }

}
}
