// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_WIRE_CLIENT_INSTANCE_H_
#define SRC_DAWN_WIRE_CLIENT_INSTANCE_H_

#include "dawn/webgpu.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/ObjectBase.h"
#include "dawn/wire/client/RequestTracker.h"

namespace dawn::wire::client {

WGPUBool ClientGetInstanceFeatures(WGPUInstanceFeatures* features);
WGPUInstance ClientCreateInstance(WGPUInstanceDescriptor const* descriptor);

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

    void ProcessEvents();
    WGPUWaitStatus WaitAny(size_t count, WGPUFutureWaitInfo* infos, uint64_t timeoutNS);

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
