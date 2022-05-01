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

#ifndef SRC_DAWN_NATIVE_VULKAN_FENCEDDELETER_H_
#define SRC_DAWN_NATIVE_VULKAN_FENCEDDELETER_H_

#include "dawn/common/SerialQueue.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/IntegerTypes.h"

namespace dawn::native::vulkan {

class Device;

class FencedDeleter {
  public:
    explicit FencedDeleter(Device* device);
    ~FencedDeleter();

    void DeleteWhenUnused(VkBuffer buffer);
    void DeleteWhenUnused(VkDescriptorPool pool);
    void DeleteWhenUnused(VkDeviceMemory memory);
    void DeleteWhenUnused(VkFramebuffer framebuffer);
    void DeleteWhenUnused(VkImage image);
    void DeleteWhenUnused(VkImageView view);
    void DeleteWhenUnused(VkPipelineLayout layout);
    void DeleteWhenUnused(VkRenderPass renderPass);
    void DeleteWhenUnused(VkPipeline pipeline);
    void DeleteWhenUnused(VkQueryPool querypool);
    void DeleteWhenUnused(VkSampler sampler);
    void DeleteWhenUnused(VkSemaphore semaphore);
    void DeleteWhenUnused(VkShaderModule module);
    void DeleteWhenUnused(VkSurfaceKHR surface);
    void DeleteWhenUnused(VkSwapchainKHR swapChain);

    void Tick(ExecutionSerial completedSerial);

  private:
    Device* mDevice = nullptr;
    SerialQueue<ExecutionSerial, VkBuffer> mBuffersToDelete;
    SerialQueue<ExecutionSerial, VkDescriptorPool> mDescriptorPoolsToDelete;
    SerialQueue<ExecutionSerial, VkDeviceMemory> mMemoriesToDelete;
    SerialQueue<ExecutionSerial, VkFramebuffer> mFramebuffersToDelete;
    SerialQueue<ExecutionSerial, VkImage> mImagesToDelete;
    SerialQueue<ExecutionSerial, VkImageView> mImageViewsToDelete;
    SerialQueue<ExecutionSerial, VkPipeline> mPipelinesToDelete;
    SerialQueue<ExecutionSerial, VkPipelineLayout> mPipelineLayoutsToDelete;
    SerialQueue<ExecutionSerial, VkQueryPool> mQueryPoolsToDelete;
    SerialQueue<ExecutionSerial, VkRenderPass> mRenderPassesToDelete;
    SerialQueue<ExecutionSerial, VkSampler> mSamplersToDelete;
    SerialQueue<ExecutionSerial, VkSemaphore> mSemaphoresToDelete;
    SerialQueue<ExecutionSerial, VkShaderModule> mShaderModulesToDelete;
    SerialQueue<ExecutionSerial, VkSurfaceKHR> mSurfacesToDelete;
    SerialQueue<ExecutionSerial, VkSwapchainKHR> mSwapChainsToDelete;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_FENCEDDELETER_H_
