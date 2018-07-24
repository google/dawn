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
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Device.h"

namespace backend {

    MaybeError ValidatePipelineLayoutDescriptor(DeviceBase*,
                                                const dawn::PipelineLayoutDescriptor* descriptor) {
        DAWN_TRY_ASSERT(descriptor->nextInChain == nullptr, "nextInChain must be nullptr");
        DAWN_TRY_ASSERT(descriptor->numBindGroupLayouts <= kMaxBindGroups,
                        "too many bind group layouts");
        for (uint32_t i = 0; i < descriptor->numBindGroupLayouts; ++i) {
            DAWN_TRY_ASSERT(descriptor->bindGroupLayouts[i].Get() != nullptr,
                            "bind group layouts may not be null");
        }
        return {};
    }

    // PipelineLayoutBase

    PipelineLayoutBase::PipelineLayoutBase(DeviceBase* device,
                                           const dawn::PipelineLayoutDescriptor* descriptor)
        : mDevice(device) {
        ASSERT(descriptor->numBindGroupLayouts <= kMaxBindGroups);
        for (uint32_t group = 0; group < descriptor->numBindGroupLayouts; ++group) {
            mBindGroupLayouts[group] =
                reinterpret_cast<BindGroupLayoutBase*>(descriptor->bindGroupLayouts[group].Get());
            mMask.set(group);
        }
    }

    const BindGroupLayoutBase* PipelineLayoutBase::GetBindGroupLayout(size_t group) const {
        ASSERT(group < kMaxBindGroups);
        return mBindGroupLayouts[group].Get();
    }

    const std::bitset<kMaxBindGroups> PipelineLayoutBase::GetBindGroupLayoutsMask() const {
        return mMask;
    }

    std::bitset<kMaxBindGroups> PipelineLayoutBase::InheritedGroupsMask(
        const PipelineLayoutBase* other) const {
        return {(1 << GroupsInheritUpTo(other)) - 1u};
    }

    uint32_t PipelineLayoutBase::GroupsInheritUpTo(const PipelineLayoutBase* other) const {
        for (uint32_t i = 0; i < kMaxBindGroups; ++i) {
            if (!mMask[i] || mBindGroupLayouts[i].Get() != other->mBindGroupLayouts[i].Get()) {
                return i;
            }
        }
        return kMaxBindGroups + 1;
    }

    DeviceBase* PipelineLayoutBase::GetDevice() const {
        return mDevice;
    }

}  // namespace backend
