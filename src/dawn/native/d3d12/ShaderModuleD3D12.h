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

#ifndef SRC_DAWN_NATIVE_D3D12_SHADERMODULED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_SHADERMODULED3D12_H_

#include "dawn/native/ShaderModule.h"

#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native {
struct ProgrammableStage;
}  // namespace dawn::native

namespace dawn::native::d3d12 {

class Device;
class PipelineLayout;

// Manages a ref to one of the various representations of shader blobs and information used to
// emulate vertex/instance index starts
struct CompiledShader {
    ComPtr<ID3DBlob> compiledFXCShader;
    ComPtr<IDxcBlob> compiledDXCShader;
    D3D12_SHADER_BYTECODE GetD3D12ShaderBytecode() const;

    bool usesVertexOrInstanceIndex;
};

class ShaderModule final : public ShaderModuleBase {
  public:
    static ResultOrError<Ref<ShaderModule>> Create(Device* device,
                                                   const ShaderModuleDescriptor* descriptor,
                                                   ShaderModuleParseResult* parseResult,
                                                   OwnedCompilationMessages* compilationMessages);

    ResultOrError<CompiledShader> Compile(const ProgrammableStage& programmableStage,
                                          SingleShaderStage stage,
                                          const PipelineLayout* layout,
                                          uint32_t compileFlags);

  private:
    ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor);
    ~ShaderModule() override = default;
    MaybeError Initialize(ShaderModuleParseResult* parseResult,
                          OwnedCompilationMessages* compilationMessages);
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_SHADERMODULED3D12_H_
