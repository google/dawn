// Copyright 2018 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_SHADERMODULEVK_H_
#define SRC_DAWN_NATIVE_VULKAN_SHADERMODULEVK_H_

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include "dawn/common/HashUtils.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/ShaderModule.h"

namespace dawn::native {

struct ProgrammableStage;

namespace vulkan {

struct TransformedShaderModuleCacheKey {
    const PipelineLayoutBase* layout;
    std::string entryPoint;
    PipelineConstantEntries constants;

    bool operator==(const TransformedShaderModuleCacheKey& other) const;
};

struct TransformedShaderModuleCacheKeyHashFunc {
    size_t operator()(const TransformedShaderModuleCacheKey& key) const;
};

class Device;
class PipelineLayout;

class ShaderModule final : public ShaderModuleBase {
  public:
    struct ModuleAndSpirv {
        VkShaderModule module;
        const uint32_t* spirv;
        size_t wordCount;
        const char* remappedEntryPoint;
    };

    static ResultOrError<Ref<ShaderModule>> Create(Device* device,
                                                   const ShaderModuleDescriptor* descriptor,
                                                   ShaderModuleParseResult* parseResult,
                                                   OwnedCompilationMessages* compilationMessages);

    ResultOrError<ModuleAndSpirv> GetHandleAndSpirv(SingleShaderStage stage,
                                                    const ProgrammableStage& programmableStage,
                                                    const PipelineLayout* layout,
                                                    bool clampFragDepth);

  private:
    ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor);
    ~ShaderModule() override;
    MaybeError Initialize(ShaderModuleParseResult* parseResult,
                          OwnedCompilationMessages* compilationMessages);
    void DestroyImpl() override;

    // New handles created by GetHandleAndSpirv at pipeline creation time.
    class ConcurrentTransformedShaderModuleCache;
    std::unique_ptr<ConcurrentTransformedShaderModuleCache> mTransformedShaderModuleCache;
};

}  // namespace vulkan

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_VULKAN_SHADERMODULEVK_H_
