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

#include "backend/vulkan/FencedDeleter.h"

#include "backend/vulkan/VulkanBackend.h"

namespace backend { namespace vulkan {

    FencedDeleter::FencedDeleter(Device* device) : mDevice(device) {
    }

    FencedDeleter::~FencedDeleter() {
        ASSERT(mBuffersToDelete.Empty());
        ASSERT(mImagesToDelete.Empty());
        ASSERT(mMemoriesToDelete.Empty());
        ASSERT(mPipelineLayoutsToDelete.Empty());
        ASSERT(mRenderPassesToDelete.Empty());
    }

    void FencedDeleter::DeleteWhenUnused(VkBuffer buffer) {
        mBuffersToDelete.Enqueue(buffer, mDevice->GetSerial());
    }

    void FencedDeleter::DeleteWhenUnused(VkDeviceMemory memory) {
        mMemoriesToDelete.Enqueue(memory, mDevice->GetSerial());
    }

    void FencedDeleter::DeleteWhenUnused(VkImage image) {
        mImagesToDelete.Enqueue(image, mDevice->GetSerial());
    }

    void FencedDeleter::DeleteWhenUnused(VkPipelineLayout layout) {
        mPipelineLayoutsToDelete.Enqueue(layout, mDevice->GetSerial());
    }

    void FencedDeleter::DeleteWhenUnused(VkRenderPass renderPass) {
        mRenderPassesToDelete.Enqueue(renderPass, mDevice->GetSerial());
    }

    void FencedDeleter::Tick(Serial completedSerial) {
        VkDevice vkDevice = mDevice->GetVkDevice();

        // Buffers and images must be deleted before memories because it is invalid to free memory
        // that still have resources bound to it.
        for (VkBuffer buffer : mBuffersToDelete.IterateUpTo(completedSerial)) {
            mDevice->fn.DestroyBuffer(vkDevice, buffer, nullptr);
        }
        mBuffersToDelete.ClearUpTo(completedSerial);
        for (VkImage image : mImagesToDelete.IterateUpTo(completedSerial)) {
            mDevice->fn.DestroyImage(vkDevice, image, nullptr);
        }
        mImagesToDelete.ClearUpTo(completedSerial);

        for (VkDeviceMemory memory : mMemoriesToDelete.IterateUpTo(completedSerial)) {
            mDevice->fn.FreeMemory(vkDevice, memory, nullptr);
        }
        mMemoriesToDelete.ClearUpTo(completedSerial);

        for (VkPipelineLayout layout : mPipelineLayoutsToDelete.IterateUpTo(completedSerial)) {
            mDevice->fn.DestroyPipelineLayout(vkDevice, layout, nullptr);
        }
        mPipelineLayoutsToDelete.ClearUpTo(completedSerial);

        for (VkRenderPass renderPass : mRenderPassesToDelete.IterateUpTo(completedSerial)) {
            mDevice->fn.DestroyRenderPass(vkDevice, renderPass, nullptr);
        }
        mRenderPassesToDelete.ClearUpTo(completedSerial);
    }

}}  // namespace backend::vulkan
