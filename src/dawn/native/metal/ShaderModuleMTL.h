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

#ifndef SRC_DAWN_NATIVE_METAL_SHADERMODULEMTL_H_
#define SRC_DAWN_NATIVE_METAL_SHADERMODULEMTL_H_

#include <string>
#include <vector>

#include "dawn/native/ShaderModule.h"

#include "dawn/common/NSRef.h"
#include "dawn/native/Error.h"

#import <Metal/Metal.h>

namespace dawn::native::metal {

class Device;
class PipelineLayout;
class RenderPipeline;

class ShaderModule final : public ShaderModuleBase {
  public:
    static ResultOrError<Ref<ShaderModule>> Create(Device* device,
                                                   const ShaderModuleDescriptor* descriptor,
                                                   ShaderModuleParseResult* parseResult,
                                                   OwnedCompilationMessages* compilationMessages);

    struct MetalFunctionData {
        NSPRef<id<MTLFunction>> function;
        bool needsStorageBufferLength;
        std::vector<uint32_t> workgroupAllocations;
    };

    // MTLFunctionConstantValues needs @available tag to compile
    // Use id (like void*) in function signature as workaround and do static cast inside
    MaybeError CreateFunction(const char* entryPointName,
                              SingleShaderStage stage,
                              const PipelineLayout* layout,
                              MetalFunctionData* out,
                              id constantValues = nil,
                              uint32_t sampleMask = 0xFFFFFFFF,
                              const RenderPipeline* renderPipeline = nullptr);

  private:
    ResultOrError<std::string> TranslateToMSL(const char* entryPointName,
                                              SingleShaderStage stage,
                                              const PipelineLayout* layout,
                                              uint32_t sampleMask,
                                              const RenderPipeline* renderPipeline,
                                              std::string* remappedEntryPointName,
                                              bool* needsStorageBufferLength,
                                              bool* hasInvariantAttribute,
                                              std::vector<uint32_t>* workgroupAllocations);
    ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor);
    ~ShaderModule() override;
    MaybeError Initialize(ShaderModuleParseResult* parseResult,
                          OwnedCompilationMessages* compilationMessages);
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_SHADERMODULEMTL_H_
