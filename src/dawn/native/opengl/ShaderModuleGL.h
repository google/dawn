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

#ifndef SRC_DAWN_NATIVE_OPENGL_SHADERMODULEGL_H_
#define SRC_DAWN_NATIVE_OPENGL_SHADERMODULEGL_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "dawn/native/Serializable.h"
#include "dawn/native/ShaderModule.h"
#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native {

struct ProgrammableStage;

namespace stream {
class Sink;
class Source;
}  // namespace stream

namespace opengl {

class Device;
class PipelineLayout;
struct OpenGLFunctions;

std::string GetBindingName(BindGroupIndex group, BindingNumber bindingNumber);

#define BINDING_LOCATION_MEMBERS(X) \
    X(BindGroupIndex, group)        \
    X(BindingNumber, binding)
DAWN_SERIALIZABLE(struct, BindingLocation, BINDING_LOCATION_MEMBERS){};
#undef BINDING_LOCATION_MEMBERS

bool operator<(const BindingLocation& a, const BindingLocation& b);

#define COMBINED_SAMPLER_MEMBERS(X)                                                         \
    X(BindingLocation, samplerLocation)                                                     \
    X(BindingLocation, textureLocation)                                                     \
    /* OpenGL requires a sampler with texelFetch. If this is true, the developer did not */ \
    /* provide one and Dawn should bind a placeholder non-filtering sampler;  */            \
    /* |samplerLocation| is unused. */                                                      \
    X(bool, usePlaceholderSampler)

DAWN_SERIALIZABLE(struct, CombinedSampler, COMBINED_SAMPLER_MEMBERS) {
    std::string GetName() const;
};
#undef COMBINED_SAMPLER_MEMBERS

bool operator<(const CombinedSampler& a, const CombinedSampler& b);

using CombinedSamplerInfo = std::vector<CombinedSampler>;

class ShaderModule final : public ShaderModuleBase {
  public:
    static ResultOrError<Ref<ShaderModule>> Create(Device* device,
                                                   const ShaderModuleDescriptor* descriptor,
                                                   ShaderModuleParseResult* parseResult,
                                                   OwnedCompilationMessages* compilationMessages);

    ResultOrError<GLuint> CompileShader(const OpenGLFunctions& gl,
                                        const ProgrammableStage& programmableStage,
                                        SingleShaderStage stage,
                                        CombinedSamplerInfo* combinedSamplers,
                                        const PipelineLayout* layout,
                                        bool* needsPlaceholderSampler) const;

  private:
    ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor);
    ~ShaderModule() override = default;
    MaybeError Initialize(ShaderModuleParseResult* parseResult,
                          OwnedCompilationMessages* compilationMessages);
};

}  // namespace opengl
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_OPENGL_SHADERMODULEGL_H_
