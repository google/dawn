// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_OPENGL_PIPELINEGL_H_
#define SRC_DAWN_NATIVE_OPENGL_PIPELINEGL_H_

#include <utility>
#include <vector>

#include "dawn/common/ityp_vector.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/opengl/BindingPoint.h"
#include "dawn/native/opengl/IntegerTypes.h"
#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native {
struct ProgrammableStage;
}  // namespace dawn::native

namespace dawn::native::opengl {

struct OpenGLFunctions;
class PipelineLayout;
class Sampler;
class Buffer;
class TextureView;

class PipelineGL {
  public:
    PipelineGL();
    ~PipelineGL();

    const std::vector<TextureUnit>& GetTextureUnitsForSampler(FlatBindingIndex index) const;
    const std::vector<TextureUnit>& GetTextureUnitsForTextureView(FlatBindingIndex index) const;
    GLuint GetProgramHandle() const;

    const BindingPointToFunctionAndOffset& GetBindingPointBuiltinDataInfo() const;
    bool NeedsTextureBuiltinUniformBuffer() const;

    bool NeedsSSBOLengthUniformBuffer() const;

  protected:
    MaybeError ApplyNow(const OpenGLFunctions& gl, const PipelineLayout* layout);
    MaybeError InitializeBase(const OpenGLFunctions& gl,
                              const PipelineLayout* layout,
                              const PerStage<ProgrammableStage>& stages,
                              bool usesVertexIndex,
                              bool usesInstanceIndex,
                              bool usesFragDepth,
                              VertexAttributeMask bgraSwizzleAttributes);
    void DeleteProgram(const OpenGLFunctions& gl);

  private:
    GLuint mProgram;
    ityp::vector<FlatBindingIndex, std::vector<TextureUnit>> mUnitsForSamplers;
    ityp::vector<FlatBindingIndex, std::vector<TextureUnit>> mUnitsForTextures;
    std::vector<TextureUnit> mPlaceholderSamplerUnits;
    // TODO(enga): This could live on the Device, or elsewhere, but currently it makes Device
    // destruction complex as it requires the sampler to be destroyed before the sampler cache.
    Ref<Sampler> mPlaceholderSampler;

    // Flag indicates if this pipeline has ssbo.length and need to use the array length from uniform
    // workaround.
    bool mNeedsSSBOLengthUniformBuffer = false;

    // Reflect info from tint: a map from texture binding point to extra data need to push into the
    // internal uniform buffer.
    BindingPointToFunctionAndOffset mBindingPointEmulatedBuiltins;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_PIPELINEGL_H_
