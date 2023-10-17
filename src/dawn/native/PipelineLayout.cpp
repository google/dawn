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

#include "dawn/native/PipelineLayout.h"

#include <algorithm>
#include <map>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Numeric.h"
#include "dawn/common/ityp_stack_vec.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/ShaderModule.h"

namespace dawn::native {

MaybeError ValidatePipelineLayoutDescriptor(DeviceBase* device,
                                            const PipelineLayoutDescriptor* descriptor,
                                            PipelineCompatibilityToken pipelineCompatibilityToken) {
    // Validation for any pixel local storage.
    const PipelineLayoutPixelLocalStorage* pls = nullptr;
    FindInChain(descriptor->nextInChain, &pls);
    if (pls != nullptr) {
        StackVector<StorageAttachmentInfoForValidation, 4> attachments;
        for (size_t i = 0; i < pls->storageAttachmentCount; i++) {
            const PipelineLayoutStorageAttachment& attachment = pls->storageAttachments[i];

            const Format* format;
            DAWN_TRY_ASSIGN_CONTEXT(format, device->GetInternalFormat(attachment.format),
                                    "validating storageAttachments[%i]", i);
            DAWN_INVALID_IF(!format->supportsStorageAttachment,
                            "storageAttachments[%i]'s format (%s) cannot be used with %s.", i,
                            format->format, wgpu::TextureUsage::StorageAttachment);

            attachments->push_back({attachment.offset, attachment.format});
        }

        DAWN_TRY(ValidatePLSInfo(device, pls->totalPixelLocalStorageSize,
                                 {attachments->data(), attachments->size()}));
    }

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

        AccumulateBindingCounts(
            &bindingCounts,
            descriptor->bindGroupLayouts[i]->GetInternalBindGroupLayout()->GetBindingCountInfo());
    }

    DAWN_TRY(ValidateBindingCounts(device->GetLimits(), bindingCounts));
    return {};
}

// PipelineLayoutBase

PipelineLayoutBase::PipelineLayoutBase(DeviceBase* device,
                                       const PipelineLayoutDescriptor* descriptor,
                                       ApiObjectBase::UntrackedByDeviceTag tag)
    : ApiObjectBase(device, descriptor->label) {
    DAWN_ASSERT(descriptor->bindGroupLayoutCount <= kMaxBindGroups);
    for (BindGroupIndex group(0);
         group < BindGroupIndex(checked_cast<uint32_t>(descriptor->bindGroupLayoutCount));
         ++group) {
        mBindGroupLayouts[group] = descriptor->bindGroupLayouts[static_cast<uint32_t>(group)];
        mMask.set(group);
    }

    // Gather the PLS information.
    const PipelineLayoutPixelLocalStorage* pls = nullptr;
    FindInChain(descriptor->nextInChain, &pls);
    if (pls != nullptr) {
        mHasPLS = true;
        mStorageAttachmentSlots = std::vector<wgpu::TextureFormat>(
            pls->totalPixelLocalStorageSize / kPLSSlotByteSize, wgpu::TextureFormat::Undefined);
        for (size_t i = 0; i < pls->storageAttachmentCount; i++) {
            size_t slot = pls->storageAttachments[i].offset / kPLSSlotByteSize;
            mStorageAttachmentSlots[slot] = pls->storageAttachments[i].format;
        }
    }
}

PipelineLayoutBase::PipelineLayoutBase(DeviceBase* device,
                                       const PipelineLayoutDescriptor* descriptor)
    : PipelineLayoutBase(device, descriptor, kUntrackedByDevice) {
    GetObjectTrackingList()->Track(this);
}

PipelineLayoutBase::PipelineLayoutBase(DeviceBase* device,
                                       ObjectBase::ErrorTag tag,
                                       const char* label)
    : ApiObjectBase(device, tag, label) {}

PipelineLayoutBase::~PipelineLayoutBase() = default;

void PipelineLayoutBase::DestroyImpl() {
    Uncache();
}

// static
PipelineLayoutBase* PipelineLayoutBase::MakeError(DeviceBase* device, const char* label) {
    return new PipelineLayoutBase(device, ObjectBase::kError, label);
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
            DAWN_ASSERT(mergedEntry.texture.sampleType != wgpu::TextureSampleType::Float);
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
                        DAWN_UNREACHABLE();
                        break;
                    default:
                        if (shaderBinding.texture.compatibleSampleTypes ==
                            (SampleTypeBit::Float | SampleTypeBit::UnfilterableFloat)) {
                            // Default to UnfilterableFloat. It will be promoted to Float if it
                            // is used with a sampler.
                            entry.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
                        } else {
                            DAWN_UNREACHABLE();
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

    DAWN_ASSERT(!stages.empty());

    PipelineCompatibilityToken pipelineCompatibilityToken =
        device->GetNextPipelineCompatibilityToken();

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

        // TODO(dawn:1704): Find if we can usefully deduce the PLS for the pipeline layout.
        DAWN_INVALID_IF(
            metadata.usesPixelLocal,
            "Implicit layouts are not supported for entry-points using `pixel_local` blocks.");

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
    DAWN_ASSERT(!result->IsError());

    // Check in debug that the pipeline layout is compatible with the current pipeline.
    for (const StageAndDescriptor& stage : stages) {
        const EntryPointMetadata& metadata = stage.module->GetEntryPoint(stage.entryPoint);
        DAWN_ASSERT(
            ValidateCompatibilityWithPipelineLayout(device, metadata, result.Get()).IsSuccess());
    }

    return std::move(result);
}

ObjectType PipelineLayoutBase::GetType() const {
    return ObjectType::PipelineLayout;
}

const BindGroupLayoutBase* PipelineLayoutBase::GetFrontendBindGroupLayout(
    BindGroupIndex group) const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(group < kMaxBindGroupsTyped);
    DAWN_ASSERT(mMask[group]);
    const BindGroupLayoutBase* bgl = mBindGroupLayouts[group].Get();
    DAWN_ASSERT(bgl != nullptr);
    return bgl;
}

BindGroupLayoutBase* PipelineLayoutBase::GetFrontendBindGroupLayout(BindGroupIndex group) {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(group < kMaxBindGroupsTyped);
    DAWN_ASSERT(mMask[group]);
    BindGroupLayoutBase* bgl = mBindGroupLayouts[group].Get();
    DAWN_ASSERT(bgl != nullptr);
    return bgl;
}

const BindGroupLayoutInternalBase* PipelineLayoutBase::GetBindGroupLayout(
    BindGroupIndex group) const {
    return GetFrontendBindGroupLayout(group)->GetInternalBindGroupLayout();
}

BindGroupLayoutInternalBase* PipelineLayoutBase::GetBindGroupLayout(BindGroupIndex group) {
    return GetFrontendBindGroupLayout(group)->GetInternalBindGroupLayout();
}

const BindGroupLayoutMask& PipelineLayoutBase::GetBindGroupLayoutsMask() const {
    DAWN_ASSERT(!IsError());
    return mMask;
}

bool PipelineLayoutBase::HasPixelLocalStorage() const {
    return mHasPLS;
}

const std::vector<wgpu::TextureFormat>& PipelineLayoutBase::GetStorageAttachmentSlots() const {
    return mStorageAttachmentSlots;
}

bool PipelineLayoutBase::HasAnyStorageAttachments() const {
    for (auto format : mStorageAttachmentSlots) {
        if (format != wgpu::TextureFormat::Undefined) {
            return true;
        }
    }
    return false;
}

BindGroupLayoutMask PipelineLayoutBase::InheritedGroupsMask(const PipelineLayoutBase* other) const {
    DAWN_ASSERT(!IsError());
    return {(1 << static_cast<uint32_t>(GroupsInheritUpTo(other))) - 1u};
}

BindGroupIndex PipelineLayoutBase::GroupsInheritUpTo(const PipelineLayoutBase* other) const {
    DAWN_ASSERT(!IsError());

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

    // Hash the PLS state
    recorder.Record(mHasPLS);
    for (wgpu::TextureFormat slotFormat : mStorageAttachmentSlots) {
        recorder.Record(slotFormat);
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

    // Check PLS
    if (a->mHasPLS != b->mHasPLS) {
        return false;
    }
    if (a->mStorageAttachmentSlots.size() != b->mStorageAttachmentSlots.size()) {
        return false;
    }
    for (size_t i = 0; i < a->mStorageAttachmentSlots.size(); i++) {
        if (a->mStorageAttachmentSlots[i] != b->mStorageAttachmentSlots[i]) {
            return false;
        }
    }

    return true;
}

}  // namespace dawn::native
