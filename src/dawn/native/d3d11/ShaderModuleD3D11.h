// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D11_SHADERMODULED3D11_H_
#define SRC_DAWN_NATIVE_D3D11_SHADERMODULED3D11_H_

#include <optional>
#include <string>

#include "dawn/native/Blob.h"
#include "dawn/native/Serializable.h"
#include "dawn/native/ShaderModule.h"
#include "dawn/native/d3d/ShaderUtils.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native {
struct ProgrammableStage;
}  // namespace dawn::native

namespace dawn::native::d3d11 {

class Device;
class PipelineLayout;

class ShaderModule final : public ShaderModuleBase {
  public:
    static ResultOrError<Ref<ShaderModule>> Create(Device* device,
                                                   const ShaderModuleDescriptor* descriptor,
                                                   ShaderModuleParseResult* parseResult,
                                                   OwnedCompilationMessages* compilationMessages);

    ResultOrError<d3d::CompiledShader> Compile(
        const ProgrammableStage& programmableStage,
        SingleShaderStage stage,
        const PipelineLayout* layout,
        uint32_t compileFlags,
        const std::optional<dawn::native::d3d::InterStageShaderVariablesMask>&
            usedInterstageVariables = {});

  private:
    ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor);
    ~ShaderModule() override = default;
    MaybeError Initialize(ShaderModuleParseResult* parseResult,
                          OwnedCompilationMessages* compilationMessages);
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_SHADERMODULED3D11_H_
