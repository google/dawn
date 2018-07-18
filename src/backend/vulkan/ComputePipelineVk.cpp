// Copyright 2018 The Dawn Authors
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

#include "backend/vulkan/ComputePipelineVk.h"

#include "backend/vulkan/DeviceVk.h"
#include "backend/vulkan/FencedDeleter.h"
#include "backend/vulkan/PipelineLayoutVk.h"
#include "backend/vulkan/ShaderModuleVk.h"

namespace backend { namespace vulkan {

    ComputePipeline::ComputePipeline(ComputePipelineBuilder* builder)
        : ComputePipelineBase(builder), mDevice(ToBackend(builder->GetDevice())) {
        VkComputePipelineCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.layout = ToBackend(GetLayout())->GetHandle();
        createInfo.basePipelineHandle = VK_NULL_HANDLE;
        createInfo.basePipelineIndex = -1;

        const auto& stageInfo = builder->GetStageInfo(dawn::ShaderStage::Compute);
        createInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.stage.pNext = nullptr;
        createInfo.stage.flags = 0;
        createInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        createInfo.stage.module = ToBackend(stageInfo.module)->GetHandle();
        createInfo.stage.pName = stageInfo.entryPoint.c_str();
        createInfo.stage.pSpecializationInfo = nullptr;

        if (mDevice->fn.CreateComputePipelines(mDevice->GetVkDevice(), VK_NULL_HANDLE, 1,
                                               &createInfo, nullptr, &mHandle) != VK_SUCCESS) {
            ASSERT(false);
        }
    }

    ComputePipeline::~ComputePipeline() {
        if (mHandle != VK_NULL_HANDLE) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(mHandle);
            mHandle = VK_NULL_HANDLE;
        }
    }

    VkPipeline ComputePipeline::GetHandle() const {
        return mHandle;
    }

}}  // namespace backend::vulkan
