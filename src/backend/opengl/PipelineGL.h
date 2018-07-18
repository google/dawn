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

#ifndef BACKEND_OPENGL_PIPELINEGL_H_
#define BACKEND_OPENGL_PIPELINEGL_H_

#include "backend/Pipeline.h"

#include "glad/glad.h"

#include <vector>

namespace backend { namespace opengl {

    class Device;
    class PersistentPipelineState;
    class ShaderModule;

    class PipelineGL {
      public:
        PipelineGL(PipelineBase* parent, PipelineBuilder* builder);

        using GLPushConstantInfo = std::array<GLint, kMaxPushConstants>;
        using BindingLocations =
            std::array<std::array<GLint, kMaxBindingsPerGroup>, kMaxBindGroups>;

        const GLPushConstantInfo& GetGLPushConstants(dawn::ShaderStage stage) const;
        const std::vector<GLuint>& GetTextureUnitsForSampler(GLuint index) const;
        const std::vector<GLuint>& GetTextureUnitsForTexture(GLuint index) const;
        GLuint GetProgramHandle() const;

        void ApplyNow();

      private:
        GLuint mProgram;
        PerStage<GLPushConstantInfo> mGlPushConstants;
        std::vector<std::vector<GLuint>> mUnitsForSamplers;
        std::vector<std::vector<GLuint>> mUnitsForTextures;
    };

}}  // namespace backend::opengl

#endif  // BACKEND_OPENGL_PIPELINEGL_H_
