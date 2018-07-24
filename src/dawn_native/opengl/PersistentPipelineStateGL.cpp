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

#include "dawn_native/opengl/PersistentPipelineStateGL.h"

namespace dawn_native { namespace opengl {

    void PersistentPipelineState::SetDefaultState() {
        CallGLStencilFunc();
    }

    void PersistentPipelineState::SetStencilFuncsAndMask(GLenum stencilBackCompareFunction,
                                                         GLenum stencilFrontCompareFunction,
                                                         uint32_t stencilReadMask) {
        if (mStencilBackCompareFunction == stencilBackCompareFunction &&
            mStencilFrontCompareFunction == stencilFrontCompareFunction &&
            mStencilReadMask == stencilReadMask) {
            return;
        }

        mStencilBackCompareFunction = stencilBackCompareFunction;
        mStencilFrontCompareFunction = stencilFrontCompareFunction;
        mStencilReadMask = stencilReadMask;
        CallGLStencilFunc();
    }

    void PersistentPipelineState::SetStencilReference(uint32_t stencilReference) {
        if (mStencilReference == stencilReference) {
            return;
        }

        mStencilReference = stencilReference;
        CallGLStencilFunc();
    }

    void PersistentPipelineState::CallGLStencilFunc() {
        glStencilFuncSeparate(GL_BACK, mStencilBackCompareFunction, mStencilReference,
                              mStencilReadMask);
        glStencilFuncSeparate(GL_FRONT, mStencilFrontCompareFunction, mStencilReference,
                              mStencilReadMask);
    }

}}  // namespace dawn_native::opengl
