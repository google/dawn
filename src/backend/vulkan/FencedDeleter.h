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

#ifndef BACKEND_VULKAN_FENCEDDELETER_H_
#define BACKEND_VULKAN_FENCEDDELETER_H_

#include "backend/vulkan/vulkan_platform.h"
#include "common/SerialQueue.h"

namespace backend { namespace vulkan {

    class Device;

    class FencedDeleter {
      public:
        FencedDeleter(Device* device);
        ~FencedDeleter();

        void DeleteWhenUnused(VkBuffer buffer);
        void DeleteWhenUnused(VkDeviceMemory memory);

        void Tick(Serial completedSerial);

      private:
        Device* mDevice = nullptr;
        SerialQueue<VkBuffer> mBuffersToDelete;
        SerialQueue<VkDeviceMemory> mMemoriesToDelete;
    };

}}  // namespace backend::vulkan

#endif  // BACKEND_VULKAN_FENCEDDELETER_H_
