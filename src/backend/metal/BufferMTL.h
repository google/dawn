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

#ifndef BACKEND_METAL_BUFFERMTL_H_
#define BACKEND_METAL_BUFFERMTL_H_

#include "backend/Buffer.h"
#include "common/SerialQueue.h"

#import <Metal/Metal.h>

namespace backend {
namespace metal {

    class Device;

    class Buffer : public BufferBase {
        public:
            Buffer(BufferBuilder* builder);
            ~Buffer();

            id<MTLBuffer> GetMTLBuffer();

            void OnMapReadCommandSerialFinished(uint32_t mapSerial, uint32_t offset);

        private:
            void SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) override;
            void MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) override;
            void UnmapImpl() override;
            void TransitionUsageImpl(nxt::BufferUsageBit currentUsage, nxt::BufferUsageBit targetUsage) override;

            id<MTLBuffer> mtlBuffer = nil;
    };

    class BufferView : public BufferViewBase {
        public:
            BufferView(BufferViewBuilder* builder);
    };

    class MapReadRequestTracker {
        public:
            MapReadRequestTracker(Device* device);
            ~MapReadRequestTracker();

            void Track(Buffer* buffer, uint32_t mapSerial, uint32_t offset);
            void Tick(Serial finishedSerial);

        private:
            Device* device;

            struct Request {
                Ref<Buffer> buffer;
                uint32_t mapSerial;
                uint32_t offset;
            };
            SerialQueue<Request> inflightRequests;
    };

}
}

#endif // BACKEND_METAL_BUFFERMTL_H_
