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

#ifndef SRC_DAWN_UTILS_WIREHELPER_H_
#define SRC_DAWN_UTILS_WIREHELPER_H_

#include <cstdint>
#include <memory>
#include <utility>

#include "dawn/webgpu_cpp.h"

struct DawnProcTable;

namespace utils {

class WireHelper {
  public:
    virtual ~WireHelper();

    // Registers the instance on the wire, if present.
    // Returns the wgpu::Instance which is the client instance on the wire, and
    // the backend instance without the wire.
    // The function should not take ownership of |backendInstance|.
    virtual wgpu::Instance RegisterInstance(WGPUInstance backendInstance) = 0;

    virtual void BeginWireTrace(const char* name) = 0;

    virtual bool FlushClient() = 0;
    virtual bool FlushServer() = 0;
};

std::unique_ptr<WireHelper> CreateWireHelper(const DawnProcTable& procs,
                                             bool useWire,
                                             const char* wireTraceDir = nullptr);

}  // namespace utils

#endif  // SRC_DAWN_UTILS_WIREHELPER_H_
