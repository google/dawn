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

#ifndef BACKEND_OPENGL_PERSISTENTPIPELINESTATEGL_H_
#define BACKEND_OPENGL_PERSISTENTPIPELINESTATEGL_H_

#include "dawn/dawncpp.h"

#include "glad/glad.h"

namespace backend { namespace opengl {

    class PersistentPipelineState {
      public:
        void SetDefaultState();
        void SetStencilFuncsAndMask(GLenum stencilBackCompareFunction,
                                    GLenum stencilFrontCompareFunction,
                                    uint32_t stencilReadMask);
        void SetStencilReference(uint32_t stencilReference);

      private:
        void CallGLStencilFunc();

        GLenum mStencilBackCompareFunction = GL_ALWAYS;
        GLenum mStencilFrontCompareFunction = GL_ALWAYS;
        GLuint mStencilReadMask = 0xffffffff;
        GLuint mStencilReference = 0;
    };

}}  // namespace backend::opengl

#endif  // BACKEND_OPENGL_PERSISTENTPIPELINESTATEGL_H_
