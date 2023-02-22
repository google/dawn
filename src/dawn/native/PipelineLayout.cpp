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

#include "dawn/native/PipelineLayout.h"

#include <algorithm>
#include <map>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/common/ityp_stack_vec.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/ShaderModule.h"

namespace dawn::native {

MaybeError ValidatePipelineLayoutDescriptor(DeviceBase* device,
                                            const PipelineLayoutDescriptor* descriptor,
                                            PipelineCompatibilityToken pipelineCompatibilityToken) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain is not nullptr.");
    DAWN_INVALID_IF(descriptor->bindGroupLayoutCount > kMaxBindGroups,
                    "bindGroupLayoutCount (%i) is larger than the maximum allowed (%i).",
                    descriptor->bindGroupLayoutCount, kMaxBindGroups);

    BindingCounts bindingCounts = {};
    for (uint32_t i = 0; i < descriptor->bindGroupLayoutCount; ++i) {
        DAWN_TRY(device->ValidateObject(descriptor->bindGroupLayouts[i]));
        DAWN_INVALID_IF(descriptor->bindGroupLayouts[i]->GetPipelineCompatibilityToken() !=
                            pipelineCompatibilityToken,
                        "bindGroupLayouts[%i] (%s) is used to create a pipeline layout but it was "
                        "created as part of a pipeline's default layout.",
                        i, descriptor->bindGroupLayouts[i]);

        AccumulateBindingCounts(&bindingCounts,
                                descriptor->bindGroupLayouts[i]->GetBindingCountInfo());
    }

    DAWN_TRY(ValidateBindingCounts(bindingCounts));
    return {};
}

// PipelineLayoutBase

PipelineLayoutBase::PipelineLayoutBase(DeviceBase* device,
                                       const PipelineLayoutDescriptor* descriptor,
                                       ApiObjectBase::UntrackedByDeviceTag tag)
    : ApiObjectBase(device, descriptor->label) {
    ASSERT(descriptor->bindGroupLayoutCount <= kMaxBindGroups);
    for (BindGroupIndex group(0); group < BindGroupIndex(descriptor->bindGroupLayoutCount);
         ++group) {
        mBindGroupLayouts[group] = descriptor->bindGroupLayouts[static_cast<uint32_t>(group)];
        mMask.set(group);
    }
}

PipelineLayoutBase::PipelineLayoutBase(DeviceBase* device,
                                       const PipelineLayoutDescriptor* descriptor)
    : PipelineLayoutBase(device, descriptor, kUntrackedByDevice) {
    GetObjectTrackingList()->Track(this);
}

PipelineLayoutBase::PipelineLayoutBase(DeviceBase* device, ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag) {}

PipelineLayoutBase::~PipelineLayoutBase() = default;

void PipelineLayoutBase::DestroyImpl() {
    if (IsCachedReference()) {
        // Do not uncache the actual cached object if we are a blueprint.
        GetDevice()->UncachePipelineLayout(this);
    }
}

// static
PipelineLayoutBase* PipelineLayoutBase::MakeError(DeviceBase* device) {
    return new PipelineLayoutBase(device, ObjectBase::kError);
}

// static
ResultOrError<Ref<PipelineLayoutBase>> PipelineLayoutBase::CreateDefault(
    DeviceBase* device,
    std::vector<StageAndDescriptor> stages) {
    using EntryMap = std::map<BindingNumber, BindGroupLayoutEntry>;

    // Merges two entries at the same location, if they are allowed to be merged.
    auto MergeEntries = [](BindGroupLayoutEntry* modifiedEntry,
                           const BindGroupLayoutEntry& mergedEntry) -> MaybeError {
        // Visibility is excluded because we take the OR across stages.
        bool compatible =
            modifiedEntry->binding == mergedEntry.binding &&
            modifiedEntry->buffer.type == mergedEntry.buffer.type &&
            modifiedEntry->sampler.type == mergedEntry.sampler.type &&
            // Compatibility between these sample types is checked below.
            (modifiedEntry->texture.sampleType != wgpu::TextureSampleType::Undefined) ==
                (mergedEntry.texture.sampleType != wgpu::TextureSampleType::Undefined) &&
            modifiedEntry->storageTexture.access == mergedEntry.storageTexture.access;

        // Minimum buffer binding size excluded because we take the maximum seen across stages.
        if (modifiedEntry->buffer.type != wgpu::BufferBindingType::Undefined) {
            compatible = compatible && modifiedEntry->buffer.hasDynamicOffset ==
                                           mergedEntry.buffer.hasDynamicOffset;
        }

        if (modifiedEntry->texture.sampleType != wgpu::TextureSampleType::Undefined) {
            // Sample types are compatible if they are exactly equal,
            // or if the |modifiedEntry| is Float and the |mergedEntry| is UnfilterableFloat.
            // Note that the |mergedEntry| never has type Float. Texture bindings all start
            // as UnfilterableFloat and are promoted to Float if they are statically used with
            // a sampler.
            ASSERT(mergedEntry.texture.sampleType != wgpu::TextureSampleType::Float);
            bool compatibleSampleTypes =
                modifiedEntry->texture.sampleType == mergedEntry.texture.sampleType ||
                (modifiedEntry->texture.sampleType == wgpu::TextureSampleType::Float &&
                 mergedEntry.texture.sampleType == wgpu::TextureSampleType::UnfilterableFloat);
            compatible =
                compatible && compatibleSampleTypes &&
                modifiedEntry->texture.viewDimension == mergedEntry.texture.viewDimension &&
                modifiedEntry->texture.multisampled == mergedEntry.texture.multisampled;
        }

        if (modifiedEntry->storageTexture.access != wgpu::StorageTextureAccess::Undefined) {
            compatible =
                compatible &&
                modifiedEntry->storageTexture.format == mergedEntry.storageTexture.format &&
                modifiedEntry->storageTexture.viewDimension ==
                    mergedEntry.storageTexture.viewDimension;
        }

        // Check if any properties are incompatible with existing entry
        // If compatible, we will merge some properties
        // TODO(dawn:563): Improve the error message by doing early-outs when bindings aren't
        // compatible instead of a single check at the end.
        if (!compatible) {
            return DAWN_VALIDATION_ERROR(
                "Duplicate binding in default pipeline layout initialization "
                "not compatible with previous declaration");
        }

        // Use the max |minBufferBindingSize| we find.
        modifiedEntry->buffer.minBindingSize =
            std::max(modifiedEntry->buffer.minBindingSize, mergedEntry.buffer.minBindingSize);

        // Use the OR of all the stages at which we find this binding.
        modifiedEntry->visibility |= mergedEntry.visibility;

        return {};
    };

    // Does the trivial conversions from a ShaderBindingInfo to a BindGroupLayoutEntry
    auto ConvertMetadataToEntry =
        [](const ShaderBindingInfo& shaderBinding,
           const ExternalTextureBindingLayout* externalTextureBindingEntry)
        -> BindGroupLayoutEntry {
        BindGroupLayoutEntry entry = {};
        switch (shaderBinding.bindingType) {
            case BindingInfoType::Buffer:
                entry.buffer.type = shaderBinding.buffer.type;
                entry.buffer.hasDynamicOffset = shaderBinding.buffer.hasDynamicOffset;
                entry.buffer.minBindingSize = shaderBinding.buffer.minBindingSize;
                break;
            case BindingInfoType::Sampler:
                if (shaderBinding.sampler.isComparison) {
                    entry.sampler.type = wgpu::SamplerBindingType::Comparison;
                } else {
                    entry.sampler.type = wgpu::SamplerBindingType::Filtering;
                }
                break;
            case BindingInfoType::Texture:
                switch (shaderBinding.texture.compatibleSampleTypes) {
                    case SampleTypeBit::Depth:
                        entry.texture.sampleType = wgpu::TextureSampleType::Depth;
                        break;
                    case SampleTypeBit::Sint:
                        entry.texture.sampleType = wgpu::TextureSampleType::Sint;
                        break;
                    case SampleTypeBit::Uint:
                        entry.texture.sampleType = wgpu::TextureSampleType::Uint;
                        break;
                    case SampleTypeBit::Float:
                    case SampleTypeBit::UnfilterableFloat:
                    case SampleTypeBit::None:
                        UNREACHABLE();
                        break;
                    default:
                        if (shaderBinding.texture.compatibleSampleTypes ==
                            (SampleTypeBit::Float | SampleTypeBit::UnfilterableFloat)) {
                            // Default to UnfilterableFloat. It will be promoted to Float if it
                            // is used with a sampler.
                            entry.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
                        } else {
                            UNREACHABLE();
                        }
                }
                entry.texture.viewDimension = shaderBinding.texture.viewDimension;
                entry.texture.multisampled = shaderBinding.texture.multisampled;
                break;
            case BindingInfoType::StorageTexture:
                entry.storageTexture.access = shaderBinding.storageTexture.access;
                entry.storageTexture.format = shaderBinding.storageTexture.format;
                entry.storageTexture.viewDimension = shaderBinding.storageTexture.viewDimension;
                break;
            case BindingInfoType::ExternalTexture:
                entry.nextInChain = externalTextureBindingEntry;
                break;
        }
        return entry;
    };

    PipelineCompatibilityToken pipelineCompatibilityToken =
        device->GetNextPipelineCompatibilityToken();

    // Creates the BGL from the entries for a stage, checking it is valid.
    auto CreateBGL = [](DeviceBase* device, const EntryMap& entries,
                        PipelineCompatibilityToken pipelineCompatibilityToken)
        -> ResultOrError<Ref<BindGroupLayoutBase>> {
        std::vector<BindGroupLayoutEntry> entryVec;
        entryVec.reserve(entries.size());
        for (auto& [_, entry] : entries) {
            entryVec.push_back(entry);
        }

        BindGroupLayoutDescriptor desc = {};
        desc.entries = entryVec.data();
        desc.entryCount = entryVec.size();

        if (device->IsValidationEnabled()) {
            DAWN_TRY_CONTEXT(ValidateBindGroupLayoutDescriptor(device, &desc), "validating %s",
                             &desc);
        }
        return device->GetOrCreateBindGroupLayout(&desc, pipelineCompatibilityToken);
    };

    ASSERT(!stages.empty());

    // Data which BindGroupLayoutDescriptor will point to for creation
    ityp::array<BindGroupIndex, std::map<BindingNumber, BindGroupLayoutEntry>, kMaxBindGroups>
        entryData = {};

    // External texture binding layouts are chained structs that are set as a pointer within
    // the bind group layout entry. We declare an entry here so that it can be used when needed
    // in each BindGroupLayoutEntry and so it can stay alive until the call to
    // GetOrCreateBindGroupLayout. Because ExternalTextureBindingLayout is an empty struct,
    // there's no issue with using the same struct multiple times.
    ExternalTextureBindingLayout externalTextureBindingLayout;

    // Loops over all the reflected BindGroupLayoutEntries from shaders.
    for (const StageAndDescriptor& stage : stages) {
        const EntryPointMetadata& metadata = stage.module->GetEntryPoint(stage.entryPoint);

        for (BindGroupIndex group(0); group < metadata.bindings.size(); ++group) {
            for (const auto& [bindingNumber, shaderBinding] : metadata.bindings[group]) {
                // Create the BindGroupLayoutEntry
                BindGroupLayoutEntry entry =
                    ConvertMetadataToEntry(shaderBinding, &externalTextureBindingLayout);
                entry.binding = static_cast<uint32_t>(bindingNumber);
                entry.visibility = StageBit(stage.shaderStage);

                // Add it to our map of all entries, if there is an existing entry, then we
                // need to merge, if we can.
                const auto& [existingEntry, inserted] =
                    entryData[group].insert({bindingNumber, entry});
                if (!inserted) {
                    DAWN_TRY_CONTEXT(MergeEntries(&existingEntry->second, entry),
                                     "merging implicit bindings for @group(%u) @binding(%u).",
                                     uint32_t(group), uint32_t(bindingNumber));
                }
            }
        }

        // Promote any Unfilterable textures used with a sampler to Filtering.
        for (const EntryPointMetadata::SamplerTexturePair& pair : metadata.samplerTexturePairs) {
            BindGroupLayoutEntry* entry = &entryData[pair.texture.group][pair.texture.binding];
            if (entry->texture.sampleType == wgpu::TextureSampleType::UnfilterableFloat) {
                entry->texture.sampleType = wgpu::TextureSampleType::Float;
            }
        }
    }

    // Create the bind group layouts. We need to keep track of the last non-empty BGL because
    // Dawn doesn't yet know that an empty BGL and a null BGL are the same thing.
    // TODO(cwallez@chromium.org): remove this when Dawn knows that empty and null BGL are the
    // same.
    BindGroupIndex pipelineBGLCount = BindGroupIndex(0);
    ityp::array<BindGroupIndex, Ref<BindGroupLayoutBase>, kMaxBindGroups> bindGroupLayouts = {};
    for (BindGroupIndex group(0); group < kMaxBindGroupsTyped; ++group) {
        DAWN_TRY_ASSIGN(bindGroupLayouts[group],
                        CreateBGL(device, entryData[group], pipelineCompatibilityToken));
        if (entryData[group].size() != 0) {
            pipelineBGLCount = group + BindGroupIndex(1);
        }
    }

    // Create the deduced pipeline layout, validating if it is valid.
    ityp::array<BindGroupIndex, BindGroupLayoutBase*, kMaxBindGroups> bgls = {};
    for (BindGroupIndex group(0); group < pipelineBGLCount; ++group) {
        bgls[group] = bindGroupLayouts[group].Get();
    }

    PipelineLayoutDescriptor desc = {};
    desc.bindGroupLayouts = bgls.data();
    desc.bindGroupLayoutCount = static_cast<uint32_t>(pipelineBGLCount);

    DAWN_TRY(ValidatePipelineLayoutDescriptor(device, &desc, pipelineCompatibilityToken));

    Ref<PipelineLayoutBase> result;
    DAWN_TRY_ASSIGN(result, device->GetOrCreatePipelineLayout(&desc));
    ASSERT(!result->IsError());

    // Check in debug that the pipeline layout is compatible with the current pipeline.
    for (const StageAndDescriptor& stage : stages) {
        const EntryPointMetadata& metadata = stage.module->GetEntryPoint(stage.entryPoint);
        ASSERT(ValidateCompatibilityWithPipelineLayout(device, metadata, result.Get()).IsSuccess());
    }

    return std::move(result);
}

ObjectType PipelineLayoutBase::GetType() const {
    return ObjectType::PipelineLayout;
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

BindGroupLayoutMask PipelineLayoutBase::InheritedGroupsMask(const PipelineLayoutBase* other) const {
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

size_t PipelineLayoutBase::ComputeContentHash() {
    ObjectContentHasher recorder;
    recorder.Record(mMask);

    for (BindGroupIndex group : IterateBitSet(mMask)) {
        recorder.Record(GetBindGroupLayout(group)->GetContentHash());
    }

    return recorder.GetContentHash();
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

}  // namespace dawn::native
