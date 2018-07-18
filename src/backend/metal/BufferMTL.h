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

#ifndef BACKEND_METAL_BUFFERMTL_H_
#define BACKEND_METAL_BUFFERMTL_H_

#include "backend/Buffer.h"
#include "common/SerialQueue.h"

#import <Metal/Metal.h>

namespace backend { namespace metal {

    class Device;

    class Buffer : public BufferBase {
      public:
        Buffer(BufferBuilder* builder);
        ~Buffer();

        id<MTLBuffer> GetMTLBuffer();

        void OnMapCommandSerialFinished(uint32_t mapSerial, uint32_t offset, bool isWrite);

      private:
        void SetSubDataImpl(uint32_t start, uint32_t count, const uint8_t* data) override;
        void MapReadAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) override;
        void MapWriteAsyncImpl(uint32_t serial, uint32_t start, uint32_t count) override;
        void UnmapImpl() override;

        id<MTLBuffer> mMtlBuffer = nil;
    };

    class BufferView : public BufferViewBase {
      public:
        BufferView(BufferViewBuilder* builder);
    };

    class MapRequestTracker {
      public:
        MapRequestTracker(Device* device);
        ~MapRequestTracker();

        void Track(Buffer* buffer, uint32_t mapSerial, uint32_t offset, bool isWrite);
        void Tick(Serial finishedSerial);

      private:
        Device* mDevice;

        struct Request {
            Ref<Buffer> buffer;
            uint32_t mapSerial;
            uint32_t offset;
            bool isWrite;
        };
        SerialQueue<Request> mInflightRequests;
    };

}}  // namespace backend::metal

#endif  // BACKEND_METAL_BUFFERMTL_H_
