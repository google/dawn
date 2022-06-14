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

#ifndef SRC_DAWN_WIRE_CLIENT_ADAPTER_H_
#define SRC_DAWN_WIRE_CLIENT_ADAPTER_H_

#include "dawn/webgpu.h"

#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/LimitsAndFeatures.h"
#include "dawn/wire/client/ObjectBase.h"
#include "dawn/wire/client/RequestTracker.h"

namespace dawn::wire::client {

class Adapter final : public ObjectBase {
  public:
    using ObjectBase::ObjectBase;
    ~Adapter() override;

    void CancelCallbacksForDisconnect() override;

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

    bool OnRequestDeviceCallback(uint64_t requestSerial,
                                 WGPURequestDeviceStatus status,
                                 const char* message,
                                 const WGPUSupportedLimits* limits,
                                 uint32_t featuresCount,
                                 const WGPUFeatureName* features);

    // Unimplementable. Only availale in dawn_native.
    WGPUDevice CreateDevice(const WGPUDeviceDescriptor*);

  private:
    LimitsAndFeatures mLimitsAndFeatures;
    WGPUAdapterProperties mProperties;

    struct RequestDeviceData {
        WGPURequestDeviceCallback callback = nullptr;
        ObjectId deviceObjectId;
        void* userdata = nullptr;
    };
    RequestTracker<RequestDeviceData> mRequestDeviceRequests;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_ADAPTER_H_
