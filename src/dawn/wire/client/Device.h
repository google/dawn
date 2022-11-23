// Copyright 2019 The Dawn Authors
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

#ifndef SRC_DAWN_WIRE_CLIENT_DEVICE_H_
#define SRC_DAWN_WIRE_CLIENT_DEVICE_H_

#include <memory>

#include "dawn/common/LinkedList.h"
#include "dawn/webgpu.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/ApiObjects_autogen.h"
#include "dawn/wire/client/LimitsAndFeatures.h"
#include "dawn/wire/client/ObjectBase.h"
#include "dawn/wire/client/RequestTracker.h"

namespace dawn::wire::client {

class Client;
class Queue;

class Device final : public ObjectBase {
  public:
    explicit Device(const ObjectBaseParams& params);
    ~Device() override;

    void SetUncapturedErrorCallback(WGPUErrorCallback errorCallback, void* errorUserdata);
    void SetLoggingCallback(WGPULoggingCallback errorCallback, void* errorUserdata);
    void SetDeviceLostCallback(WGPUDeviceLostCallback errorCallback, void* errorUserdata);
    void InjectError(WGPUErrorType type, const char* message);
    bool PopErrorScope(WGPUErrorCallback callback, void* userdata);
    WGPUBuffer CreateBuffer(const WGPUBufferDescriptor* descriptor);
    WGPUBuffer CreateErrorBuffer(const WGPUBufferDescriptor* descriptor);
    void CreateComputePipelineAsync(WGPUComputePipelineDescriptor const* descriptor,
                                    WGPUCreateComputePipelineAsyncCallback callback,
                                    void* userdata);
    void CreateRenderPipelineAsync(WGPURenderPipelineDescriptor const* descriptor,
                                   WGPUCreateRenderPipelineAsyncCallback callback,
                                   void* userdata);
    WGPUQuerySet CreateQuerySet(const WGPUQuerySetDescriptor* descriptor);
    WGPUTexture CreateTexture(const WGPUTextureDescriptor* descriptor);
    WGPUTexture CreateErrorTexture(const WGPUTextureDescriptor* descriptor);

    void HandleError(WGPUErrorType errorType, const char* message);
    void HandleLogging(WGPULoggingType loggingType, const char* message);
    void HandleDeviceLost(WGPUDeviceLostReason reason, const char* message);
    bool OnPopErrorScopeCallback(uint64_t requestSerial, WGPUErrorType type, const char* message);
    bool OnCreateComputePipelineAsyncCallback(uint64_t requestSerial,
                                              WGPUCreatePipelineAsyncStatus status,
                                              const char* message);
    bool OnCreateRenderPipelineAsyncCallback(uint64_t requestSerial,
                                             WGPUCreatePipelineAsyncStatus status,
                                             const char* message);

    bool GetLimits(WGPUSupportedLimits* limits) const;
    bool HasFeature(WGPUFeatureName feature) const;
    size_t EnumerateFeatures(WGPUFeatureName* features) const;
    void SetLimits(const WGPUSupportedLimits* limits);
    void SetFeatures(const WGPUFeatureName* features, uint32_t featuresCount);

    WGPUAdapter GetAdapter();  // Not implemented in the wire.
    WGPUQueue GetQueue();

    void CancelCallbacksForDisconnect() override;

    std::weak_ptr<bool> GetAliveWeakPtr();

  private:
    LimitsAndFeatures mLimitsAndFeatures;
    struct ErrorScopeData {
        WGPUErrorCallback callback = nullptr;
        void* userdata = nullptr;
    };
    RequestTracker<ErrorScopeData> mErrorScopes;

    struct CreatePipelineAsyncRequest {
        WGPUCreateComputePipelineAsyncCallback createComputePipelineAsyncCallback = nullptr;
        WGPUCreateRenderPipelineAsyncCallback createRenderPipelineAsyncCallback = nullptr;
        void* userdata = nullptr;
        ObjectId pipelineObjectID;
    };
    RequestTracker<CreatePipelineAsyncRequest> mCreatePipelineAsyncRequests;

    WGPUErrorCallback mErrorCallback = nullptr;
    WGPUDeviceLostCallback mDeviceLostCallback = nullptr;
    WGPULoggingCallback mLoggingCallback = nullptr;
    bool mDidRunLostCallback = false;
    void* mErrorUserdata = nullptr;
    void* mDeviceLostUserdata = nullptr;
    void* mLoggingUserdata = nullptr;

    Queue* mQueue = nullptr;

    std::shared_ptr<bool> mIsAlive;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_DEVICE_H_
