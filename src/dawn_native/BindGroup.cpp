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
#include "dawn_native/Device.h"
#include "dawn_native/Sampler.h"
#include "dawn_native/Texture.h"

namespace dawn_native {

    namespace {

        // Helper functions to perform binding-type specific validation

        MaybeError ValidateBufferBinding(const DeviceBase* device,
                                         const BindGroupEntry& entry,
                                         const BindingInfo& bindingInfo) {
            if (entry.buffer == nullptr || entry.sampler != nullptr ||
                entry.textureView != nullptr) {
                return DAWN_VALIDATION_ERROR("Expected buffer binding");
            }
            DAWN_TRY(device->ValidateObject(entry.buffer));

            ASSERT(bindingInfo.bindingType == BindingInfoType::Buffer);

            wgpu::BufferUsage requiredUsage;
            uint64_t maxBindingSize;
            switch (bindingInfo.buffer.type) {
                case wgpu::BufferBindingType::Uniform:
                    requiredUsage = wgpu::BufferUsage::Uniform;
                    maxBindingSize = kMaxUniformBufferBindingSize;
                    break;
                case wgpu::BufferBindingType::Storage:
                case wgpu::BufferBindingType::ReadOnlyStorage:
                    requiredUsage = wgpu::BufferUsage::Storage;
                    maxBindingSize = std::numeric_limits<uint64_t>::max();
                    break;
                case wgpu::BufferBindingType::Undefined:
                    UNREACHABLE();
            }

            uint64_t bufferSize = entry.buffer->GetSize();

            // Handle wgpu::WholeSize, avoiding overflows.
            if (entry.offset > bufferSize) {
                return DAWN_VALIDATION_ERROR("Buffer binding doesn't fit in the buffer");
            }
            uint64_t bindingSize =
                (entry.size == wgpu::kWholeSize) ? bufferSize - entry.offset : entry.size;

            if (bindingSize > bufferSize) {
                return DAWN_VALIDATION_ERROR("Buffer binding size larger than the buffer");
            }

            if (bindingSize == 0) {
                return DAWN_VALIDATION_ERROR("Buffer binding size cannot be zero.");
            }

            // Note that no overflow can happen because we already checked that
            // bufferSize >= bindingSize
            if (entry.offset > bufferSize - bindingSize) {
                return DAWN_VALIDATION_ERROR("Buffer binding doesn't fit in the buffer");
            }

            if (!IsAligned(entry.offset, 256)) {
                return DAWN_VALIDATION_ERROR(
                    "Buffer offset for bind group needs to be 256-byte aligned");
            }

            if (!(entry.buffer->GetUsage() & requiredUsage)) {
                return DAWN_VALIDATION_ERROR("buffer binding usage mismatch");
            }

            if (bindingSize < bindingInfo.buffer.minBindingSize) {
                return DAWN_VALIDATION_ERROR(
                    "Binding size smaller than minimum buffer size: binding " +
                    std::to_string(entry.binding) + " given " + std::to_string(bindingSize) +
                    " bytes, required " + std::to_string(bindingInfo.buffer.minBindingSize) +
                    " bytes");
            }

            if (bindingSize > maxBindingSize) {
                return DAWN_VALIDATION_ERROR(
                    "Binding size bigger than maximum uniform buffer binding size: binding " +
                    std::to_string(entry.binding) + " given " + std::to_string(bindingSize) +
                    " bytes, maximum is " + std::to_string(kMaxUniformBufferBindingSize) +
                    " bytes");
            }

            return {};
        }

        MaybeError ValidateTextureBinding(const DeviceBase* device,
                                          const BindGroupEntry& entry,
                                          const BindingInfo& bindingInfo) {
            if (entry.textureView == nullptr || entry.sampler != nullptr ||
                entry.buffer != nullptr) {
                return DAWN_VALIDATION_ERROR("Expected texture binding");
            }
            DAWN_TRY(device->ValidateObject(entry.textureView));

            TextureViewBase* view = entry.textureView;

            Aspect aspect = view->GetAspects();
            if (!HasOneBit(aspect)) {
                return DAWN_VALIDATION_ERROR("Texture view must select a single aspect");
            }

            TextureBase* texture = view->GetTexture();
            switch (bindingInfo.bindingType) {
                case BindingInfoType::Texture: {
                    ComponentTypeBit supportedTypes =
                        texture->GetFormat().GetAspectInfo(aspect).supportedComponentTypes;
                    ComponentTypeBit requiredType =
                        SampleTypeToComponentTypeBit(bindingInfo.texture.sampleType);

                    if (!(texture->GetUsage() & wgpu::TextureUsage::Sampled)) {
                        return DAWN_VALIDATION_ERROR("Texture binding usage mismatch");
                    }

                    if (texture->IsMultisampledTexture() != bindingInfo.texture.multisampled) {
                        return DAWN_VALIDATION_ERROR("Texture multisampling mismatch");
                    }

                    if ((supportedTypes & requiredType) == 0) {
                        return DAWN_VALIDATION_ERROR("Texture component type usage mismatch");
                    }

                    if (entry.textureView->GetDimension() != bindingInfo.texture.viewDimension) {
                        return DAWN_VALIDATION_ERROR("Texture view dimension mismatch");
                    }
                    break;
                }
                case BindingInfoType::StorageTexture: {
                    ASSERT(!texture->IsMultisampledTexture());

                    if (!(texture->GetUsage() & wgpu::TextureUsage::Storage)) {
                        return DAWN_VALIDATION_ERROR("Storage Texture binding usage mismatch");
                    }

                    if (texture->GetFormat().format != bindingInfo.storageTexture.format) {
                        return DAWN_VALIDATION_ERROR("Storage texture format mismatch");
                    }
                    if (entry.textureView->GetDimension() !=
                        bindingInfo.storageTexture.viewDimension) {
                        return DAWN_VALIDATION_ERROR("Storage texture view dimension mismatch");
                    }
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
            if (entry.sampler == nullptr || entry.textureView != nullptr ||
                entry.buffer != nullptr) {
                return DAWN_VALIDATION_ERROR("Expected sampler binding");
            }
            DAWN_TRY(device->ValidateObject(entry.sampler));

            ASSERT(bindingInfo.bindingType == BindingInfoType::Sampler);

            switch (bindingInfo.sampler.type) {
                case wgpu::SamplerBindingType::Filtering:
                case wgpu::SamplerBindingType::NonFiltering:
                    if (entry.sampler->HasCompareFunction()) {
                        return DAWN_VALIDATION_ERROR("Did not expect comparison sampler");
                    }
                    break;
                case wgpu::SamplerBindingType::Comparison:
                    if (!entry.sampler->HasCompareFunction()) {
                        return DAWN_VALIDATION_ERROR("Expected comparison sampler");
                    }
                    break;
                default:
                    UNREACHABLE();
                    break;
            }

            return {};
        }

    }  // anonymous namespace

    MaybeError ValidateBindGroupDescriptor(DeviceBase* device,
                                           const BindGroupDescriptor* descriptor) {
        if (descriptor->nextInChain != nullptr) {
            return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
        }

        DAWN_TRY(device->ValidateObject(descriptor->layout));

        if (BindingIndex(descriptor->entryCount) != descriptor->layout->GetBindingCount()) {
            return DAWN_VALIDATION_ERROR("numBindings mismatch");
        }

        const BindGroupLayoutBase::BindingMap& bindingMap = descriptor->layout->GetBindingMap();
        ASSERT(bindingMap.size() <= kMaxBindingsPerPipelineLayout);

        ityp::bitset<BindingIndex, kMaxBindingsPerPipelineLayout> bindingsSet;
        for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
            const BindGroupEntry& entry = descriptor->entries[i];

            const auto& it = bindingMap.find(BindingNumber(entry.binding));
            if (it == bindingMap.end()) {
                return DAWN_VALIDATION_ERROR("setting non-existent binding");
            }
            BindingIndex bindingIndex = it->second;
            ASSERT(bindingIndex < descriptor->layout->GetBindingCount());

            if (bindingsSet[bindingIndex]) {
                return DAWN_VALIDATION_ERROR("binding set twice");
            }
            bindingsSet.set(bindingIndex);

            const BindingInfo& bindingInfo = descriptor->layout->GetBindingInfo(bindingIndex);

            // Perform binding-type specific validation.
            switch (bindingInfo.bindingType) {
                case BindingInfoType::Buffer:
                    DAWN_TRY(ValidateBufferBinding(device, entry, bindingInfo));
                    break;
                case BindingInfoType::Texture:
                case BindingInfoType::StorageTexture:
                    DAWN_TRY(ValidateTextureBinding(device, entry, bindingInfo));
                    break;
                case BindingInfoType::Sampler:
                    DAWN_TRY(ValidateSamplerBinding(device, entry, bindingInfo));
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
        : ObjectBase(device),
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
    }

    BindGroupBase::~BindGroupBase() {
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
        RefCounted::DeleteThis();
    }

    BindGroupBase::BindGroupBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : ObjectBase(device, tag), mBindingData() {
    }

    // static
    BindGroupBase* BindGroupBase::MakeError(DeviceBase* device) {
        return new BindGroupBase(device, ObjectBase::kError);
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

}  // namespace dawn_native
