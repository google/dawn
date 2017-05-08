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

#include "PipelineLayout.h"

#include "BindGroupLayout.h"
#include "Device.h"

namespace backend {

    // PipelineLayoutBase

    PipelineLayoutBase::PipelineLayoutBase(PipelineLayoutBuilder* builder)
        : bindGroupLayouts(std::move(builder->bindGroupLayouts)), mask(builder->mask) {
    }

    const BindGroupLayoutBase* PipelineLayoutBase::GetBindGroupLayout(size_t group) const {
        ASSERT(group < kMaxBindGroups);
        return bindGroupLayouts[group].Get();
    }

    const std::bitset<kMaxBindGroups> PipelineLayoutBase::GetBindGroupsLayoutMask() const {
        return mask;
    }

    // PipelineLayoutBuilder

    PipelineLayoutBuilder::PipelineLayoutBuilder(DeviceBase* device) : Builder(device) {
    }

    PipelineLayoutBase* PipelineLayoutBuilder::GetResult() {
        // TODO(cwallez@chromium.org): this is a hack, have the null bind group layout somewhere in the device
        // once we have a cache of BGL
        for (size_t group = 0; group < kMaxBindGroups; ++group) {
            if (!bindGroupLayouts[group]) {
                bindGroupLayouts[group] = device->CreateBindGroupLayoutBuilder()->GetResult();
            }
        }

        MarkConsumed();
        return device->CreatePipelineLayout(this);
    }

    void PipelineLayoutBuilder::SetBindGroupLayout(uint32_t groupIndex, BindGroupLayoutBase* layout) {
        if (groupIndex >= kMaxBindGroups) {
            HandleError("groupIndex is over the maximum allowed");
            return;
        }
        if (mask[groupIndex]) {
            HandleError("Bind group layout already specified");
            return;
        }

        bindGroupLayouts[groupIndex] = layout;
        mask.set(groupIndex);
    }

}
