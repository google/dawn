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

#ifndef SRC_DAWN_WIRE_CLIENT_ADAPTER_H_
#define SRC_DAWN_WIRE_CLIENT_ADAPTER_H_

#include <vector>

#include "dawn/webgpu.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/LimitsAndFeatures.h"
#include "dawn/wire/client/ObjectBase.h"
#include "dawn/wire/client/RequestTracker.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire::client {

class Adapter final : public ObjectWithEventsBase {
  public:
    using ObjectWithEventsBase::ObjectWithEventsBase;
    ~Adapter() override;

    void CancelCallbacksForDisconnect() override;
    ObjectType GetObjectType() const override;

    bool GetLimits(WGPUSupportedLimits* limits) const;
    bool HasFeature(WGPUFeatureName feature) const;
    size_t EnumerateFeatures(WGPUFeatureName* features) const;
    void SetLimits(const WGPUSupportedLimits* limits);
    void SetFeatures(const WGPUFeatureName* features, uint32_t featuresCount);
    void SetProperties(const WGPUAdapterProperties* properties);
    void GetProperties(WGPUAdapterProperties* properties) const;
    void RequestDevice(const WGPUDeviceDescriptor* descriptor,
                       WGPURequestDeviceCallback callback,
                       void* userdata);
    WGPUFuture RequestDeviceF(const WGPUDeviceDescriptor* descriptor,
                              const WGPURequestDeviceCallbackInfo& callbackInfo);

    // Unimplementable. Only availale in dawn_native.
    WGPUInstance GetInstance() const;
    WGPUDevice CreateDevice(const WGPUDeviceDescriptor*);

  private:
    LimitsAndFeatures mLimitsAndFeatures;
    WGPUAdapterProperties mProperties;
    std::vector<WGPUMemoryHeapInfo> mMemoryHeapInfo;
    WGPUAdapterPropertiesD3D mD3DProperties;

    struct RequestDeviceData {
        WGPURequestDeviceCallback callback = nullptr;
        ObjectId deviceObjectId;
        // TODO(https://crbug.com/dawn/2345): Investigate `DanglingUntriaged` in dawn/wire.
        raw_ptr<void, DanglingUntriaged> userdata = nullptr;
    };
    RequestTracker<RequestDeviceData> mRequestDeviceRequests;
};

void ClientAdapterPropertiesFreeMembers(WGPUAdapterProperties);
void ClientAdapterPropertiesMemoryHeapsFreeMembers(WGPUAdapterPropertiesMemoryHeaps);

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_ADAPTER_H_
