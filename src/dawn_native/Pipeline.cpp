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

#include "dawn_native/Pipeline.h"

#include "dawn_native/DepthStencilState.h"
#include "dawn_native/Device.h"
#include "dawn_native/InputState.h"
#include "dawn_native/PipelineLayout.h"
#include "dawn_native/ShaderModule.h"

namespace dawn_native {

    // PipelineBase

    PipelineBase::PipelineBase(DeviceBase* device,
                               PipelineLayoutBase* layout,
                               dawn::ShaderStageBit stages)
        : ObjectBase(device), mStageMask(stages), mLayout(layout), mDevice(device) {
    }

    void PipelineBase::ExtractModuleData(dawn::ShaderStage stage, ShaderModuleBase* module) {
        PushConstantInfo* info = &mPushConstants[stage];

        const auto& moduleInfo = module->GetPushConstants();
        info->mask = moduleInfo.mask;

        for (uint32_t i = 0; i < moduleInfo.names.size(); i++) {
            uint32_t size = moduleInfo.sizes[i];
            if (size == 0) {
                continue;
            }

            for (uint32_t offset = 0; offset < size; offset++) {
                info->types[i + offset] = moduleInfo.types[i];
            }
            i += size - 1;
        }
    }

    const PipelineBase::PushConstantInfo& PipelineBase::GetPushConstants(
        dawn::ShaderStage stage) const {
        return mPushConstants[stage];
    }

    dawn::ShaderStageBit PipelineBase::GetStageMask() const {
        return mStageMask;
    }

    PipelineLayoutBase* PipelineBase::GetLayout() {
        return mLayout.Get();
    }

    DeviceBase* PipelineBase::GetDevice() const {
        return mDevice;
    }

}  // namespace dawn_native
