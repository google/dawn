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

#include "dawn_native/PipelineLayout.h"

#include "common/Assert.h"
#include "common/BitSetIterator.h"
#include "common/HashUtils.h"
#include "common/ityp_stack_vec.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Device.h"
#include "dawn_native/ShaderModule.h"

namespace dawn_native {

    namespace {

        bool InferredBindGroupLayoutEntriesCompatible(const BindGroupLayoutEntry& lhs,
                                                      const BindGroupLayoutEntry& rhs) {
            // Minimum buffer binding size excluded because we take the maximum seen across stages.
            // Visibility is excluded because we take the OR across stages.
            return lhs.binding == rhs.binding && lhs.type == rhs.type &&
                   lhs.hasDynamicOffset == rhs.hasDynamicOffset &&
                   lhs.multisampled == rhs.multisampled && lhs.viewDimension == rhs.viewDimension &&
                   lhs.textureComponentType == rhs.textureComponentType;
        }

    }  // anonymous namespace

    MaybeError ValidatePipelineLayoutDescriptor(DeviceBase* device,
                                                const PipelineLayoutDescriptor* descriptor) {
        if (descriptor->nextInChain != nullptr) {
            return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
        }

        if (descriptor->bindGroupLayoutCount > kMaxBindGroups) {
            return DAWN_VALIDATION_ERROR("too many bind group layouts");
        }

        BindingCounts bindingCounts = {};
        for (uint32_t i = 0; i < descriptor->bindGroupLayoutCount; ++i) {
            DAWN_TRY(device->ValidateObject(descriptor->bindGroupLayouts[i]));
            AccumulateBindingCounts(&bindingCounts,
                                    descriptor->bindGroupLayouts[i]->GetBindingCountInfo());
        }

        DAWN_TRY(ValidateBindingCounts(bindingCounts));
        return {};
    }

    // PipelineLayoutBase

    PipelineLayoutBase::PipelineLayoutBase(DeviceBase* device,
                                           const PipelineLayoutDescriptor* descriptor)
        : CachedObject(device) {
        ASSERT(descriptor->bindGroupLayoutCount <= kMaxBindGroups);
        for (BindGroupIndex group(0); group < BindGroupIndex(descriptor->bindGroupLayoutCount);
             ++group) {
            mBindGroupLayouts[group] = descriptor->bindGroupLayouts[static_cast<uint32_t>(group)];
            mMask.set(group);
        }
    }

    PipelineLayoutBase::PipelineLayoutBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : CachedObject(device, tag) {
    }

    PipelineLayoutBase::~PipelineLayoutBase() {
        // Do not uncache the actual cached object if we are a blueprint
        if (IsCachedReference()) {
            GetDevice()->UncachePipelineLayout(this);
        }
    }

    // static
    PipelineLayoutBase* PipelineLayoutBase::MakeError(DeviceBase* device) {
        return new PipelineLayoutBase(device, ObjectBase::kError);
    }

    // static
    ResultOrError<PipelineLayoutBase*> PipelineLayoutBase::CreateDefault(
        DeviceBase* device,
        std::vector<StageAndDescriptor> stages) {
        ASSERT(!stages.empty());

        // Data which BindGroupLayoutDescriptor will point to for creation
        ityp::array<
            BindGroupIndex,
            ityp::stack_vec<BindingIndex, BindGroupLayoutEntry, kMaxOptimalBindingsPerGroup>,
            kMaxBindGroups>
            entryData = {};

        // A map of bindings to the index in |entryData|
        ityp::array<BindGroupIndex, std::map<BindingNumber, BindingIndex>, kMaxBindGroups>
            usedBindingsMap = {};

        // A counter of how many bindings we've populated in |entryData|
        ityp::array<BindGroupIndex, BindingIndex, kMaxBindGroups> entryCounts = {};

        BindingCounts bindingCounts = {};
        BindGroupIndex bindGroupLayoutCount(0);
        for (const StageAndDescriptor& stage : stages) {
            // Extract argument for this stage.
            SingleShaderStage shaderStage = stage.first;
            const EntryPointMetadata::BindingInfo& info =
                stage.second->module->GetEntryPoint(stage.second->entryPoint, shaderStage).bindings;

            for (BindGroupIndex group(0); group < info.size(); ++group) {
                for (const auto& it : info[group]) {
                    BindingNumber bindingNumber = it.first;
                    const EntryPointMetadata::ShaderBindingInfo& bindingInfo = it.second;

                    BindGroupLayoutEntry bindingSlot;
                    bindingSlot.binding = static_cast<uint32_t>(bindingNumber);
                    bindingSlot.visibility = StageBit(shaderStage);
                    bindingSlot.type = bindingInfo.type;
                    bindingSlot.hasDynamicOffset = false;
                    bindingSlot.multisampled = bindingInfo.multisampled;
                    bindingSlot.viewDimension = bindingInfo.viewDimension;
                    bindingSlot.textureComponentType =
                        Format::FormatTypeToTextureComponentType(bindingInfo.textureComponentType);
                    bindingSlot.storageTextureFormat = bindingInfo.storageTextureFormat;
                    bindingSlot.minBufferBindingSize = bindingInfo.minBufferBindingSize;

                    {
                        const auto& it = usedBindingsMap[group].find(bindingNumber);
                        if (it != usedBindingsMap[group].end()) {
                            BindGroupLayoutEntry* existingEntry = &entryData[group][it->second];

                            // Check if any properties are incompatible with existing entry
                            // If compatible, we will merge some properties
                            if (!InferredBindGroupLayoutEntriesCompatible(*existingEntry,
                                                                          bindingSlot)) {
                                return DAWN_VALIDATION_ERROR(
                                    "Duplicate binding in default pipeline layout initialization "
                                    "not compatible with previous declaration");
                            }

                            // Use the max |minBufferBindingSize| we find.
                            existingEntry->minBufferBindingSize =
                                std::max(existingEntry->minBufferBindingSize,
                                         bindingSlot.minBufferBindingSize);

                            // Use the OR of all the stages at which we find this binding.
                            existingEntry->visibility |= bindingSlot.visibility;

                            // Already used slot, continue
                            continue;
                        }
                    }

                    IncrementBindingCounts(&bindingCounts, bindingSlot);
                    BindingIndex currentBindingCount = entryCounts[group];
                    entryData[group].resize(currentBindingCount + BindingIndex(1));
                    entryData[group][currentBindingCount] = bindingSlot;

                    usedBindingsMap[group][bindingNumber] = currentBindingCount;

                    entryCounts[group]++;

                    bindGroupLayoutCount =
                        std::max(bindGroupLayoutCount, group + BindGroupIndex(1));
                }
            }
        }

        // Create the deduced BGLs, validating if they are valid.
        ityp::array<BindGroupIndex, Ref<BindGroupLayoutBase>, kMaxBindGroups> bindGroupLayouts = {};
        for (BindGroupIndex group(0); group < bindGroupLayoutCount; ++group) {
            BindGroupLayoutDescriptor desc = {};
            desc.entries = entryData[group].data();
            desc.entryCount = static_cast<uint32_t>(entryCounts[group]);

            DAWN_TRY(ValidateBindGroupLayoutDescriptor(device, &desc));
            DAWN_TRY_ASSIGN(bindGroupLayouts[group], device->GetOrCreateBindGroupLayout(&desc));

            ASSERT(!bindGroupLayouts[group]->IsError());
        }

        // Create the deduced pipeline layout, validating if it is valid.
        PipelineLayoutBase* pipelineLayout = nullptr;
        {
            ityp::array<BindGroupIndex, BindGroupLayoutBase*, kMaxBindGroups> bgls = {};
            for (BindGroupIndex group(0); group < bindGroupLayoutCount; ++group) {
                bgls[group] = bindGroupLayouts[group].Get();
            }

            PipelineLayoutDescriptor desc = {};
            desc.bindGroupLayouts = bgls.data();
            desc.bindGroupLayoutCount = static_cast<uint32_t>(bindGroupLayoutCount);

            DAWN_TRY(ValidatePipelineLayoutDescriptor(device, &desc));
            DAWN_TRY_ASSIGN(pipelineLayout, device->GetOrCreatePipelineLayout(&desc));

            ASSERT(!pipelineLayout->IsError());
        }

        // Sanity check in debug that the pipeline layout is compatible with the current pipeline.
        for (const StageAndDescriptor& stage : stages) {
            const EntryPointMetadata& metadata =
                stage.second->module->GetEntryPoint(stage.second->entryPoint, stage.first);
            ASSERT(ValidateCompatibilityWithPipelineLayout(metadata, pipelineLayout).IsSuccess());
        }

        return pipelineLayout;
    }

    const BindGroupLayoutBase* PipelineLayoutBase::GetBindGroupLayout(BindGroupIndex group) const {
        ASSERT(!IsError());
        ASSERT(group < kMaxBindGroupsTyped);
        ASSERT(mMask[group]);
        const BindGroupLayoutBase* bgl = mBindGroupLayouts[group].Get();
        ASSERT(bgl != nullptr);
        return bgl;
    }

    BindGroupLayoutBase* PipelineLayoutBase::GetBindGroupLayout(BindGroupIndex group) {
        ASSERT(!IsError());
        ASSERT(group < kMaxBindGroupsTyped);
        ASSERT(mMask[group]);
        BindGroupLayoutBase* bgl = mBindGroupLayouts[group].Get();
        ASSERT(bgl != nullptr);
        return bgl;
    }

    const BindGroupLayoutMask& PipelineLayoutBase::GetBindGroupLayoutsMask() const {
        ASSERT(!IsError());
        return mMask;
    }

    BindGroupLayoutMask PipelineLayoutBase::InheritedGroupsMask(
        const PipelineLayoutBase* other) const {
        ASSERT(!IsError());
        return {(1 << static_cast<uint32_t>(GroupsInheritUpTo(other))) - 1u};
    }

    BindGroupIndex PipelineLayoutBase::GroupsInheritUpTo(const PipelineLayoutBase* other) const {
        ASSERT(!IsError());

        for (BindGroupIndex i(0); i < kMaxBindGroupsTyped; ++i) {
            if (!mMask[i] || mBindGroupLayouts[i].Get() != other->mBindGroupLayouts[i].Get()) {
                return i;
            }
        }
        return kMaxBindGroupsTyped;
    }

    size_t PipelineLayoutBase::HashFunc::operator()(const PipelineLayoutBase* pl) const {
        size_t hash = Hash(pl->mMask);

        for (BindGroupIndex group : IterateBitSet(pl->mMask)) {
            HashCombine(&hash, pl->GetBindGroupLayout(group));
        }

        return hash;
    }

    bool PipelineLayoutBase::EqualityFunc::operator()(const PipelineLayoutBase* a,
                                                      const PipelineLayoutBase* b) const {
        if (a->mMask != b->mMask) {
            return false;
        }

        for (BindGroupIndex group : IterateBitSet(a->mMask)) {
            if (a->GetBindGroupLayout(group) != b->GetBindGroupLayout(group)) {
                return false;
            }
        }

        return true;
    }

}  // namespace dawn_native
