// Copyright 2020 The Dawn Authors
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

#ifndef SRC_DAWN_WIRE_CLIENT_QUEUE_H_
#define SRC_DAWN_WIRE_CLIENT_QUEUE_H_

#include "dawn/webgpu.h"

#include "dawn/wire/WireClient.h"
#include "dawn/wire/client/ObjectBase.h"
#include "dawn/wire/client/RequestTracker.h"

namespace dawn::wire::client {

class Queue final : public ObjectBase {
  public:
    using ObjectBase::ObjectBase;
    ~Queue() override;

    bool OnWorkDoneCallback(uint64_t requestSerial, WGPUQueueWorkDoneStatus status);

    // Dawn API
    void OnSubmittedWorkDone(uint64_t signalValue,
                             WGPUQueueWorkDoneCallback callback,
                             void* userdata);
    void WriteBuffer(WGPUBuffer cBuffer, uint64_t bufferOffset, const void* data, size_t size);
    void WriteTexture(const WGPUImageCopyTexture* destination,
                      const void* data,
                      size_t dataSize,
                      const WGPUTextureDataLayout* dataLayout,
                      const WGPUExtent3D* writeSize);

  private:
    void CancelCallbacksForDisconnect() override;
    void ClearAllCallbacks(WGPUQueueWorkDoneStatus status);

    struct OnWorkDoneData {
        WGPUQueueWorkDoneCallback callback = nullptr;
        void* userdata = nullptr;
    };
    RequestTracker<OnWorkDoneData> mOnWorkDoneRequests;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_QUEUE_H_
