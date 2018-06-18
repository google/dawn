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

#ifndef BACKEND_VULKAN_BUFFERVK_H_
#define BACKEND_VULKAN_BUFFERVK_H_

#include "backend/Buffer.h"

#include "backend/vulkan/MemoryAllocator.h"
#include "common/SerialQueue.h"
#include "common/vulkan_platform.h"

namespace backend { namespace vulkan {

    class Device;

    class Buffer : public BufferBase {
      public:
        Buffer(BufferBuilder* builder);
        ~Buffer();

        void OnMapReadCommandSerialFinished(uint32_t mapSerial, const void* data);
        void OnMapWriteCommandSerialFinished(uint32_t mapSerial, void* data);

        VkBuffer GetHandle() const;

        void RecordBarrier(VkCommandBuffer commands,
                           nxt::BufferUsageBit currentUsage,
                           nxt::BufferUsageBit targetUsage) const;

      private:
        void SetSubDataImpl(uint32_t start, uint32_t count, const uint8_t* data) override;
        void MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) override;
        void MapWriteAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) override;
        void UnmapImpl() override;
        void TransitionUsageImpl(nxt::BufferUsageBit currentUsage,
                                 nxt::BufferUsageBit targetUsage) override;

        VkBuffer mHandle = VK_NULL_HANDLE;
        DeviceMemoryAllocation mMemoryAllocation;
    };

    using BufferView = BufferViewBase;

    class MapRequestTracker {
      public:
        MapRequestTracker(Device* device);
        ~MapRequestTracker();

        void Track(Buffer* buffer, uint32_t mapSerial, void* data, bool isWrite);
        void Tick(Serial finishedSerial);

      private:
        Device* mDevice;

        struct Request {
            Ref<Buffer> buffer;
            uint32_t mapSerial;
            void* data;
            bool isWrite;
        };
        SerialQueue<Request> mInflightRequests;
    };

}}  // namespace backend::vulkan

#endif  // BACKEND_VULKAN_BUFFERVK_H_
