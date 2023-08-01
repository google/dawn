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

#include "dawn/native/BindGroup.h"

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Device.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/Texture.h"
#include "dawn/native/utils/WGPUHelpers.h"

namespace dawn::native {

namespace {

// Helper functions to perform binding-type specific validation

MaybeError ValidateBufferBinding(const DeviceBase* device,
                                 const BindGroupEntry& entry,
                                 const BindingInfo& bindingInfo) {
    DAWN_INVALID_IF(entry.buffer == nullptr, "Binding entry buffer not set.");

    DAWN_INVALID_IF(entry.sampler != nullptr || entry.textureView != nullptr,
                    "Expected only buffer to be set for binding entry.");

    DAWN_INVALID_IF(entry.nextInChain != nullptr, "nextInChain must be nullptr.");

    DAWN_TRY(device->ValidateObject(entry.buffer));

    ASSERT(bindingInfo.bindingType == BindingInfoType::Buffer);

    uint64_t bufferSize = entry.buffer->GetSize();

    // Handle wgpu::WholeSize, avoiding overflows.
    DAWN_INVALID_IF(entry.offset > bufferSize,
                    "Binding offset (%u) is larger than the size (%u) of %s.", entry.offset,
                    bufferSize, entry.buffer);

    uint64_t bindingSize =
        (entry.size == wgpu::kWholeSize) ? bufferSize - entry.offset : entry.size;

    DAWN_INVALID_IF(bindingSize > bufferSize,
                    "Binding size (%u) is larger than the size (%u) of %s.", bindingSize,
                    bufferSize, entry.buffer);

    DAWN_INVALID_IF(bindingSize == 0, "Binding size is zero");

    // Note that no overflow can happen because we already checked that
    // bufferSize >= bindingSize
    DAWN_INVALID_IF(entry.offset > bufferSize - bindingSize,
                    "Binding range (offset: %u, size: %u) doesn't fit in the size (%u) of %s.",
                    entry.offset, bufferSize, bindingSize, entry.buffer);

    wgpu::BufferUsage requiredUsage;
    uint64_t maxBindingSize;
    uint64_t requiredBindingAlignment;
    switch (bindingInfo.buffer.type) {
        case wgpu::BufferBindingType::Uniform:
            requiredUsage = wgpu::BufferUsage::Uniform;
            maxBindingSize = device->GetLimits().v1.maxUniformBufferBindingSize;
            requiredBindingAlignment = device->GetLimits().v1.minUniformBufferOffsetAlignment;
            break;
        case wgpu::BufferBindingType::Storage:
        case wgpu::BufferBindingType::ReadOnlyStorage:
            requiredUsage = wgpu::BufferUsage::Storage;
            maxBindingSize = device->GetLimits().v1.maxStorageBufferBindingSize;
            requiredBindingAlignment = device->GetLimits().v1.minStorageBufferOffsetAlignment;
            DAWN_INVALID_IF(bindingSize % 4 != 0,
                            "Binding size (%u) isn't a multiple of 4 when binding type is (%s).",
                            bindingSize, bindingInfo.buffer.type);
            break;
        case kInternalStorageBufferBinding:
            requiredUsage = kInternalStorageBuffer;
            maxBindingSize = device->GetLimits().v1.maxStorageBufferBindingSize;
            requiredBindingAlignment = device->GetLimits().v1.minStorageBufferOffsetAlignment;
            break;
        case wgpu::BufferBindingType::Undefined:
            UNREACHABLE();
    }

    DAWN_INVALID_IF(!IsAligned(entry.offset, requiredBindingAlignment),
                    "Offset (%u) does not satisfy the minimum %s alignment (%u).", entry.offset,
                    bindingInfo.buffer.type, requiredBindingAlignment);

    DAWN_INVALID_IF(!(entry.buffer->GetUsage() & requiredUsage),
                    "Binding usage (%s) of %s doesn't match expected usage (%s).",
                    entry.buffer->GetUsageExternalOnly(), entry.buffer, requiredUsage);

    DAWN_INVALID_IF(bindingSize < bindingInfo.buffer.minBindingSize,
                    "Binding size (%u) is smaller than the minimum binding size (%u).", bindingSize,
                    bindingInfo.buffer.minBindingSize);

    DAWN_INVALID_IF(bindingSize > maxBindingSize,
                    "Binding size (%u) is larger than the maximum binding size (%u).", bindingSize,
                    maxBindingSize);

    return {};
}

MaybeError ValidateTextureBinding(DeviceBase* device,
                                  const BindGroupEntry& entry,
                                  const BindingInfo& bindingInfo,
                                  UsageValidationMode mode) {
    DAWN_INVALID_IF(entry.textureView == nullptr, "Binding entry textureView not set.");

    DAWN_INVALID_IF(entry.sampler != nullptr || entry.buffer != nullptr,
                    "Expected only textureView to be set for binding entry.");

    DAWN_INVALID_IF(entry.nextInChain != nullptr, "nextInChain must be nullptr.");

    DAWN_TRY(device->ValidateObject(entry.textureView));

    TextureViewBase* view = entry.textureView;

    Aspect aspect = view->GetAspects();
    DAWN_INVALID_IF(!HasOneBit(aspect), "Multiple aspects (%s) selected in %s.", aspect, view);

    TextureBase* texture = view->GetTexture();
    switch (bindingInfo.bindingType) {
        case BindingInfoType::Texture: {
            SampleTypeBit supportedTypes =
                texture->GetFormat().GetAspectInfo(aspect).supportedSampleTypes;
            DAWN_TRY(ValidateCanUseAs(texture, wgpu::TextureUsage::TextureBinding, mode));

            DAWN_INVALID_IF(texture->IsMultisampledTexture() != bindingInfo.texture.multisampled,
                            "Sample count (%u) of %s doesn't match expectation (multisampled: %d).",
                            texture->GetSampleCount(), texture, bindingInfo.texture.multisampled);

            SampleTypeBit requiredType;
            if (bindingInfo.texture.sampleType == kInternalResolveAttachmentSampleType) {
                // If the binding's sample type is kInternalResolveAttachmentSampleType,
                // then the supported types must contain float.
                requiredType = SampleTypeBit::UnfilterableFloat;
            } else {
                requiredType = SampleTypeToSampleTypeBit(bindingInfo.texture.sampleType);
            }

            DAWN_INVALID_IF(
                !(supportedTypes & requiredType),
                "None of the supported sample types (%s) of %s match the expected sample "
                "types (%s).",
                supportedTypes, texture, requiredType);

            DAWN_INVALID_IF(entry.textureView->GetDimension() != bindingInfo.texture.viewDimension,
                            "Dimension (%s) of %s doesn't match the expected dimension (%s).",
                            entry.textureView->GetDimension(), entry.textureView,
                            bindingInfo.texture.viewDimension);
            break;
        }
        case BindingInfoType::StorageTexture: {
            DAWN_TRY(ValidateCanUseAs(texture, wgpu::TextureUsage::StorageBinding, mode));

            ASSERT(!texture->IsMultisampledTexture());

            DAWN_INVALID_IF(texture->GetFormat().format != bindingInfo.storageTexture.format,
                            "Format (%s) of %s expected to be (%s).", texture->GetFormat().format,
                            texture, bindingInfo.storageTexture.format);

            DAWN_INVALID_IF(
                entry.textureView->GetDimension() != bindingInfo.storageTexture.viewDimension,
                "Dimension (%s) of %s doesn't match the expected dimension (%s).",
                entry.textureView->GetDimension(), entry.textureView,
                bindingInfo.storageTexture.viewDimension);

            DAWN_INVALID_IF(entry.textureView->GetLevelCount() != 1,
                            "mipLevelCount (%u) of %s expected to be 1.",
                            entry.textureView->GetLevelCount(), entry.textureView);
            break;
        }
        default:
            UNREACHABLE();
            break;
    }

    return {};
}

MaybeError ValidateSamplerBinding(const DeviceBase* device,
                                  const BindGroupEntry& entry,
                                  const BindingInfo& bindingInfo) {
    DAWN_INVALID_IF(entry.sampler == nullptr, "Binding entry sampler not set.");

    DAWN_INVALID_IF(entry.textureView != nullptr || entry.buffer != nullptr,
                    "Expected only sampler to be set for binding entry.");

    DAWN_INVALID_IF(entry.nextInChain != nullptr, "nextInChain must be nullptr.");

    DAWN_TRY(device->ValidateObject(entry.sampler));

    ASSERT(bindingInfo.bindingType == BindingInfoType::Sampler);

    switch (bindingInfo.sampler.type) {
        case wgpu::SamplerBindingType::NonFiltering:
            DAWN_INVALID_IF(entry.sampler->IsFiltering(),
                            "Filtering sampler %s is incompatible with non-filtering sampler "
                            "binding.",
                            entry.sampler);
            [[fallthrough]];
        case wgpu::SamplerBindingType::Filtering:
            DAWN_INVALID_IF(entry.sampler->IsComparison(),
                            "Comparison sampler %s is incompatible with non-comparison sampler "
                            "binding.",
                            entry.sampler);
            break;
        case wgpu::SamplerBindingType::Comparison:
            DAWN_INVALID_IF(!entry.sampler->IsComparison(),
                            "Non-comparison sampler %s is incompatible with comparison sampler "
                            "binding.",
                            entry.sampler);
            break;
        default:
            UNREACHABLE();
            break;
    }

    return {};
}

MaybeError ValidateExternalTextureBinding(
    const DeviceBase* device,
    const BindGroupEntry& entry,
    const ExternalTextureBindingEntry* externalTextureBindingEntry,
    const ExternalTextureBindingExpansionMap& expansions) {
    DAWN_INVALID_IF(externalTextureBindingEntry == nullptr,
                    "Binding entry external texture not set.");

    DAWN_INVALID_IF(
        entry.sampler != nullptr || entry.textureView != nullptr || entry.buffer != nullptr,
        "Expected only external texture to be set for binding entry.");

    DAWN_INVALID_IF(expansions.find(BindingNumber(entry.binding)) == expansions.end(),
                    "External texture binding entry %u is not present in the bind group layout.",
                    entry.binding);

    DAWN_TRY(ValidateSingleSType(externalTextureBindingEntry->nextInChain,
                                 wgpu::SType::ExternalTextureBindingEntry));

    DAWN_TRY(device->ValidateObject(externalTextureBindingEntry->externalTexture));

    return {};
}

template <typename F>
void ForEachUnverifiedBufferBindingIndexImpl(const BindGroupLayoutBase* bgl, F&& f) {
    uint32_t packedIndex = 0;
    for (BindingIndex bindingIndex{0}; bindingIndex < bgl->GetBufferCount(); ++bindingIndex) {
        if (bgl->GetBindingInfo(bindingIndex).buffer.minBindingSize == 0) {
            f(bindingIndex, packedIndex++);
        }
    }
}

}  // anonymous namespace

MaybeError ValidateBindGroupDescriptor(DeviceBase* device,
                                       const BindGroupDescriptor* descriptor,
                                       UsageValidationMode mode) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr.");

    DAWN_TRY(device->ValidateObject(descriptor->layout));

    DAWN_INVALID_IF(
        descriptor->entryCount != descriptor->layout->GetUnexpandedBindingCount(),
        "Number of entries (%u) did not match the number of entries (%u) specified in %s."
        "\nExpected layout: %s",
        descriptor->entryCount, static_cast<uint32_t>(descriptor->layout->GetBindingCount()),
        descriptor->layout, descriptor->layout->EntriesToString());

    const BindGroupLayoutInternalBase::BindingMap& bindingMap = descriptor->layout->GetBindingMap();
    ASSERT(bindingMap.size() <= kMaxBindingsPerPipelineLayout);

    ityp::bitset<BindingIndex, kMaxBindingsPerPipelineLayout> bindingsSet;
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        const BindGroupEntry& entry = descriptor->entries[i];

        const auto& it = bindingMap.find(BindingNumber(entry.binding));
        DAWN_INVALID_IF(it == bindingMap.end(),
                        "In entries[%u], binding index %u not present in the bind group layout."
                        "\nExpected layout: %s",
                        i, entry.binding, descriptor->layout->EntriesToString());

        BindingIndex bindingIndex = it->second;
        ASSERT(bindingIndex < descriptor->layout->GetBindingCount());

        DAWN_INVALID_IF(bindingsSet[bindingIndex],
                        "In entries[%u], binding index %u already used by a previous entry", i,
                        entry.binding);

        bindingsSet.set(bindingIndex);

        // Below this block we validate entries based on the bind group layout, in which
        // external textures have been expanded into their underlying contents. For this reason
        // we must identify external texture binding entries by checking the bind group entry
        // itself.
        // TODO(dawn:1293): Store external textures in
        // BindGroupLayoutBase::BindingDataPointers::bindings so checking external textures can
        // be moved in the switch below.
        const ExternalTextureBindingEntry* externalTextureBindingEntry = nullptr;
        FindInChain(entry.nextInChain, &externalTextureBindingEntry);
        if (externalTextureBindingEntry != nullptr) {
            DAWN_TRY(ValidateExternalTextureBinding(
                device, entry, externalTextureBindingEntry,
                descriptor->layout->GetExternalTextureBindingExpansionMap()));
            continue;
        } else {
            DAWN_INVALID_IF(descriptor->layout->GetExternalTextureBindingExpansionMap().count(
                                BindingNumber(entry.binding)),
                            "entries[%u] is not an ExternalTexture when the layout contains an "
                            "ExternalTexture entry.",
                            i);
        }

        const BindingInfo& bindingInfo = descriptor->layout->GetBindingInfo(bindingIndex);

        // Perform binding-type specific validation.
        switch (bindingInfo.bindingType) {
            case BindingInfoType::Buffer:
                // TODO(dawn:1485): Validate buffer binding with usage validation mode.
                DAWN_TRY_CONTEXT(ValidateBufferBinding(device, entry, bindingInfo),
                                 "validating entries[%u] as a Buffer."
                                 "\nExpected entry layout: %s",
                                 i, bindingInfo);
                break;
            case BindingInfoType::Texture:
            case BindingInfoType::StorageTexture:
                DAWN_TRY_CONTEXT(ValidateTextureBinding(device, entry, bindingInfo, mode),
                                 "validating entries[%u] as a Texture."
                                 "\nExpected entry layout: %s",
                                 i, bindingInfo);
                break;
            case BindingInfoType::Sampler:
                DAWN_TRY_CONTEXT(ValidateSamplerBinding(device, entry, bindingInfo),
                                 "validating entries[%u] as a Sampler."
                                 "\nExpected entry layout: %s",
                                 i, bindingInfo);
                break;
            case BindingInfoType::ExternalTexture:
                UNREACHABLE();
                break;
        }
    }

    // This should always be true because
    //  - numBindings has to match between the bind group and its layout.
    //  - Each binding must be set at most once
    //
    // We don't validate the equality because it wouldn't be possible to cover it with a test.
    ASSERT(bindingsSet.count() == descriptor->layout->GetUnexpandedBindingCount());

    return {};
}

// BindGroup

BindGroupBase::BindGroupBase(DeviceBase* device,
                             const BindGroupDescriptor* descriptor,
                             void* bindingDataStart)
    : ApiObjectBase(device, descriptor->label),
      mLayout(descriptor->layout),
      mBindingData(mLayout->ComputeBindingDataPointers(bindingDataStart)) {
    for (BindingIndex i{0}; i < mLayout->GetBindingCount(); ++i) {
        // TODO(enga): Shouldn't be needed when bindings are tightly packed.
        // This is to fill Ref<ObjectBase> holes with nullptrs.
        new (&mBindingData.bindings[i]) Ref<ObjectBase>();
    }

    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        const BindGroupEntry& entry = descriptor->entries[i];

        BindingIndex bindingIndex =
            descriptor->layout->GetBindingIndex(BindingNumber(entry.binding));
        ASSERT(bindingIndex < mLayout->GetBindingCount());

        // Only a single binding type should be set, so once we found it we can skip to the
        // next loop iteration.

        if (entry.buffer != nullptr) {
            ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
            mBindingData.bindings[bindingIndex] = entry.buffer;
            mBindingData.bufferData[bindingIndex].offset = entry.offset;
            uint64_t bufferSize = (entry.size == wgpu::kWholeSize)
                                      ? entry.buffer->GetSize() - entry.offset
                                      : entry.size;
            mBindingData.bufferData[bindingIndex].size = bufferSize;
            continue;
        }

        if (entry.textureView != nullptr) {
            ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
            mBindingData.bindings[bindingIndex] = entry.textureView;
            continue;
        }

        if (entry.sampler != nullptr) {
            ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
            mBindingData.bindings[bindingIndex] = entry.sampler;
            continue;
        }

        // Here we unpack external texture bindings into multiple additional bindings for the
        // external texture's contents. New binding locations previously determined in the bind
        // group layout are created in this bind group and filled with the external texture's
        // underlying resources.
        const ExternalTextureBindingEntry* externalTextureBindingEntry = nullptr;
        FindInChain(entry.nextInChain, &externalTextureBindingEntry);
        if (externalTextureBindingEntry != nullptr) {
            mBoundExternalTextures.push_back(externalTextureBindingEntry->externalTexture);

            ExternalTextureBindingExpansionMap expansions =
                mLayout->GetExternalTextureBindingExpansionMap();
            ExternalTextureBindingExpansionMap::iterator it =
                expansions.find(BindingNumber(entry.binding));

            ASSERT(it != expansions.end());

            BindingIndex plane0BindingIndex =
                descriptor->layout->GetBindingIndex(it->second.plane0);
            BindingIndex plane1BindingIndex =
                descriptor->layout->GetBindingIndex(it->second.plane1);
            BindingIndex paramsBindingIndex =
                descriptor->layout->GetBindingIndex(it->second.params);

            ASSERT(mBindingData.bindings[plane0BindingIndex] == nullptr);

            mBindingData.bindings[plane0BindingIndex] =
                externalTextureBindingEntry->externalTexture->GetTextureViews()[0];

            ASSERT(mBindingData.bindings[plane1BindingIndex] == nullptr);
            mBindingData.bindings[plane1BindingIndex] =
                externalTextureBindingEntry->externalTexture->GetTextureViews()[1];

            ASSERT(mBindingData.bindings[paramsBindingIndex] == nullptr);
            mBindingData.bindings[paramsBindingIndex] =
                externalTextureBindingEntry->externalTexture->GetParamsBuffer();
            mBindingData.bufferData[paramsBindingIndex].offset = 0;
            mBindingData.bufferData[paramsBindingIndex].size =
                sizeof(dawn_native::ExternalTextureParams);

            continue;
        }
    }

    ForEachUnverifiedBufferBindingIndexImpl(mLayout.Get(),
                                            [&](BindingIndex bindingIndex, uint32_t packedIndex) {
                                                mBindingData.unverifiedBufferSizes[packedIndex] =
                                                    mBindingData.bufferData[bindingIndex].size;
                                            });

    GetObjectTrackingList()->Track(this);
}

BindGroupBase::~BindGroupBase() = default;

void BindGroupBase::DestroyImpl() {
    if (mLayout != nullptr) {
        ASSERT(!IsError());
        for (BindingIndex i{0}; i < mLayout->GetBindingCount(); ++i) {
            mBindingData.bindings[i].~Ref<ObjectBase>();
        }
    }
}

void BindGroupBase::DeleteThis() {
    // Add another ref to the layout so that if this is the last ref, the layout
    // is destroyed after the bind group. The bind group is slab-allocated inside
    // memory owned by the layout (except for the null backend).
    Ref<BindGroupLayoutBase> layout = mLayout;
    ApiObjectBase::DeleteThis();
}

BindGroupBase::BindGroupBase(DeviceBase* device, ObjectBase::ErrorTag tag, const char* label)
    : ApiObjectBase(device, tag, label), mBindingData() {}

// static
BindGroupBase* BindGroupBase::MakeError(DeviceBase* device, const char* label) {
    return new BindGroupBase(device, ObjectBase::kError, label);
}

ObjectType BindGroupBase::GetType() const {
    return ObjectType::BindGroup;
}

BindGroupLayoutBase* BindGroupBase::GetLayout() {
    ASSERT(!IsError());
    return mLayout.Get();
}

const BindGroupLayoutBase* BindGroupBase::GetLayout() const {
    ASSERT(!IsError());
    return mLayout.Get();
}

const ityp::span<uint32_t, uint64_t>& BindGroupBase::GetUnverifiedBufferSizes() const {
    ASSERT(!IsError());
    return mBindingData.unverifiedBufferSizes;
}

BufferBinding BindGroupBase::GetBindingAsBufferBinding(BindingIndex bindingIndex) {
    ASSERT(!IsError());
    ASSERT(bindingIndex < mLayout->GetBindingCount());
    ASSERT(mLayout->GetBindingInfo(bindingIndex).bindingType == BindingInfoType::Buffer);
    BufferBase* buffer = static_cast<BufferBase*>(mBindingData.bindings[bindingIndex].Get());
    return {buffer, mBindingData.bufferData[bindingIndex].offset,
            mBindingData.bufferData[bindingIndex].size};
}

SamplerBase* BindGroupBase::GetBindingAsSampler(BindingIndex bindingIndex) const {
    ASSERT(!IsError());
    ASSERT(bindingIndex < mLayout->GetBindingCount());
    ASSERT(mLayout->GetBindingInfo(bindingIndex).bindingType == BindingInfoType::Sampler);
    return static_cast<SamplerBase*>(mBindingData.bindings[bindingIndex].Get());
}

TextureViewBase* BindGroupBase::GetBindingAsTextureView(BindingIndex bindingIndex) {
    ASSERT(!IsError());
    ASSERT(bindingIndex < mLayout->GetBindingCount());
    ASSERT(mLayout->GetBindingInfo(bindingIndex).bindingType == BindingInfoType::Texture ||
           mLayout->GetBindingInfo(bindingIndex).bindingType == BindingInfoType::StorageTexture);
    return static_cast<TextureViewBase*>(mBindingData.bindings[bindingIndex].Get());
}

const std::vector<Ref<ExternalTextureBase>>& BindGroupBase::GetBoundExternalTextures() const {
    return mBoundExternalTextures;
}

void BindGroupBase::ForEachUnverifiedBufferBindingIndex(
    std::function<void(BindingIndex, uint32_t)> fn) const {
    ForEachUnverifiedBufferBindingIndexImpl(mLayout.Get(), fn);
}

}  // namespace dawn::native
