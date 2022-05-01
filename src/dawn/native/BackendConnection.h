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

#ifndef SRC_DAWN_NATIVE_BACKENDCONNECTION_H_
#define SRC_DAWN_NATIVE_BACKENDCONNECTION_H_

#include <memory>
#include <vector>

#include "dawn/native/Adapter.h"
#include "dawn/native/DawnNative.h"

namespace dawn::native {

// An common interface for all backends. Mostly used to create adapters for a particular
// backend.
class BackendConnection {
  public:
    BackendConnection(InstanceBase* instance, wgpu::BackendType type);
    virtual ~BackendConnection() = default;

    wgpu::BackendType GetType() const;
    InstanceBase* GetInstance() const;

    // Returns all the adapters for the system that can be created by the backend, without extra
    // options (such as debug adapters, custom driver libraries, etc.)
    virtual std::vector<Ref<AdapterBase>> DiscoverDefaultAdapters() = 0;

    // Returns new adapters created with the backend-specific options.
    virtual ResultOrError<std::vector<Ref<AdapterBase>>> DiscoverAdapters(
        const AdapterDiscoveryOptionsBase* options);

  private:
    InstanceBase* mInstance = nullptr;
    wgpu::BackendType mType;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BACKENDCONNECTION_H_
