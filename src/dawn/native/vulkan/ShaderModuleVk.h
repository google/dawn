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

#include "dawn/native/ShaderModule.h"

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"

#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

namespace dawn::native::vulkan {

    class Device;
    class PipelineLayout;

    class ShaderModule final : public ShaderModuleBase {
      public:
        using Spirv = std::vector<uint32_t>;
        using ModuleAndSpirv = std::pair<VkShaderModule, const Spirv*>;

        static ResultOrError<Ref<ShaderModule>> Create(Device* device,
                                                       const ShaderModuleDescriptor* descriptor,
                                                       ShaderModuleParseResult* parseResult);

        ResultOrError<ModuleAndSpirv> GetHandleAndSpirv(const char* entryPointName,
                                                        PipelineLayout* layout);

      private:
        ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor);
        ~ShaderModule() override;
        MaybeError Initialize(ShaderModuleParseResult* parseResult);
        void DestroyImpl() override;

        // New handles created by GetHandleAndSpirv at pipeline creation time.
        class ConcurrentTransformedShaderModuleCache {
          public:
            explicit ConcurrentTransformedShaderModuleCache(Device* device);
            ~ConcurrentTransformedShaderModuleCache();

            std::optional<ModuleAndSpirv> Find(const PipelineLayoutEntryPointPair& key);
            ModuleAndSpirv AddOrGet(const PipelineLayoutEntryPointPair& key,
                                    VkShaderModule module,
                                    std::vector<uint32_t>&& spirv);

          private:
            using Entry = std::pair<VkShaderModule, std::unique_ptr<Spirv>>;

            Device* mDevice;
            std::mutex mMutex;
            std::unordered_map<PipelineLayoutEntryPointPair,
                               Entry,
                               PipelineLayoutEntryPointPairHashFunc>
                mTransformedShaderModuleCache;
        };
        std::unique_ptr<ConcurrentTransformedShaderModuleCache> mTransformedShaderModuleCache;
    };

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_SHADERMODULEVK_H_
