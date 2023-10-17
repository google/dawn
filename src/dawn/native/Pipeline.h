// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
    PipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag, const char* label);

  private:
    MaybeError ValidateGetBindGroupLayout(BindGroupIndex group);

    wgpu::ShaderStage mStageMask = wgpu::ShaderStage::None;
    PerStage<ProgrammableStage> mStages;

    Ref<PipelineLayoutBase> mLayout;
    RequiredBufferSizes mMinBufferSizes;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_PIPELINE_H_
