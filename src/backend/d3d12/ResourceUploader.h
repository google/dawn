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

#ifndef BACKEND_D3D12_RESOURCEUPLOADER_H_
#define BACKEND_D3D12_RESOURCEUPLOADER_H_

#include "d3d12_platform.h"

#include "common/SerialQueue.h"

#include <map>
#include <vector>

namespace backend {
namespace d3d12 {

    class Device;

    class ResourceUploader {
        public:
            ResourceUploader(Device* device);

            void UploadToBuffer(ComPtr<ID3D12Resource> resource, uint32_t start, uint32_t count, const uint8_t* data);
            void FreeCompletedResources(const uint64_t lastCompletedSerial);

        private:
            Device* device;

            SerialQueue<ComPtr<ID3D12Resource>> uploadingResources;
    };

}
}

#endif // BACKEND_D3D12_RESOURCEUPLOADER_H_
