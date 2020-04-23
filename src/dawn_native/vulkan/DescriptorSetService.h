// Copyright 2019 The Dawn Authors
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

#ifndef DAWNNATIVE_VULKAN_DESCRIPTORSETSERVICE_H_
#define DAWNNATIVE_VULKAN_DESCRIPTORSETSERVICE_H_

#include "common/SerialQueue.h"

#include "dawn_native/vulkan/BindGroupLayoutVk.h"

#include <vector>

namespace dawn_native { namespace vulkan {

    class BindGroupLayout;
    class Device;

    // Handles everything related to descriptor sets that isn't tied to a particular
    // BindGroupLayout.
    class DescriptorSetService {
      public:
        DescriptorSetService(Device* device);
        ~DescriptorSetService();

        // Will call layout->FinishDeallocation when the serial is passed.
        void AddDeferredDeallocation(BindGroupLayout* layout, size_t index);

        void Tick(Serial completedSerial);

      private:
        Device* mDevice;

        struct Deallocation {
            Ref<BindGroupLayout> layout;
            size_t index;
        };
        SerialQueue<Deallocation> mDeallocations;
    };

}}  // namespace dawn_native::vulkan

#endif  // DAWNNATIVE_VULKAN_DESCRIPTORSETSERVICE_H_
