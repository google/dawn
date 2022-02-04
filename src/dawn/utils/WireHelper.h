// Copyright 2021 The Dawn Authors
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

#ifndef UTILS_WIREHELPER_H_
#define UTILS_WIREHELPER_H_

#include "dawn/webgpu_cpp.h"

#include <cstdint>
#include <memory>

namespace utils {

    class WireHelper {
      public:
        virtual ~WireHelper();

        // Registers the device on the wire, if present.
        // Returns a pair of the client device and backend device.
        // The function should take ownership of |backendDevice|.
        virtual std::pair<wgpu::Device, WGPUDevice> RegisterDevice(WGPUDevice backendDevice) = 0;

        virtual void BeginWireTrace(const char* name) = 0;

        virtual bool FlushClient() = 0;
        virtual bool FlushServer() = 0;
    };

    std::unique_ptr<WireHelper> CreateWireHelper(bool useWire, const char* wireTraceDir = nullptr);

}  // namespace utils

#endif  // UTILS_WIREHELPER_H_
