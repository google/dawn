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
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/Device.h"
#include "dawn_native/Sampler.h"
#include "dawn_native/Texture.h"

namespace dawn_native {

    namespace {

        // Helper functions to perform binding-type specific validation

        MaybeError ValidateBufferBinding(const DeviceBase* device,
                                         const BindGroupBinding& binding,
                                         wgpu::BufferUsage requiredUsage) {
            if (binding.buffer == nullptr || binding.sampler != nullptr ||
                binding.textureView != nullptr) {
                return DAWN_VALIDATION_ERROR("expected buffer binding");
            }
            DAWN_TRY(device->ValidateObject(binding.buffer));

            uint64_t bufferSize = binding.buffer->GetSize();
            uint64_t bindingSize = (binding.size == wgpu::kWholeSize) ? bufferSize : binding.size;
            if (bindingSize > bufferSize) {
                return DAWN_VALIDATION_ERROR("Buffer binding size larger than the buffer");
            }

            // Note that no overflow can happen because we already checked that
            // bufferSize >= bindingSize
            if (binding.offset > bufferSize - bindingSize) {
                return DAWN_VALIDATION_ERROR("Buffer binding doesn't fit in the buffer");
            }

            if (!IsAligned(binding.offset, 256)) {
                return DAWN_VALIDATION_ERROR(
                    "Buffer offset for bind group needs to be 256-byte aligned");
            }

            if (!(binding.buffer->GetUsage() & requiredUsage)) {
                return DAWN_VALIDATION_ERROR("buffer binding usage mismatch");
            }

            return {};
        }

        MaybeError ValidateTextureBinding(const DeviceBase* device,
                                          const BindGroupBinding& binding,
                                          wgpu::TextureUsage requiredUsage,
                                          const BindingInfo& bindingInfo) {
            if (binding.textureView == nullptr || binding.sampler != nullptr ||
                binding.buffer != nullptr) {
                return DAWN_VALIDATION_ERROR("expected texture binding");
            }
            DAWN_TRY(device->ValidateObject(binding.textureView));

            TextureBase* texture = binding.textureView->GetTexture();

            if (!(texture->GetUsage() & requiredUsage)) {
                return DAWN_VALIDATION_ERROR("texture binding usage mismatch");
            }

            if (texture->IsMultisampledTexture() != bindingInfo.multisampled) {
                return DAWN_VALIDATION_ERROR("texture multisampling mismatch");
            }

            switch (requiredUsage) {
                case wgpu::TextureUsage::Sampled: {
                    if (!texture->GetFormat().HasComponentType(bindingInfo.textureComponentType)) {
                        return DAWN_VALIDATION_ERROR("texture component type usage mismatch");
                    }
                    break;
                }
                case wgpu::TextureUsage::Storage: {
                    if (texture->GetFormat().format != bindingInfo.storageTextureFormat) {
                        return DAWN_VALIDATION_ERROR("storage texture format mismatch");
                    }
                    break;
                }
                default:
                    UNREACHABLE();
                    break;
            }

            if (binding.textureView->GetDimension() != bindingInfo.textureDimension) {
                return DAWN_VALIDATION_ERROR("texture view dimension mismatch");
            }

            return {};
        }

        MaybeError ValidateSamplerBinding(const DeviceBase* device,
                                          const BindGroupBinding& binding) {
            if (binding.sampler == nullptr || binding.textureView != nullptr ||
                binding.buffer != nullptr) {
                return DAWN_VALIDATION_ERROR("expected sampler binding");
            }
            DAWN_TRY(device->ValidateObject(binding.sampler));

            return {};
        }

    }  // anonymous namespace

    MaybeError ValidateBindGroupDescriptor(DeviceBase* device,
                                           const BindGroupDescriptor* descriptor) {
        if (descriptor->nextInChain != nullptr) {
            return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
        }

        DAWN_TRY(device->ValidateObject(descriptor->layout));
        if (descriptor->bindingCount != descriptor->layout->GetBindingCount()) {
            return DAWN_VALIDATION_ERROR("numBindings mismatch");
        }

        const BindGroupLayoutBase::BindingMap& bindingMap = descriptor->layout->GetBindingMap();

        std::bitset<kMaxBindingsPerGroup> bindingsSet;
        for (uint32_t i = 0; i < descriptor->bindingCount; ++i) {
            const BindGroupBinding& binding = descriptor->bindings[i];

            const auto& it = bindingMap.find(BindingNumber(binding.binding));
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
            switch (bindingInfo.type) {
                case wgpu::BindingType::UniformBuffer:
                    DAWN_TRY(ValidateBufferBinding(device, binding, wgpu::BufferUsage::Uniform));
                    break;
                case wgpu::BindingType::StorageBuffer:
                case wgpu::BindingType::ReadonlyStorageBuffer:
                    DAWN_TRY(ValidateBufferBinding(device, binding, wgpu::BufferUsage::Storage));
                    break;
                case wgpu::BindingType::SampledTexture:
                    DAWN_TRY(ValidateTextureBinding(device, binding, wgpu::TextureUsage::Sampled,
                                                    bindingInfo));
                    break;
                case wgpu::BindingType::Sampler:
                    DAWN_TRY(ValidateSamplerBinding(device, binding));
                    break;
                // TODO(jiawei.shao@intel.com): support creating bind group with read-only and
                // write-only storage textures.
                case wgpu::BindingType::ReadonlyStorageTexture:
                case wgpu::BindingType::WriteonlyStorageTexture:
                    DAWN_TRY(ValidateTextureBinding(device, binding, wgpu::TextureUsage::Storage,
                                                    bindingInfo));
                    break;
                case wgpu::BindingType::StorageTexture:
                    UNREACHABLE();
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
        for (BindingIndex i = 0; i < mLayout->GetBindingCount(); ++i) {
            // TODO(enga): Shouldn't be needed when bindings are tightly packed.
            // This is to fill Ref<ObjectBase> holes with nullptrs.
            new (&mBindingData.bindings[i]) Ref<ObjectBase>();
        }

        for (uint32_t i = 0; i < descriptor->bindingCount; ++i) {
            const BindGroupBinding& binding = descriptor->bindings[i];

            BindingIndex bindingIndex =
                descriptor->layout->GetBindingIndex(BindingNumber(binding.binding));
            ASSERT(bindingIndex < mLayout->GetBindingCount());

            // Only a single binding type should be set, so once we found it we can skip to the
            // next loop iteration.

            if (binding.buffer != nullptr) {
                ASSERT(mBindingData.bindings[bindingIndex].Get() == nullptr);
                mBindingData.bindings[bindingIndex] = binding.buffer;
                mBindingData.bufferData[bindingIndex].offset = binding.offset;
                uint64_t bufferSize =
                    (binding.size == wgpu::kWholeSize) ? binding.buffer->GetSize() : binding.size;
                mBindingData.bufferData[bindingIndex].size = bufferSize;
                continue;
            }

            if (binding.textureView != nullptr) {
                ASSERT(mBindingData.bindings[bindingIndex].Get() == nullptr);
                mBindingData.bindings[bindingIndex] = binding.textureView;
                continue;
            }

            if (binding.sampler != nullptr) {
                ASSERT(mBindingData.bindings[bindingIndex].Get() == nullptr);
                mBindingData.bindings[bindingIndex] = binding.sampler;
                continue;
            }
        }
    }

    BindGroupBase::~BindGroupBase() {
        if (mLayout) {
            ASSERT(!IsError());
            for (BindingIndex i = 0; i < mLayout->GetBindingCount(); ++i) {
                mBindingData.bindings[i].~Ref<ObjectBase>();
            }
        }
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

    BufferBinding BindGroupBase::GetBindingAsBufferBinding(BindingIndex bindingIndex) {
        ASSERT(!IsError());
        ASSERT(bindingIndex < mLayout->GetBindingCount());
        ASSERT(mLayout->GetBindingInfo(bindingIndex).type == wgpu::BindingType::UniformBuffer ||
               mLayout->GetBindingInfo(bindingIndex).type == wgpu::BindingType::StorageBuffer ||
               mLayout->GetBindingInfo(bindingIndex).type ==
                   wgpu::BindingType::ReadonlyStorageBuffer);
        BufferBase* buffer = static_cast<BufferBase*>(mBindingData.bindings[bindingIndex].Get());
        return {buffer, mBindingData.bufferData[bindingIndex].offset,
                mBindingData.bufferData[bindingIndex].size};
    }

    SamplerBase* BindGroupBase::GetBindingAsSampler(BindingIndex bindingIndex) {
        ASSERT(!IsError());
        ASSERT(bindingIndex < mLayout->GetBindingCount());
        ASSERT(mLayout->GetBindingInfo(bindingIndex).type == wgpu::BindingType::Sampler);
        return static_cast<SamplerBase*>(mBindingData.bindings[bindingIndex].Get());
    }

    TextureViewBase* BindGroupBase::GetBindingAsTextureView(BindingIndex bindingIndex) {
        ASSERT(!IsError());
        ASSERT(bindingIndex < mLayout->GetBindingCount());
        ASSERT(mLayout->GetBindingInfo(bindingIndex).type == wgpu::BindingType::SampledTexture);
        return static_cast<TextureViewBase*>(mBindingData.bindings[bindingIndex].Get());
    }

}  // namespace dawn_native
