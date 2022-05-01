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

#ifndef SRC_DAWN_NATIVE_OPENGL_PERSISTENTPIPELINESTATEGL_H_
#define SRC_DAWN_NATIVE_OPENGL_PERSISTENTPIPELINESTATEGL_H_

#include "dawn/native/dawn_platform.h"
#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native::opengl {

struct OpenGLFunctions;

class PersistentPipelineState {
  public:
    void SetDefaultState(const OpenGLFunctions& gl);
    void SetStencilFuncsAndMask(const OpenGLFunctions& gl,
                                GLenum stencilBackCompareFunction,
                                GLenum stencilFrontCompareFunction,
                                uint32_t stencilReadMask);
    void SetStencilReference(const OpenGLFunctions& gl, uint32_t stencilReference);

  private:
    void CallGLStencilFunc(const OpenGLFunctions& gl);

    GLenum mStencilBackCompareFunction = GL_ALWAYS;
    GLenum mStencilFrontCompareFunction = GL_ALWAYS;
    GLuint mStencilReadMask = 0xffffffff;
    GLuint mStencilReference = 0;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_PERSISTENTPIPELINESTATEGL_H_
