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

namespace dawn::native {
struct ProgrammableStage;
}

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
        MTLSize localWorkgroupSize;
    };

    MaybeError CreateFunction(SingleShaderStage stage,
                              const ProgrammableStage& programmableStage,
                              const PipelineLayout* layout,
                              MetalFunctionData* out,
                              uint32_t sampleMask = 0xFFFFFFFF,
                              const RenderPipeline* renderPipeline = nullptr);

  private:
    ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor);
    ~ShaderModule() override;
    MaybeError Initialize(ShaderModuleParseResult* parseResult,
                          OwnedCompilationMessages* compilationMessages);
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_SHADERMODULEMTL_H_
