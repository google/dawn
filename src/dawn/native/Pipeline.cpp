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

#include "dawn/native/Pipeline.h"

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/ShaderModule.h"

namespace dawn::native {
MaybeError ValidateProgrammableStage(DeviceBase* device,
                                     const ShaderModuleBase* module,
                                     const std::string& entryPoint,
                                     uint32_t constantCount,
                                     const ConstantEntry* constants,
                                     const PipelineLayoutBase* layout,
                                     SingleShaderStage stage) {
    DAWN_TRY(device->ValidateObject(module));

    DAWN_INVALID_IF(!module->HasEntryPoint(entryPoint),
                    "Entry point \"%s\" doesn't exist in the shader module %s.", entryPoint,
                    module);

    const EntryPointMetadata& metadata = module->GetEntryPoint(entryPoint);

    if (!metadata.infringedLimitErrors.empty()) {
        std::ostringstream out;
        out << "Entry point \"" << entryPoint << "\" infringes limits:\n";
        for (const std::string& limit : metadata.infringedLimitErrors) {
            out << " - " << limit << "\n";
        }
        return DAWN_VALIDATION_ERROR(out.str());
    }

    DAWN_INVALID_IF(metadata.stage != stage,
                    "The stage (%s) of the entry point \"%s\" isn't the expected one (%s).",
                    metadata.stage, entryPoint, stage);

    if (layout != nullptr) {
        DAWN_TRY(ValidateCompatibilityWithPipelineLayout(device, metadata, layout));
    }

    if (constantCount > 0u && device->IsToggleEnabled(Toggle::DisallowUnsafeAPIs)) {
        return DAWN_VALIDATION_ERROR(
            "Pipeline overridable constants are disallowed because they are partially "
            "implemented.");
    }

    // Validate if overridable constants exist in shader module
    // pipelineBase is not yet constructed at this moment so iterate constants from descriptor
    size_t numUninitializedConstants = metadata.uninitializedOverridableConstants.size();
    // Keep an initialized constants sets to handle duplicate initialization cases
    std::unordered_set<std::string> stageInitializedConstantIdentifiers;
    for (uint32_t i = 0; i < constantCount; i++) {
        DAWN_INVALID_IF(metadata.overridableConstants.count(constants[i].key) == 0,
                        "Pipeline overridable constant \"%s\" not found in %s.", constants[i].key,
                        module);

        if (stageInitializedConstantIdentifiers.count(constants[i].key) == 0) {
            if (metadata.uninitializedOverridableConstants.count(constants[i].key) > 0) {
                numUninitializedConstants--;
            }
            stageInitializedConstantIdentifiers.insert(constants[i].key);
        } else {
            // There are duplicate initializations
            return DAWN_FORMAT_VALIDATION_ERROR(
                "Pipeline overridable constants \"%s\" is set more than once in %s",
                constants[i].key, module);
        }
    }

    // Validate if any overridable constant is left uninitialized
    if (DAWN_UNLIKELY(numUninitializedConstants > 0)) {
        std::string uninitializedConstantsArray;
        bool isFirst = true;
        for (std::string identifier : metadata.uninitializedOverridableConstants) {
            if (stageInitializedConstantIdentifiers.count(identifier) > 0) {
                continue;
            }

            if (isFirst) {
                isFirst = false;
            } else {
                uninitializedConstantsArray.append(", ");
            }
            uninitializedConstantsArray.append(identifier);
        }

        return DAWN_FORMAT_VALIDATION_ERROR(
            "There are uninitialized pipeline overridable constants in shader module %s, their "
            "identifiers:[%s]",
            module, uninitializedConstantsArray);
    }

    return {};
}

// PipelineBase

PipelineBase::PipelineBase(DeviceBase* device,
                           PipelineLayoutBase* layout,
                           const char* label,
                           std::vector<StageAndDescriptor> stages)
    : ApiObjectBase(device, label), mLayout(layout) {
    ASSERT(!stages.empty());

    for (const StageAndDescriptor& stage : stages) {
        // Extract argument for this stage.
        SingleShaderStage shaderStage = stage.shaderStage;
        ShaderModuleBase* module = stage.module;
        const char* entryPointName = stage.entryPoint.c_str();

        const EntryPointMetadata& metadata = module->GetEntryPoint(entryPointName);
        ASSERT(metadata.stage == shaderStage);

        // Record them internally.
        bool isFirstStage = mStageMask == wgpu::ShaderStage::None;
        mStageMask |= StageBit(shaderStage);
        mStages[shaderStage] = {module, entryPointName, &metadata, {}};
        auto& constants = mStages[shaderStage].constants;
        for (uint32_t i = 0; i < stage.constantCount; i++) {
            constants.emplace(stage.constants[i].key, stage.constants[i].value);
        }

        // Compute the max() of all minBufferSizes across all stages.
        RequiredBufferSizes stageMinBufferSizes =
            ComputeRequiredBufferSizesForLayout(metadata, layout);

        if (isFirstStage) {
            mMinBufferSizes = std::move(stageMinBufferSizes);
        } else {
            for (BindGroupIndex group(0); group < mMinBufferSizes.size(); ++group) {
                ASSERT(stageMinBufferSizes[group].size() == mMinBufferSizes[group].size());

                for (size_t i = 0; i < stageMinBufferSizes[group].size(); ++i) {
                    mMinBufferSizes[group][i] =
                        std::max(mMinBufferSizes[group][i], stageMinBufferSizes[group][i]);
                }
            }
        }
    }
}

PipelineBase::PipelineBase(DeviceBase* device) : ApiObjectBase(device, kLabelNotImplemented) {}

PipelineBase::PipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag) {}

PipelineBase::~PipelineBase() = default;

PipelineLayoutBase* PipelineBase::GetLayout() {
    ASSERT(!IsError());
    return mLayout.Get();
}

const PipelineLayoutBase* PipelineBase::GetLayout() const {
    ASSERT(!IsError());
    return mLayout.Get();
}

const RequiredBufferSizes& PipelineBase::GetMinBufferSizes() const {
    ASSERT(!IsError());
    return mMinBufferSizes;
}

const ProgrammableStage& PipelineBase::GetStage(SingleShaderStage stage) const {
    ASSERT(!IsError());
    return mStages[stage];
}

const PerStage<ProgrammableStage>& PipelineBase::GetAllStages() const {
    return mStages;
}

wgpu::ShaderStage PipelineBase::GetStageMask() const {
    return mStageMask;
}

MaybeError PipelineBase::ValidateGetBindGroupLayout(uint32_t groupIndex) {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));
    DAWN_TRY(GetDevice()->ValidateObject(mLayout.Get()));
    DAWN_INVALID_IF(groupIndex >= kMaxBindGroups,
                    "Bind group layout index (%u) exceeds the maximum number of bind groups (%u).",
                    groupIndex, kMaxBindGroups);
    return {};
}

ResultOrError<Ref<BindGroupLayoutBase>> PipelineBase::GetBindGroupLayout(uint32_t groupIndexIn) {
    DAWN_TRY(ValidateGetBindGroupLayout(groupIndexIn));

    BindGroupIndex groupIndex(groupIndexIn);
    if (!mLayout->GetBindGroupLayoutsMask()[groupIndex]) {
        return Ref<BindGroupLayoutBase>(GetDevice()->GetEmptyBindGroupLayout());
    } else {
        return Ref<BindGroupLayoutBase>(mLayout->GetBindGroupLayout(groupIndex));
    }
}

BindGroupLayoutBase* PipelineBase::APIGetBindGroupLayout(uint32_t groupIndexIn) {
    Ref<BindGroupLayoutBase> result;
    if (GetDevice()->ConsumedError(GetBindGroupLayout(groupIndexIn), &result,
                                   "Validating GetBindGroupLayout (%u) on %s", groupIndexIn,
                                   this)) {
        return BindGroupLayoutBase::MakeError(GetDevice());
    }
    return result.Detach();
}

size_t PipelineBase::ComputeContentHash() {
    ObjectContentHasher recorder;
    recorder.Record(mLayout->GetContentHash());

    recorder.Record(mStageMask);
    for (SingleShaderStage stage : IterateStages(mStageMask)) {
        recorder.Record(mStages[stage].module->GetContentHash());
        recorder.Record(mStages[stage].entryPoint);
    }

    return recorder.GetContentHash();
}

// static
bool PipelineBase::EqualForCache(const PipelineBase* a, const PipelineBase* b) {
    // The layout is deduplicated so it can be compared by pointer.
    if (a->mLayout.Get() != b->mLayout.Get() || a->mStageMask != b->mStageMask) {
        return false;
    }

    for (SingleShaderStage stage : IterateStages(a->mStageMask)) {
        // The module is deduplicated so it can be compared by pointer.
        if (a->mStages[stage].module.Get() != b->mStages[stage].module.Get() ||
            a->mStages[stage].entryPoint != b->mStages[stage].entryPoint) {
            return false;
        }
    }

    return true;
}

}  // namespace dawn::native
