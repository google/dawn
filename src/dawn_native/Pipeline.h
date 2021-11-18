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

#ifndef DAWNNATIVE_PIPELINE_H_
#define DAWNNATIVE_PIPELINE_H_

#include "dawn_native/CachedObject.h"
#include "dawn_native/Forward.h"
#include "dawn_native/ObjectBase.h"
#include "dawn_native/PerStage.h"
#include "dawn_native/PipelineLayout.h"
#include "dawn_native/ShaderModule.h"

#include "dawn_native/dawn_platform.h"

#include <array>
#include <bitset>

namespace dawn_native {

    MaybeError ValidateProgrammableStage(DeviceBase* device,
                                         const ShaderModuleBase* module,
                                         const std::string& entryPoint,
                                         uint32_t constantCount,
                                         const ConstantEntry* constants,
                                         const PipelineLayoutBase* layout,
                                         SingleShaderStage stage);

    // Use map to make sure constant keys are sorted for creating shader cache keys
    using PipelineConstantEntries = std::map<std::string, double>;

    struct ProgrammableStage {
        Ref<ShaderModuleBase> module;
        std::string entryPoint;

        // The metadata lives as long as module, that's ref-ed in the same structure.
        const EntryPointMetadata* metadata = nullptr;

        PipelineConstantEntries constants;
    };

    class PipelineBase : public ApiObjectBase, public CachedObject {
      public:
        ~PipelineBase() override;

        PipelineLayoutBase* GetLayout();
        const PipelineLayoutBase* GetLayout() const;
        const RequiredBufferSizes& GetMinBufferSizes() const;
        const ProgrammableStage& GetStage(SingleShaderStage stage) const;
        const PerStage<ProgrammableStage>& GetAllStages() const;
        wgpu::ShaderStage GetStageMask() const;

        ResultOrError<Ref<BindGroupLayoutBase>> GetBindGroupLayout(uint32_t groupIndex);

        // Helper functions for std::unordered_map-based pipeline caches.
        size_t ComputeContentHash() override;
        static bool EqualForCache(const PipelineBase* a, const PipelineBase* b);

        // Implementation of the API entrypoint. Do not use in a reentrant manner.
        BindGroupLayoutBase* APIGetBindGroupLayout(uint32_t groupIndex);

        // Initialize() should only be called once by the frontend.
        virtual MaybeError Initialize() = 0;

      protected:
        PipelineBase(DeviceBase* device,
                     PipelineLayoutBase* layout,
                     const char* label,
                     std::vector<StageAndDescriptor> stages);
        PipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag);

        // Constructor used only for mocking and testing.
        PipelineBase(DeviceBase* device);

      private:
        MaybeError ValidateGetBindGroupLayout(uint32_t group);

        wgpu::ShaderStage mStageMask = wgpu::ShaderStage::None;
        PerStage<ProgrammableStage> mStages;

        Ref<PipelineLayoutBase> mLayout;
        RequiredBufferSizes mMinBufferSizes;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_PIPELINE_H_
