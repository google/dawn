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
        ASSERT(mMemoriesToDelete.Empty());
    }

    void FencedDeleter::DeleteWhenUnused(VkBuffer buffer) {
        mBuffersToDelete.Enqueue(buffer, mDevice->GetSerial());
    }

    void FencedDeleter::DeleteWhenUnused(VkDeviceMemory memory) {
        mMemoriesToDelete.Enqueue(memory, mDevice->GetSerial());
    }

    void FencedDeleter::Tick(Serial completedSerial) {
        // Buffers and textures must be deleted before memories because it is invalid to free memory
        // that still have resources bound to it.
        for (VkBuffer buffer : mBuffersToDelete.IterateUpTo(completedSerial)) {
            mDevice->fn.DestroyBuffer(mDevice->GetVkDevice(), buffer, nullptr);
        }
        mBuffersToDelete.ClearUpTo(completedSerial);

        for (VkDeviceMemory memory : mMemoriesToDelete.IterateUpTo(completedSerial)) {
            mDevice->fn.FreeMemory(mDevice->GetVkDevice(), memory, nullptr);
        }
        mMemoriesToDelete.ClearUpTo(completedSerial);
    }

}}  // namespace backend::vulkan
