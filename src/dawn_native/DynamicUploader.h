// Copyright 2018 The Dawn Authors
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

#ifndef DAWNNATIVE_DYNAMICUPLOADER_H_
#define DAWNNATIVE_DYNAMICUPLOADER_H_

#include "dawn_native/Forward.h"
#include "dawn_native/RingBuffer.h"

#include <memory>

// DynamicUploader is the front-end implementation used to manage multiple ring buffers for upload
// usage.
namespace dawn_native {

    class DynamicUploader {
      public:
        DynamicUploader(DeviceBase* device);
        ~DynamicUploader() = default;

        ResultOrError<UploadHandle> Allocate(uint32_t requiredSize, uint32_t alignment);
        void Tick(Serial lastCompletedSerial);

        RingBuffer* GetLargestBuffer();

        MaybeError CreateAndAppendBuffer(size_t size);

        bool IsEmpty() const;

      private:
        std::vector<std::unique_ptr<RingBuffer>> mRingBuffers;
        DeviceBase* mDevice;
    };
}  // namespace dawn_native

#endif  // DAWNNATIVE_DYNAMICUPLOADER_H_