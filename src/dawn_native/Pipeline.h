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
#include "dawn_native/PerStage.h"
#include "dawn_native/PipelineLayout.h"
#include "dawn_native/ShaderModule.h"

#include "dawn_native/dawn_platform.h"

#include <array>
#include <bitset>

namespace dawn_native {

    MaybeError ValidateProgrammableStageDescriptor(const DeviceBase* device,
                                                   const ProgrammableStageDescriptor* descriptor,
                                                   const PipelineLayoutBase* layout,
                                                   SingleShaderStage stage);

    struct ProgrammableStage {
        Ref<ShaderModuleBase> module;
        std::string entryPoint;

        // The metadata lives as long as module, that's ref-ed in the same structure.
        const EntryPointMetadata* metadata = nullptr;
    };

    class PipelineBase : public CachedObject {
      public:
        PipelineLayoutBase* GetLayout();
        const PipelineLayoutBase* GetLayout() const;
        const RequiredBufferSizes& GetMinBufferSizes() const;
        const ProgrammableStage& GetStage(SingleShaderStage stage) const;
        const PerStage<ProgrammableStage>& GetAllStages() const;

        BindGroupLayoutBase* GetBindGroupLayout(uint32_t groupIndex);

        // Helper function for the functors for std::unordered_map-based pipeline caches.
        static size_t HashForCache(const PipelineBase* pipeline);
        static bool EqualForCache(const PipelineBase* a, const PipelineBase* b);

      protected:
        PipelineBase(DeviceBase* device,
                     PipelineLayoutBase* layout,
                     std::vector<StageAndDescriptor> stages);
        PipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag);

      private:
        MaybeError ValidateGetBindGroupLayout(uint32_t group);

        wgpu::ShaderStage mStageMask = wgpu::ShaderStage::None;
        PerStage<ProgrammableStage> mStages;

        Ref<PipelineLayoutBase> mLayout;
        RequiredBufferSizes mMinBufferSizes;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_PIPELINE_H_
