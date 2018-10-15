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
#include "dawn_native/Texture.h"

namespace dawn_native {

    // BindGroup

    BindGroupBase::BindGroupBase(BindGroupBuilder* builder)
        : ObjectBase(builder->GetDevice()),
          mLayout(std::move(builder->mLayout)),
          mBindings(std::move(builder->mBindings)) {
    }

    const BindGroupLayoutBase* BindGroupBase::GetLayout() const {
        return mLayout.Get();
    }

    BufferViewBase* BindGroupBase::GetBindingAsBufferView(size_t binding) {
        ASSERT(binding < kMaxBindingsPerGroup);
        ASSERT(mLayout->GetBindingInfo().mask[binding]);
        ASSERT(mLayout->GetBindingInfo().types[binding] == dawn::BindingType::UniformBuffer ||
               mLayout->GetBindingInfo().types[binding] == dawn::BindingType::StorageBuffer);
        return reinterpret_cast<BufferViewBase*>(mBindings[binding].Get());
    }

    SamplerBase* BindGroupBase::GetBindingAsSampler(size_t binding) {
        ASSERT(binding < kMaxBindingsPerGroup);
        ASSERT(mLayout->GetBindingInfo().mask[binding]);
        ASSERT(mLayout->GetBindingInfo().types[binding] == dawn::BindingType::Sampler);
        return reinterpret_cast<SamplerBase*>(mBindings[binding].Get());
    }

    TextureViewBase* BindGroupBase::GetBindingAsTextureView(size_t binding) {
        ASSERT(binding < kMaxBindingsPerGroup);
        ASSERT(mLayout->GetBindingInfo().mask[binding]);
        ASSERT(mLayout->GetBindingInfo().types[binding] == dawn::BindingType::SampledTexture);
        return reinterpret_cast<TextureViewBase*>(mBindings[binding].Get());
    }

    // BindGroupBuilder

    enum BindGroupSetProperties {
        BINDGROUP_PROPERTY_LAYOUT = 0x1,
    };

    BindGroupBuilder::BindGroupBuilder(DeviceBase* device) : Builder(device) {
    }

    BindGroupBase* BindGroupBuilder::GetResultImpl() {
        constexpr int allProperties = BINDGROUP_PROPERTY_LAYOUT;
        if ((mPropertiesSet & allProperties) != allProperties) {
            HandleError("Bindgroup missing properties");
            return nullptr;
        }

        if (mSetMask != mLayout->GetBindingInfo().mask) {
            HandleError("Bindgroup missing bindings");
            return nullptr;
        }

        return GetDevice()->CreateBindGroup(this);
    }

    void BindGroupBuilder::SetLayout(BindGroupLayoutBase* layout) {
        if ((mPropertiesSet & BINDGROUP_PROPERTY_LAYOUT) != 0) {
            HandleError("Bindgroup layout property set multiple times");
            return;
        }

        mLayout = layout;
        mPropertiesSet |= BINDGROUP_PROPERTY_LAYOUT;
    }

    void BindGroupBuilder::SetBufferViews(uint32_t start,
                                          uint32_t count,
                                          BufferViewBase* const* bufferViews) {
        if (!SetBindingsValidationBase(start, count)) {
            return;
        }

        const auto& layoutInfo = mLayout->GetBindingInfo();
        for (size_t i = start, j = 0; i < start + count; ++i, ++j) {
            dawn::BufferUsageBit requiredBit = dawn::BufferUsageBit::None;
            switch (layoutInfo.types[i]) {
                case dawn::BindingType::UniformBuffer:
                    requiredBit = dawn::BufferUsageBit::Uniform;
                    break;

                case dawn::BindingType::StorageBuffer:
                    requiredBit = dawn::BufferUsageBit::Storage;
                    break;

                case dawn::BindingType::Sampler:
                case dawn::BindingType::SampledTexture:
                    HandleError("Setting buffer for a wrong binding type");
                    return;
            }

            if (!(bufferViews[j]->GetBuffer()->GetUsage() & requiredBit)) {
                HandleError("Buffer needs to allow the correct usage bit");
                return;
            }

            if (!IsAligned(bufferViews[j]->GetOffset(), 256)) {
                HandleError("Buffer view offset for bind group needs to be 256-byte aligned");
                return;
            }
        }

        SetBindingsBase(start, count, reinterpret_cast<ObjectBase* const*>(bufferViews));
    }

    void BindGroupBuilder::SetSamplers(uint32_t start,
                                       uint32_t count,
                                       SamplerBase* const* samplers) {
        if (!SetBindingsValidationBase(start, count)) {
            return;
        }

        const auto& layoutInfo = mLayout->GetBindingInfo();
        for (size_t i = start, j = 0; i < start + count; ++i, ++j) {
            if (layoutInfo.types[i] != dawn::BindingType::Sampler) {
                HandleError("Setting binding for a wrong layout binding type");
                return;
            }
        }

        SetBindingsBase(start, count, reinterpret_cast<ObjectBase* const*>(samplers));
    }

    void BindGroupBuilder::SetTextureViews(uint32_t start,
                                           uint32_t count,
                                           TextureViewBase* const* textureViews) {
        if (!SetBindingsValidationBase(start, count)) {
            return;
        }

        const auto& layoutInfo = mLayout->GetBindingInfo();
        for (size_t i = start, j = 0; i < start + count; ++i, ++j) {
            if (layoutInfo.types[i] != dawn::BindingType::SampledTexture) {
                HandleError("Setting binding for a wrong layout binding type");
                return;
            }

            if (!(textureViews[j]->GetTexture()->GetUsage() & dawn::TextureUsageBit::Sampled)) {
                HandleError("Texture needs to allow the sampled usage bit");
                return;
            }
        }

        SetBindingsBase(start, count, reinterpret_cast<ObjectBase* const*>(textureViews));
    }

    void BindGroupBuilder::SetBindingsBase(uint32_t start,
                                           uint32_t count,
                                           ObjectBase* const* objects) {
        for (size_t i = start, j = 0; i < start + count; ++i, ++j) {
            mSetMask.set(i);
            mBindings[i] = objects[j];
        }
    }

    bool BindGroupBuilder::SetBindingsValidationBase(uint32_t start, uint32_t count) {
        if (start + count > kMaxBindingsPerGroup) {
            HandleError("Setting bindings type over maximum number of bindings");
            return false;
        }

        if ((mPropertiesSet & BINDGROUP_PROPERTY_LAYOUT) == 0) {
            HandleError("Bindgroup layout must be set before views");
            return false;
        }

        const auto& layoutInfo = mLayout->GetBindingInfo();
        for (size_t i = start, j = 0; i < start + count; ++i, ++j) {
            if (mSetMask[i]) {
                HandleError("Setting already set binding");
                return false;
            }

            if (!layoutInfo.mask[i]) {
                HandleError("Setting binding that isn't present in the layout");
                return false;
            }
        }

        return true;
    }
}  // namespace dawn_native
