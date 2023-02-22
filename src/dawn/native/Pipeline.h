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

#ifndef SRC_DAWN_NATIVE_PIPELINE_H_
#define SRC_DAWN_NATIVE_PIPELINE_H_

#include <array>
#include <bitset>
#include <map>
#include <string>
#include <vector>

#include "dawn/native/CachedObject.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/ShaderModule.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

MaybeError ValidateProgrammableStage(DeviceBase* device,
                                     const ShaderModuleBase* module,
                                     const std::string& entryPoint,
                                     uint32_t constantCount,
                                     const ConstantEntry* constants,
                                     const PipelineLayoutBase* layout,
                                     SingleShaderStage stage);

WGPUCreatePipelineAsyncStatus CreatePipelineAsyncStatusFromErrorType(InternalErrorType error);

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
    bool HasStage(SingleShaderStage stage) const;
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

  private:
    MaybeError ValidateGetBindGroupLayout(BindGroupIndex group);

    wgpu::ShaderStage mStageMask = wgpu::ShaderStage::None;
    PerStage<ProgrammableStage> mStages;

    Ref<PipelineLayoutBase> mLayout;
    RequiredBufferSizes mMinBufferSizes;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_PIPELINE_H_
