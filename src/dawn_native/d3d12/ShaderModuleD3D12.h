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

#ifndef DAWNNATIVE_D3D12_SHADERMODULED3D12_H_
#define DAWNNATIVE_D3D12_SHADERMODULED3D12_H_

#include "dawn_native/PersistentCache.h"
#include "dawn_native/ShaderModule.h"

#include "dawn_native/d3d12/d3d12_platform.h"

namespace dawn_native { namespace d3d12 {

    class Device;
    class PipelineLayout;

    // Manages a ref to one of the various representations of shader blobs.
    struct CompiledShader {
        ScopedCachedBlob cachedShader;
        ComPtr<ID3DBlob> compiledFXCShader;
        ComPtr<IDxcBlob> compiledDXCShader;
        D3D12_SHADER_BYTECODE GetD3D12ShaderBytecode() const;
    };

    class ShaderModule final : public ShaderModuleBase {
      public:
        static ResultOrError<ShaderModule*> Create(Device* device,
                                                   const ShaderModuleDescriptor* descriptor,
                                                   ShaderModuleParseResult* parseResult);

        ResultOrError<CompiledShader> Compile(const char* entryPointName,
                                              SingleShaderStage stage,
                                              PipelineLayout* layout,
                                              uint32_t compileFlags);

      private:
        ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor);
        ~ShaderModule() override = default;
        MaybeError Initialize(ShaderModuleParseResult* parseResult);

        ResultOrError<std::string> TranslateToHLSLWithTint(
            const char* entryPointName,
            SingleShaderStage stage,
            PipelineLayout* layout,
            std::string* remappedEntryPointName) const;

        ResultOrError<std::string> TranslateToHLSLWithSPIRVCross(const char* entryPointName,
                                                                 SingleShaderStage stage,
                                                                 PipelineLayout* layout) const;

        ResultOrError<PersistentCacheKey> CreateHLSLKey(const char* entryPointName,
                                                        SingleShaderStage stage,
                                                        const std::string& hlslSource,
                                                        uint32_t compileFlags) const;

        ResultOrError<uint64_t> GetDXCompilerVersion() const;
        uint64_t GetD3DCompilerVersion() const;

#ifdef DAWN_ENABLE_WGSL
        std::unique_ptr<tint::ast::Module> mTintModule;
#endif
    };

}}  // namespace dawn_native::d3d12

#endif  // DAWNNATIVE_D3D12_SHADERMODULED3D12_H_
