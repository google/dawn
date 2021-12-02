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

#include "dawn_native/BindGroup.h"

#include "common/Assert.h"
#include "common/Math.h"
#include "common/ityp_bitset.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/ChainUtils_autogen.h"
#include "dawn_native/Device.h"
#include "dawn_native/ExternalTexture.h"
#include "dawn_native/ObjectBase.h"
#include "dawn_native/ObjectType_autogen.h"
#include "dawn_native/Sampler.h"
#include "dawn_native/Texture.h"

namespace dawn_native {

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

            wgpu::BufferUsage requiredUsage;
            uint64_t maxBindingSize;
            uint64_t requiredBindingAlignment;
            switch (bindingInfo.buffer.type) {
                case wgpu::BufferBindingType::Uniform:
                    requiredUsage = wgpu::BufferUsage::Uniform;
                    maxBindingSize = device->GetLimits().v1.maxUniformBufferBindingSize;
                    requiredBindingAlignment =
                        device->GetLimits().v1.minUniformBufferOffsetAlignment;
                    break;
                case wgpu::BufferBindingType::Storage:
                case wgpu::BufferBindingType::ReadOnlyStorage:
                    requiredUsage = wgpu::BufferUsage::Storage;
                    maxBindingSize = device->GetLimits().v1.maxStorageBufferBindingSize;
                    requiredBindingAlignment =
                        device->GetLimits().v1.minStorageBufferOffsetAlignment;
                    break;
                case kInternalStorageBufferBinding:
                    requiredUsage = kInternalStorageBuffer;
                    maxBindingSize = device->GetLimits().v1.maxStorageBufferBindingSize;
                    requiredBindingAlignment =
                        device->GetLimits().v1.minStorageBufferOffsetAlignment;
                    break;
                case wgpu::BufferBindingType::Undefined:
                    UNREACHABLE();
            }

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
            DAWN_INVALID_IF(
                entry.offset > bufferSize - bindingSize,
                "Binding range (offset: %u, size: %u) doesn't fit in the size (%u) of %s.",
                entry.offset, bufferSize, bindingSize, entry.buffer);

            DAWN_INVALID_IF(!IsAligned(entry.offset, requiredBindingAlignment),
                            "Offset (%u) does not satisfy the minimum %s alignment (%u).",
                            entry.offset, bindingInfo.buffer.type, requiredBindingAlignment);

            DAWN_INVALID_IF(!(entry.buffer->GetUsage() & requiredUsage),
                            "Binding usage (%s) of %s doesn't match expected usage (%s).",
                            entry.buffer->GetUsage(), entry.buffer, requiredUsage);

            DAWN_INVALID_IF(bindingSize < bindingInfo.buffer.minBindingSize,
                            "Binding size (%u) is smaller than the minimum binding size (%u).",
                            bindingSize, bindingInfo.buffer.minBindingSize);

            DAWN_INVALID_IF(bindingSize > maxBindingSize,
                            "Binding size (%u) is larger than the maximum binding size (%u).",
                            bindingSize, maxBindingSize);

            return {};
        }

        MaybeError ValidateTextureBinding(DeviceBase* device,
                                          const BindGroupEntry& entry,
                                          const BindingInfo& bindingInfo) {
            DAWN_INVALID_IF(entry.textureView == nullptr, "Binding entry textureView not set.");

            DAWN_INVALID_IF(entry.sampler != nullptr || entry.buffer != nullptr,
                            "Expected only textureView to be set for binding entry.");

            DAWN_INVALID_IF(entry.nextInChain != nullptr, "nextInChain must be nullptr.");

            DAWN_TRY(device->ValidateObject(entry.textureView));

            TextureViewBase* view = entry.textureView;

            Aspect aspect = view->GetAspects();
            // TODO(dawn:563): Format Aspects
            DAWN_INVALID_IF(!HasOneBit(aspect), "Multiple aspects selected in %s.", view);

            TextureBase* texture = view->GetTexture();
            switch (bindingInfo.bindingType) {
                case BindingInfoType::Texture: {
                    SampleTypeBit supportedTypes =
                        texture->GetFormat().GetAspectInfo(aspect).supportedSampleTypes;
                    SampleTypeBit requiredType =
                        SampleTypeToSampleTypeBit(bindingInfo.texture.sampleType);

                    DAWN_INVALID_IF(
                        !(texture->GetUsage() & wgpu::TextureUsage::TextureBinding),
                        "Usage (%s) of %s doesn't include TextureUsage::TextureBinding.",
                        texture->GetUsage(), texture);

                    DAWN_INVALID_IF(
                        texture->IsMultisampledTexture() != bindingInfo.texture.multisampled,
                        "Sample count (%u) of %s doesn't match expectation (multisampled: %d).",
                        texture->GetSampleCount(), texture, bindingInfo.texture.multisampled);

                    // TODO(dawn:563): Improve error message.
                    DAWN_INVALID_IF((supportedTypes & requiredType) == 0,
                                    "Texture component type usage mismatch.");

                    DAWN_INVALID_IF(
                        entry.textureView->GetDimension() != bindingInfo.texture.viewDimension,
                        "Dimension (%s) of %s doesn't match the expected dimension (%s).",
                        entry.textureView->GetDimension(), entry.textureView,
                        bindingInfo.texture.viewDimension);
                    break;
                }
                case BindingInfoType::StorageTexture: {
                    DAWN_INVALID_IF(
                        !(texture->GetUsage() & wgpu::TextureUsage::StorageBinding),
                        "Usage (%s) of %s doesn't include TextureUsage::StorageBinding.",
                        texture->GetUsage(), texture);

                    ASSERT(!texture->IsMultisampledTexture());

                    DAWN_INVALID_IF(
                        texture->GetFormat().format != bindingInfo.storageTexture.format,
                        "Format (%s) of %s expected to be (%s).", texture->GetFormat().format,
                        texture, bindingInfo.storageTexture.format);

                    DAWN_INVALID_IF(
                        entry.textureView->GetDimension() !=
                            bindingInfo.storageTexture.viewDimension,
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
                    DAWN_INVALID_IF(
                        entry.sampler->IsFiltering(),
                        "Filtering sampler %s is incompatible with non-filtering sampler "
                        "binding.",
                        entry.sampler);
                    DAWN_FALLTHROUGH;
                case wgpu::SamplerBindingType::Filtering:
                    DAWN_INVALID_IF(
                        entry.sampler->IsComparison(),
                        "Comparison sampler %s is incompatible with non-comparison sampler "
                        "binding.",
                        entry.sampler);
                    break;
                case wgpu::SamplerBindingType::Comparison:
                    DAWN_INVALID_IF(
                        !entry.sampler->IsComparison(),
                        "Non-comparison sampler %s is imcompatible with comparison sampler "
                        "binding.",
                        entry.sampler);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }

            return {};
        }

        MaybeError ValidateExternalTextureBinding(const DeviceBase* device,
                                                  const BindGroupEntry& entry,
                                                  const BindingInfo& bindingInfo) {
            const ExternalTextureBindingEntry* externalTextureBindingEntry = nullptr;
            FindInChain(entry.nextInChain, &externalTextureBindingEntry);

            DAWN_INVALID_IF(externalTextureBindingEntry == nullptr,
                            "Binding entry external texture not set.");

            DAWN_INVALID_IF(
                entry.sampler != nullptr || entry.textureView != nullptr || entry.buffer != nullptr,
                "Expected only external texture to be set for binding entry.");

            DAWN_TRY(ValidateSingleSType(externalTextureBindingEntry->nextInChain,
                                         wgpu::SType::ExternalTextureBindingEntry));

            DAWN_TRY(device->ValidateObject(externalTextureBindingEntry->externalTexture));

            return {};
        }

    }  // anonymous namespace

    MaybeError ValidateBindGroupDescriptor(DeviceBase* device,
                                           const BindGroupDescriptor* descriptor) {
        DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr.");

        DAWN_TRY(device->ValidateObject(descriptor->layout));

        DAWN_INVALID_IF(
            BindingIndex(descriptor->entryCount) != descriptor->layout->GetBindingCount(),
            "Number of entries (%u) did not match the number of entries (%u) specified in %s",
            descriptor->entryCount, static_cast<uint32_t>(descriptor->layout->GetBindingCount()),
            descriptor->layout);

        const BindGroupLayoutBase::BindingMap& bindingMap = descriptor->layout->GetBindingMap();
        ASSERT(bindingMap.size() <= kMaxBindingsPerPipelineLayout);

        ityp::bitset<BindingIndex, kMaxBindingsPerPipelineLayout> bindingsSet;
        for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
            const BindGroupEntry& entry = descriptor->entries[i];

            const auto& it = bindingMap.find(BindingNumber(entry.binding));
            DAWN_INVALID_IF(it == bindingMap.end(),
                            "In entries[%u], binding index %u not present in the bind group layout",
                            i, entry.binding);

            BindingIndex bindingIndex = it->second;
            ASSERT(bindingIndex < descriptor->layout->GetBindingCount());

            DAWN_INVALID_IF(bindingsSet[bindingIndex],
                            "In entries[%u], binding index %u already used by a previous entry", i,
                            entry.binding);

            bindingsSet.set(bindingIndex);

            const BindingInfo& bindingInfo = descriptor->layout->GetBindingInfo(bindingIndex);

            // Perform binding-type specific validation.
            switch (bindingInfo.bindingType) {
                case BindingInfoType::Buffer:
                    DAWN_TRY_CONTEXT(ValidateBufferBinding(device, entry, bindingInfo),
                                     "validating entries[%u] as a Buffer", i);
                    break;
                case BindingInfoType::Texture:
                case BindingInfoType::StorageTexture:
                    DAWN_TRY_CONTEXT(ValidateTextureBinding(device, entry, bindingInfo),
                                     "validating entries[%u] as a Texture", i);
                    break;
                case BindingInfoType::Sampler:
                    DAWN_TRY_CONTEXT(ValidateSamplerBinding(device, entry, bindingInfo),
                                     "validating entries[%u] as a Sampler", i);
                    break;
                case BindingInfoType::ExternalTexture:
                    DAWN_TRY_CONTEXT(ValidateExternalTextureBinding(device, entry, bindingInfo),
                                     "validating entries[%u] as an ExternalTexture", i);
                    break;
            }
        }

        // This should always be true because
        //  - numBindings has to match between the bind group and its layout.
        //  - Each binding must be set at most once
        //
        // We don't validate the equality because it wouldn't be possible to cover it with a test.
        ASSERT(bindingsSet.count() == bindingMap.size());

        return {};
    }  // anonymous namespace

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

            const ExternalTextureBindingEntry* externalTextureBindingEntry = nullptr;
            FindInChain(entry.nextInChain, &externalTextureBindingEntry);
            if (externalTextureBindingEntry != nullptr) {
                ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
                mBindingData.bindings[bindingIndex] = externalTextureBindingEntry->externalTexture;
                continue;
            }
        }

        uint32_t packedIdx = 0;
        for (BindingIndex bindingIndex{0}; bindingIndex < descriptor->layout->GetBufferCount();
             ++bindingIndex) {
            if (descriptor->layout->GetBindingInfo(bindingIndex).buffer.minBindingSize == 0) {
                mBindingData.unverifiedBufferSizes[packedIdx] =
                    mBindingData.bufferData[bindingIndex].size;
                ++packedIdx;
            }
        }

        TrackInDevice();
    }

    BindGroupBase::BindGroupBase(DeviceBase* device) : ApiObjectBase(device, kLabelNotImplemented) {
        TrackInDevice();
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

    BindGroupBase::BindGroupBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : ApiObjectBase(device, tag), mBindingData() {
    }

    // static
    BindGroupBase* BindGroupBase::MakeError(DeviceBase* device) {
        return new BindGroupBase(device, ObjectBase::kError);
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
               mLayout->GetBindingInfo(bindingIndex).bindingType ==
                   BindingInfoType::StorageTexture);
        return static_cast<TextureViewBase*>(mBindingData.bindings[bindingIndex].Get());
    }

    ExternalTextureBase* BindGroupBase::GetBindingAsExternalTexture(BindingIndex bindingIndex) {
        ASSERT(!IsError());
        ASSERT(bindingIndex < mLayout->GetBindingCount());
        ASSERT(mLayout->GetBindingInfo(bindingIndex).bindingType ==
               BindingInfoType::ExternalTexture);
        return static_cast<ExternalTextureBase*>(mBindingData.bindings[bindingIndex].Get());
    }

}  // namespace dawn_native
