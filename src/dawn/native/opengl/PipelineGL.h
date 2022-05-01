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

#ifndef SRC_DAWN_NATIVE_OPENGL_PIPELINEGL_H_
#define SRC_DAWN_NATIVE_OPENGL_PIPELINEGL_H_

#include <vector>

#include "dawn/native/Pipeline.h"

#include "dawn/native/PerStage.h"
#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native {
struct ProgrammableStage;
}  // namespace dawn::native

namespace dawn::native::opengl {

struct OpenGLFunctions;
class PipelineLayout;
class Sampler;

class PipelineGL {
  public:
    PipelineGL();
    ~PipelineGL();

    // For each unit a sampler is bound to we need to know if we should use filtering or not
    // because int and uint texture are only complete without filtering.
    struct SamplerUnit {
        GLuint unit;
        bool shouldUseFiltering;
    };
    const std::vector<SamplerUnit>& GetTextureUnitsForSampler(GLuint index) const;
    const std::vector<GLuint>& GetTextureUnitsForTextureView(GLuint index) const;
    GLuint GetProgramHandle() const;

  protected:
    void ApplyNow(const OpenGLFunctions& gl);
    MaybeError InitializeBase(const OpenGLFunctions& gl,
                              const PipelineLayout* layout,
                              const PerStage<ProgrammableStage>& stages);
    void DeleteProgram(const OpenGLFunctions& gl);

  private:
    GLuint mProgram;
    std::vector<std::vector<SamplerUnit>> mUnitsForSamplers;
    std::vector<std::vector<GLuint>> mUnitsForTextures;
    std::vector<GLuint> mPlaceholderSamplerUnits;
    // TODO(enga): This could live on the Device, or elsewhere, but currently it makes Device
    // destruction complex as it requires the sampler to be destroyed before the sampler cache.
    Ref<Sampler> mPlaceholderSampler;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_PIPELINEGL_H_
