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

#ifndef SRC_DAWN_WIRE_CLIENT_INSTANCE_H_
#define SRC_DAWN_WIRE_CLIENT_INSTANCE_H_

#include "dawn/webgpu.h"

#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/ObjectBase.h"
#include "dawn/wire/client/RequestTracker.h"

namespace dawn::wire::client {

class Instance final : public ObjectBase {
  public:
    using ObjectBase::ObjectBase;
    ~Instance() override;

    void CancelCallbacksForDisconnect() override;

    void RequestAdapter(const WGPURequestAdapterOptions* options,
                        WGPURequestAdapterCallback callback,
                        void* userdata);
    bool OnRequestAdapterCallback(uint64_t requestSerial,
                                  WGPURequestAdapterStatus status,
                                  const char* message,
                                  const WGPUAdapterProperties* properties,
                                  const WGPUSupportedLimits* limits,
                                  uint32_t featuresCount,
                                  const WGPUFeatureName* features);

  private:
    struct RequestAdapterData {
        WGPURequestAdapterCallback callback = nullptr;
        ObjectId adapterObjectId;
        void* userdata = nullptr;
    };
    RequestTracker<RequestAdapterData> mRequestAdapterRequests;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_INSTANCE_H_
