// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_WIRE_CLIENT_DEVICE_H_
#define SRC_DAWN_WIRE_CLIENT_DEVICE_H_

#include <webgpu/webgpu.h>

#include <memory>

#include "dawn/common/LinkedList.h"
#include "dawn/common/RefCountedWithExternalCount.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/ApiObjects_autogen.h"
#include "dawn/wire/client/LimitsAndFeatures.h"
#include "dawn/wire/client/ObjectBase.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire::client {

class Client;
class Queue;

class Device final : public RefCountedWithExternalCount<ObjectWithEventsBase> {
  public:
    Device(const ObjectBaseParams& params,
           const ObjectHandle& eventManagerHandle,
           Adapter* adapter,
           const WGPUDeviceDescriptor* descriptor);

    ObjectType GetObjectType() const override;

    void SetLimits(const WGPUSupportedLimits* limits);
    void SetFeatures(const WGPUFeatureName* features, uint32_t featuresCount);

    bool IsAlive() const;
    WGPUFuture GetDeviceLostFuture();

    void HandleError(WGPUErrorType errorType, WGPUStringView message);
    void HandleLogging(WGPULoggingType loggingType, WGPUStringView message);
    void HandleDeviceLost(WGPUDeviceLostReason reason, WGPUStringView message);
    class DeviceLostEvent;

    // WebGPU API
    void SetUncapturedErrorCallback(WGPUErrorCallback errorCallback, void* errorUserdata);
    void SetLoggingCallback(WGPULoggingCallback errorCallback, void* errorUserdata);
    void SetDeviceLostCallback(WGPUDeviceLostCallback errorCallback, void* errorUserdata);
    void InjectError(WGPUErrorType type, WGPUStringView message);
    void PopErrorScope(WGPUErrorCallback callback, void* userdata);
    WGPUFuture PopErrorScopeF(const WGPUPopErrorScopeCallbackInfo& callbackInfo);
    WGPUFuture PopErrorScope2(const WGPUPopErrorScopeCallbackInfo2& callbackInfo);

    WGPUBuffer CreateBuffer(const WGPUBufferDescriptor* descriptor);
    WGPUBuffer CreateErrorBuffer(const WGPUBufferDescriptor* descriptor);
    void CreateComputePipelineAsync(WGPUComputePipelineDescriptor const* descriptor,
                                    WGPUCreateComputePipelineAsyncCallback callback,
                                    void* userdata);
    WGPUFuture CreateComputePipelineAsyncF(
        WGPUComputePipelineDescriptor const* descriptor,
        const WGPUCreateComputePipelineAsyncCallbackInfo& callbackInfo);
    WGPUFuture CreateComputePipelineAsync2(
        WGPUComputePipelineDescriptor const* descriptor,
        const WGPUCreateComputePipelineAsyncCallbackInfo2& callbackInfo);
    void CreateRenderPipelineAsync(WGPURenderPipelineDescriptor const* descriptor,
                                   WGPUCreateRenderPipelineAsyncCallback callback,
                                   void* userdata);
    WGPUFuture CreateRenderPipelineAsyncF(
        WGPURenderPipelineDescriptor const* descriptor,
        const WGPUCreateRenderPipelineAsyncCallbackInfo& callbackInfo);
    WGPUFuture CreateRenderPipelineAsync2(
        WGPURenderPipelineDescriptor const* descriptor,
        const WGPUCreateRenderPipelineAsyncCallbackInfo2& callbackInfo);

    WGPUStatus GetLimits(WGPUSupportedLimits* limits) const;
    bool HasFeature(WGPUFeatureName feature) const;
    size_t EnumerateFeatures(WGPUFeatureName* features) const;
    void GetFeatures(WGPUSupportedFeatures* features) const;
    WGPUAdapter GetAdapter() const;
    WGPUQueue GetQueue();

    void Destroy();

  private:
    void WillDropLastExternalRef() override;
    template <typename Event,
              typename Cmd,
              typename CallbackInfo = typename Event::CallbackInfo,
              typename Descriptor = decltype(std::declval<Cmd>().descriptor)>
    WGPUFuture CreatePipelineAsyncF(Descriptor const* descriptor, const CallbackInfo& callbackInfo);

    LimitsAndFeatures mLimitsAndFeatures;

    // TODO(crbug.com/dawn/2465): This can probably just be the future id once SetDeviceLostCallback
    // is deprecated, and the callback and userdata moved into the DeviceLostEvent.
    struct DeviceLostInfo {
        FutureID futureID = kNullFutureID;
        std::unique_ptr<TrackedEvent> event = nullptr;
        WGPUDeviceLostCallback2 callback = nullptr;
        raw_ptr<void> userdata1 = nullptr;
        raw_ptr<void> userdata2 = nullptr;
    };
    DeviceLostInfo mDeviceLostInfo;

    WGPUUncapturedErrorCallbackInfo2 mUncapturedErrorCallbackInfo;
    WGPULoggingCallback mLoggingCallback = nullptr;
    raw_ptr<void> mLoggingUserdata = nullptr;

    Ref<Adapter> mAdapter;
    Ref<Queue> mQueue;
    bool mIsAlive = true;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_DEVICE_H_
