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

#ifndef DAWNNATIVE_METAL_RESOURCEUPLOADER_H_
#define DAWNNATIVE_METAL_RESOURCEUPLOADER_H_

#include "common/Serial.h"
#include "common/SerialQueue.h"

#import <Metal/Metal.h>

namespace dawn_native { namespace metal {

    class Device;

    class ResourceUploader {
      public:
        ResourceUploader(Device* device);
        ~ResourceUploader();

        void BufferSubData(id<MTLBuffer> buffer, uint32_t start, uint32_t size, const void* data);
        void Tick(Serial finishedSerial);

      private:
        Device* mDevice;
        SerialQueue<id<MTLBuffer>> mInflightUploadBuffers;
    };

}}  // namespace dawn_native::metal

#endif  // DAWNNATIVE_METAL_RESOURCEUPLOADER_H_
