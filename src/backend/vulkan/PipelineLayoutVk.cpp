// Copyright 2018 The NXT Authors
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

#include "backend/vulkan/PipelineLayoutVk.h"

#include "backend/vulkan/BindGroupLayoutVk.h"
#include "backend/vulkan/FencedDeleter.h"
#include "backend/vulkan/VulkanBackend.h"

#include "common/BitSetIterator.h"

namespace backend { namespace vulkan {

    PipelineLayout::PipelineLayout(PipelineLayoutBuilder* builder)
        : PipelineLayoutBase(builder), mDevice(ToBackend(builder->GetDevice())) {
        // Compute the array of VkDescriptorSetLayouts that will be chained in the create info.
        // TODO(cwallez@chromium.org) Vulkan doesn't allow holes in this array, should we expose
        // this constraints at the NXT level?
        uint32_t numSetLayouts = 0;
        std::array<VkDescriptorSetLayout, kMaxBindGroups> setLayouts;
        for (uint32_t setIndex : IterateBitSet(GetBindGroupsLayoutMask())) {
            setLayouts[numSetLayouts] = ToBackend(GetBindGroupLayout(setIndex))->GetHandle();
            numSetLayouts++;
        }

        // Specify NXT's push constant range on all pipeline layouts because we don't know which
        // pipelines might use it.
        VkPushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
        pushConstantRange.offset = 0;
        pushConstantRange.size = 4 * kMaxPushConstants;

        VkPipelineLayoutCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.setLayoutCount = numSetLayouts;
        createInfo.pSetLayouts = setLayouts.data();
        createInfo.pushConstantRangeCount = 1;
        createInfo.pPushConstantRanges = &pushConstantRange;

        if (mDevice->fn.CreatePipelineLayout(mDevice->GetVkDevice(), &createInfo, nullptr,
                                             &mHandle) != VK_SUCCESS) {
            ASSERT(false);
        }
    }

    PipelineLayout::~PipelineLayout() {
        if (mHandle != VK_NULL_HANDLE) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(mHandle);
            mHandle = VK_NULL_HANDLE;
        }
    }

    VkPipelineLayout PipelineLayout::GetHandle() const {
        return mHandle;
    }

}}  // namespace backend::vulkan
