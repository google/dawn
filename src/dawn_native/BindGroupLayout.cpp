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

#include "common/BitSetIterator.h"
#include "common/HashUtils.h"
#include "dawn_native/Device.h"
#include "dawn_native/ValidationUtils_autogen.h"

#include <functional>

namespace dawn_native {

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
            DAWN_TRY(ValidateShaderStageBit(binding.visibility));
            DAWN_TRY(ValidateBindingType(binding.type));

            if (binding.binding >= kMaxBindingsPerGroup) {
                return DAWN_VALIDATION_ERROR("some binding index exceeds the maximum value");
            }
            if (bindingsSet[binding.binding]) {
                return DAWN_VALIDATION_ERROR("some binding index was specified more than once");
            }

            if (binding.visibility == dawn::ShaderStageBit::None) {
                return DAWN_VALIDATION_ERROR("Visibility of bindings can't be None");
            }

            switch (binding.type) {
                case dawn::BindingType::UniformBuffer:
                    if (binding.dynamic) {
                        ++dynamicUniformBufferCount;
                    }
                    break;
                case dawn::BindingType::StorageBuffer:
                    if (binding.dynamic) {
                        ++dynamicStorageBufferCount;
                    }
                    break;
                case dawn::BindingType::SampledTexture:
                case dawn::BindingType::Sampler:
                    if (binding.dynamic) {
                        return DAWN_VALIDATION_ERROR("Samplers and textures cannot be dynamic");
                    }
                    break;
                case dawn::BindingType::ReadonlyStorageBuffer:
                    return DAWN_VALIDATION_ERROR("readonly storage buffers aren't supported (yet)");
                case dawn::BindingType::StorageTexture:
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
    }

    namespace {
        size_t HashBindingInfo(const BindGroupLayoutBase::LayoutBindingInfo& info) {
            size_t hash = Hash(info.mask);
            HashCombine(&hash, info.dynamic, info.multisampled);

            for (uint32_t binding : IterateBitSet(info.mask)) {
                HashCombine(&hash, info.visibilities[binding], info.types[binding]);
            }

            return hash;
        }

        bool operator==(const BindGroupLayoutBase::LayoutBindingInfo& a,
                        const BindGroupLayoutBase::LayoutBindingInfo& b) {
            if (a.mask != b.mask || a.dynamic != b.dynamic || a.multisampled != b.multisampled) {
                return false;
            }

            for (uint32_t binding : IterateBitSet(a.mask)) {
                if ((a.visibilities[binding] != b.visibilities[binding]) ||
                    (a.types[binding] != b.types[binding])) {
                    return false;
                }
            }

            return true;
        }
    }  // namespace

    // BindGroupLayoutBase

    BindGroupLayoutBase::BindGroupLayoutBase(DeviceBase* device,
                                             const BindGroupLayoutDescriptor* descriptor,
                                             bool blueprint)
        : ObjectBase(device), mIsBlueprint(blueprint) {
        for (uint32_t i = 0; i < descriptor->bindingCount; ++i) {
            auto& binding = descriptor->bindings[i];

            uint32_t index = binding.binding;
            mBindingInfo.visibilities[index] = binding.visibility;
            mBindingInfo.types[index] = binding.type;

            if (binding.dynamic) {
                mBindingInfo.dynamic.set(index);
                switch (binding.type) {
                    case dawn::BindingType::UniformBuffer:
                        ++mDynamicUniformBufferCount;
                        break;
                    case dawn::BindingType::StorageBuffer:
                        ++mDynamicStorageBufferCount;
                        break;
                    case dawn::BindingType::SampledTexture:
                    case dawn::BindingType::Sampler:
                    case dawn::BindingType::ReadonlyStorageBuffer:
                    case dawn::BindingType::StorageTexture:
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
        : ObjectBase(device, tag), mIsBlueprint(true) {
    }

    BindGroupLayoutBase::~BindGroupLayoutBase() {
        // Do not uncache the actual cached object if we are a blueprint
        if (!mIsBlueprint && !IsError()) {
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
