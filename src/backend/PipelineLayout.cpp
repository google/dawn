// Copyright 2017 The NXT Authors
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

#include "backend/PipelineLayout.h"

#include "backend/BindGroupLayout.h"
#include "backend/Device.h"
#include "common/Assert.h"

namespace backend {

    // PipelineLayoutBase

    PipelineLayoutBase::PipelineLayoutBase(PipelineLayoutBuilder* builder)
        : mBindGroupLayouts(std::move(builder->mBindGroupLayouts)), mMask(builder->mMask) {
    }

    const BindGroupLayoutBase* PipelineLayoutBase::GetBindGroupLayout(size_t group) const {
        ASSERT(group < kMaxBindGroups);
        return mBindGroupLayouts[group].Get();
    }

    const std::bitset<kMaxBindGroups> PipelineLayoutBase::GetBindGroupsLayoutMask() const {
        return mMask;
    }

    std::bitset<kMaxBindGroups> PipelineLayoutBase::InheritedGroupsMask(
        const PipelineLayoutBase* other) const {
        return {GroupsInheritUpTo(other) - 1};
    }

    uint32_t PipelineLayoutBase::GroupsInheritUpTo(const PipelineLayoutBase* other) const {
        for (uint32_t i = 0; i < kMaxBindGroups; ++i) {
            if (!mMask[i] || mBindGroupLayouts[i].Get() != other->mBindGroupLayouts[i].Get()) {
                return i;
            }
        }
        return kMaxBindGroups + 1;
    }

    // PipelineLayoutBuilder

    PipelineLayoutBuilder::PipelineLayoutBuilder(DeviceBase* device) : Builder(device) {
    }

    PipelineLayoutBase* PipelineLayoutBuilder::GetResultImpl() {
        // TODO(cwallez@chromium.org): this is a hack, have the null bind group layout somewhere in
        // the device once we have a cache of BGL
        for (size_t group = 0; group < kMaxBindGroups; ++group) {
            if (!mBindGroupLayouts[group]) {
                mBindGroupLayouts[group] = mDevice->CreateBindGroupLayoutBuilder()->GetResult();
            }
        }

        return mDevice->CreatePipelineLayout(this);
    }

    void PipelineLayoutBuilder::SetBindGroupLayout(uint32_t groupIndex,
                                                   BindGroupLayoutBase* layout) {
        if (groupIndex >= kMaxBindGroups) {
            HandleError("groupIndex is over the maximum allowed");
            return;
        }
        if (mMask[groupIndex]) {
            HandleError("Bind group layout already specified");
            return;
        }

        mBindGroupLayouts[groupIndex] = layout;
        mMask.set(groupIndex);
    }

}  // namespace backend
