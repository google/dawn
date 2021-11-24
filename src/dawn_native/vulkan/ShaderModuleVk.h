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

#ifndef DAWNNATIVE_VULKAN_SHADERMODULEVK_H_
#define DAWNNATIVE_VULKAN_SHADERMODULEVK_H_

#include "dawn_native/ShaderModule.h"

#include "common/vulkan_platform.h"
#include "dawn_native/Error.h"

#include <mutex>

namespace dawn_native { namespace vulkan {

    class Device;
    class PipelineLayout;

    class ShaderModule final : public ShaderModuleBase {
      public:
        static ResultOrError<Ref<ShaderModule>> Create(Device* device,
                                                       const ShaderModuleDescriptor* descriptor,
                                                       ShaderModuleParseResult* parseResult);

        ResultOrError<VkShaderModule> GetTransformedModuleHandle(const char* entryPointName,
                                                                 PipelineLayout* layout);

      private:
        ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor);
        ~ShaderModule() override;
        MaybeError Initialize(ShaderModuleParseResult* parseResult);
        void DestroyImpl() override;

        // New handles created by GetTransformedModuleHandle at pipeline creation time
        class ConcurrentTransformedShaderModuleCache {
          public:
            explicit ConcurrentTransformedShaderModuleCache(Device* device);
            ~ConcurrentTransformedShaderModuleCache();
            VkShaderModule FindShaderModule(const PipelineLayoutEntryPointPair& key);
            VkShaderModule AddOrGetCachedShaderModule(const PipelineLayoutEntryPointPair& key,
                                                      VkShaderModule value);

          private:
            Device* mDevice;
            std::mutex mMutex;
            std::unordered_map<PipelineLayoutEntryPointPair,
                               VkShaderModule,
                               PipelineLayoutEntryPointPairHashFunc>
                mTransformedShaderModuleCache;
        };
        std::unique_ptr<ConcurrentTransformedShaderModuleCache> mTransformedShaderModuleCache;
    };

}}  // namespace dawn_native::vulkan

#endif  // DAWNNATIVE_VULKAN_SHADERMODULEVK_H_
