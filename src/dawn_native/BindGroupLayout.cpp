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

#include "dawn_native/BindGroupLayout.h"

#include <functional>

#include "common/BitSetIterator.h"
#include "common/HashUtils.h"
#include "dawn_native/Device.h"
#include "dawn_native/ValidationUtils_autogen.h"

namespace dawn_native {

    MaybeError ValidateBindingTypeWithShaderStageVisibility(
        wgpu::BindingType bindingType,
        wgpu::ShaderStage shaderStageVisibility) {
        // TODO(jiawei.shao@intel.com): support read-write storage textures.
        switch (bindingType) {
            case wgpu::BindingType::StorageBuffer: {
                if ((shaderStageVisibility & wgpu::ShaderStage::Vertex) != 0) {
                    return DAWN_VALIDATION_ERROR(
                        "storage buffer binding is not supported in vertex shader");
                }
            } break;

            case wgpu::BindingType::WriteonlyStorageTexture: {
                if ((shaderStageVisibility &
                     (wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment)) != 0) {
                    return DAWN_VALIDATION_ERROR(
                        "write-only storage texture binding is only supported in compute shader");
                }
            } break;

            case wgpu::BindingType::StorageTexture: {
                return DAWN_VALIDATION_ERROR("Read-write storage texture binding is not supported");
            } break;

            case wgpu::BindingType::UniformBuffer:
            case wgpu::BindingType::ReadonlyStorageBuffer:
            case wgpu::BindingType::Sampler:
            case wgpu::BindingType::SampledTexture:
            case wgpu::BindingType::ReadonlyStorageTexture:
                break;
        }

        return {};
    }

    MaybeError ValidateBindGroupLayoutDescriptor(DeviceBase*,
                                                 const BindGroupLayoutDescriptor* descriptor) {
        if (descriptor->nextInChain != nullptr) {
            return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
        }

        std::bitset<kMaxBindingsPerGroup> bindingsSet;
        uint32_t dynamicUniformBufferCount = 0;
        uint32_t dynamicStorageBufferCount = 0;
        for (uint32_t i = 0; i < descriptor->bindingCount; ++i) {
            const BindGroupLayoutBinding& binding = descriptor->bindings[i];
            DAWN_TRY(ValidateShaderStage(binding.visibility));
            DAWN_TRY(ValidateBindingType(binding.type));
            DAWN_TRY(ValidateTextureComponentType(binding.textureComponentType));

            if (binding.textureDimension != wgpu::TextureViewDimension::Undefined) {
                DAWN_TRY(ValidateTextureViewDimension(binding.textureDimension));
            }

            if (binding.binding >= kMaxBindingsPerGroup) {
                return DAWN_VALIDATION_ERROR("some binding index exceeds the maximum value");
            }
            if (bindingsSet[binding.binding]) {
                return DAWN_VALIDATION_ERROR("some binding index was specified more than once");
            }

            DAWN_TRY(
                ValidateBindingTypeWithShaderStageVisibility(binding.type, binding.visibility));

            switch (binding.type) {
                case wgpu::BindingType::UniformBuffer:
                    if (binding.hasDynamicOffset) {
                        ++dynamicUniformBufferCount;
                    }
                    break;
                case wgpu::BindingType::StorageBuffer:
                case wgpu::BindingType::ReadonlyStorageBuffer:
                    if (binding.hasDynamicOffset) {
                        ++dynamicStorageBufferCount;
                    }
                    break;
                case wgpu::BindingType::SampledTexture:
                case wgpu::BindingType::Sampler:
                case wgpu::BindingType::ReadonlyStorageTexture:
                case wgpu::BindingType::WriteonlyStorageTexture:
                    if (binding.hasDynamicOffset) {
                        return DAWN_VALIDATION_ERROR("Samplers and textures cannot be dynamic");
                    }
                    break;
                case wgpu::BindingType::StorageTexture:
                    return DAWN_VALIDATION_ERROR("storage textures aren't supported (yet)");
            }

            if (binding.multisampled) {
                return DAWN_VALIDATION_ERROR(
                    "BindGroupLayoutBinding::multisampled must be false (for now)");
            }

            bindingsSet.set(binding.binding);
        }

        if (dynamicUniformBufferCount > kMaxDynamicUniformBufferCount) {
            return DAWN_VALIDATION_ERROR(
                "The number of dynamic uniform buffer exceeds the maximum value");
        }

        if (dynamicStorageBufferCount > kMaxDynamicStorageBufferCount) {
            return DAWN_VALIDATION_ERROR(
                "The number of dynamic storage buffer exceeds the maximum value");
        }

        return {};
    }  // namespace dawn_native

    namespace {
        size_t HashBindingInfo(const BindGroupLayoutBase::LayoutBindingInfo& info) {
            size_t hash = Hash(info.mask);
            HashCombine(&hash, info.hasDynamicOffset, info.multisampled);

            for (uint32_t binding : IterateBitSet(info.mask)) {
                HashCombine(&hash, info.visibilities[binding], info.types[binding],
                            info.textureComponentTypes[binding], info.textureDimensions[binding]);
            }

            return hash;
        }

        bool operator==(const BindGroupLayoutBase::LayoutBindingInfo& a,
                        const BindGroupLayoutBase::LayoutBindingInfo& b) {
            if (a.mask != b.mask || a.hasDynamicOffset != b.hasDynamicOffset ||
                a.multisampled != b.multisampled) {
                return false;
            }

            for (uint32_t binding : IterateBitSet(a.mask)) {
                if ((a.visibilities[binding] != b.visibilities[binding]) ||
                    (a.types[binding] != b.types[binding]) ||
                    (a.textureComponentTypes[binding] != b.textureComponentTypes[binding]) ||
                    (a.textureDimensions[binding] != b.textureDimensions[binding])) {
                    return false;
                }
            }

            return true;
        }
    }  // namespace

    // BindGroupLayoutBase

    BindGroupLayoutBase::BindGroupLayoutBase(DeviceBase* device,
                                             const BindGroupLayoutDescriptor* descriptor)
        : CachedObject(device) {
        for (uint32_t i = 0; i < descriptor->bindingCount; ++i) {
            auto& binding = descriptor->bindings[i];

            uint32_t index = binding.binding;
            mBindingInfo.visibilities[index] = binding.visibility;
            mBindingInfo.types[index] = binding.type;
            mBindingInfo.textureComponentTypes[index] = binding.textureComponentType;

            if (binding.textureDimension == wgpu::TextureViewDimension::Undefined) {
                mBindingInfo.textureDimensions[index] = wgpu::TextureViewDimension::e2D;
            } else {
                mBindingInfo.textureDimensions[index] = binding.textureDimension;
            }
            if (binding.hasDynamicOffset) {
                mBindingInfo.hasDynamicOffset.set(index);
                switch (binding.type) {
                    case wgpu::BindingType::UniformBuffer:
                        ++mDynamicUniformBufferCount;
                        break;
                    case wgpu::BindingType::StorageBuffer:
                    case wgpu::BindingType::ReadonlyStorageBuffer:
                        ++mDynamicStorageBufferCount;
                        break;
                    case wgpu::BindingType::SampledTexture:
                    case wgpu::BindingType::Sampler:
                    case wgpu::BindingType::StorageTexture:
                    case wgpu::BindingType::ReadonlyStorageTexture:
                    case wgpu::BindingType::WriteonlyStorageTexture:
                        UNREACHABLE();
                        break;
                }
            }

            mBindingInfo.multisampled.set(index, binding.multisampled);

            ASSERT(!mBindingInfo.mask[index]);
            mBindingInfo.mask.set(index);
        }
    }

    BindGroupLayoutBase::BindGroupLayoutBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : CachedObject(device, tag) {
    }

    BindGroupLayoutBase::~BindGroupLayoutBase() {
        // Do not uncache the actual cached object if we are a blueprint
        if (IsCachedReference()) {
            GetDevice()->UncacheBindGroupLayout(this);
        }
    }

    // static
    BindGroupLayoutBase* BindGroupLayoutBase::MakeError(DeviceBase* device) {
        return new BindGroupLayoutBase(device, ObjectBase::kError);
    }

    const BindGroupLayoutBase::LayoutBindingInfo& BindGroupLayoutBase::GetBindingInfo() const {
        ASSERT(!IsError());
        return mBindingInfo;
    }

    size_t BindGroupLayoutBase::HashFunc::operator()(const BindGroupLayoutBase* bgl) const {
        return HashBindingInfo(bgl->mBindingInfo);
    }

    bool BindGroupLayoutBase::EqualityFunc::operator()(const BindGroupLayoutBase* a,
                                                       const BindGroupLayoutBase* b) const {
        return a->mBindingInfo == b->mBindingInfo;
    }

    uint32_t BindGroupLayoutBase::GetDynamicBufferCount() const {
        return mDynamicStorageBufferCount + mDynamicUniformBufferCount;
    }

    uint32_t BindGroupLayoutBase::GetDynamicUniformBufferCount() const {
        return mDynamicUniformBufferCount;
    }

    uint32_t BindGroupLayoutBase::GetDynamicStorageBufferCount() const {
        return mDynamicStorageBufferCount;
    }

}  // namespace dawn_native
