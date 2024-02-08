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

#include "dawn/wire/client/Device.h"

#include <memory>
#include <string>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/wire/client/ApiObjects_autogen.h"
#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/EventManager.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire::client {
namespace {

class PopErrorScopeEvent final : public TrackedEvent {
  public:
    static constexpr EventType kType = EventType::PopErrorScope;

    explicit PopErrorScopeEvent(const WGPUPopErrorScopeCallbackInfo& callbackInfo)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mOldCallback(callbackInfo.oldCallback),
          mUserdata(callbackInfo.userdata) {
        // Exactly 1 callback should be set.
        DAWN_ASSERT((mCallback != nullptr && mOldCallback == nullptr) ||
                    (mCallback == nullptr && mOldCallback != nullptr));
    }

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID, WGPUErrorType errorType, const char* message) {
        mType = errorType;
        if (message != nullptr) {
            mMessage = message;
        }
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPUPopErrorScopeStatus_InstanceDropped;
            mMessage = std::nullopt;
        }
        if (mOldCallback) {
            mOldCallback(mType, mMessage ? mMessage->c_str() : nullptr, mUserdata);
        }
        if (mCallback) {
            mCallback(mStatus, mType, mMessage ? mMessage->c_str() : nullptr, mUserdata);
        }
    }

    // TODO(crbug.com/dawn/2021) Remove the old callback type.
    WGPUPopErrorScopeCallback mCallback;
    WGPUErrorCallback mOldCallback;
    // TODO(https://crbug.com/dawn/2345): Investigate `DanglingUntriaged` in dawn/wire.
    raw_ptr<void, DanglingUntriaged> mUserdata;

    WGPUPopErrorScopeStatus mStatus = WGPUPopErrorScopeStatus_Success;
    WGPUErrorType mType = WGPUErrorType_Unknown;
    std::optional<std::string> mMessage;
};

template <typename PipelineT, EventType Type, typename CallbackInfoT>
class CreatePipelineEventBase : public TrackedEvent {
  public:
    // Export these types upwards for ease of use.
    using Pipeline = PipelineT;
    using CallbackInfo = CallbackInfoT;

    static constexpr EventType kType = Type;

    CreatePipelineEventBase(const CallbackInfo& callbackInfo, Pipeline* pipeline)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata(callbackInfo.userdata),
          mPipeline(pipeline) {
        DAWN_ASSERT(mPipeline != nullptr);
    }

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID,
                         WGPUCreatePipelineAsyncStatus status,
                         const char* message) {
        DAWN_ASSERT(mPipeline != nullptr);
        mStatus = status;
        if (message != nullptr) {
            mMessage = message;
        }
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPUCreatePipelineAsyncStatus_InstanceDropped;
            mMessage = "A valid external Instance reference no longer exists.";
        }

        // By default, we are initialized to a success state, and on shutdown we just return success
        // so we don't need to handle it specifically.
        if (mStatus != WGPUCreatePipelineAsyncStatus_Success) {
            // If there was an error we need to reclaim the pipeline allocation.
            mPipeline->GetClient()->Free(mPipeline.get());
            mPipeline = nullptr;
        }
        if (mCallback) {
            mCallback(mStatus, ToAPI(mPipeline), mMessage ? mMessage->c_str() : nullptr, mUserdata);
        }
    }

    using Callback = decltype(std::declval<CallbackInfo>().callback);
    Callback mCallback;
    // TODO(https://crbug.com/dawn/2345): Investigate `DanglingUntriaged` in dawn/wire.
    raw_ptr<void, DanglingUntriaged> mUserdata;

    // Note that the message is optional because we want to return nullptr when it wasn't set
    // instead of a pointer to an empty string.
    WGPUCreatePipelineAsyncStatus mStatus = WGPUCreatePipelineAsyncStatus_Success;
    std::optional<std::string> mMessage;

    // TODO(https://crbug.com/dawn/2345): Investigate `DanglingUntriaged` in dawn/wire.
    raw_ptr<Pipeline, DanglingUntriaged> mPipeline = nullptr;
};

using CreateComputePipelineEvent =
    CreatePipelineEventBase<ComputePipeline,
                            EventType::CreateComputePipeline,
                            WGPUCreateComputePipelineAsyncCallbackInfo>;
using CreateRenderPipelineEvent =
    CreatePipelineEventBase<RenderPipeline,
                            EventType::CreateRenderPipeline,
                            WGPUCreateRenderPipelineAsyncCallbackInfo>;

}  // namespace

Device::Device(const ObjectBaseParams& params,
               const ObjectHandle& eventManagerHandle,
               const WGPUDeviceDescriptor* descriptor)
    : ObjectWithEventsBase(params, eventManagerHandle), mIsAlive(std::make_shared<bool>()) {
    if (descriptor && descriptor->deviceLostCallback) {
        mDeviceLostCallback = descriptor->deviceLostCallback;
        mDeviceLostUserdata = descriptor->deviceLostUserdata;
    }

#if defined(DAWN_ENABLE_ASSERTS)
    mErrorCallback = [](WGPUErrorType, char const*, void*) {
        static bool calledOnce = false;
        if (!calledOnce) {
            calledOnce = true;
            dawn::WarningLog() << "No Dawn device uncaptured error callback was set. This is "
                                  "probably not intended. If you really want to ignore errors "
                                  "and suppress this message, set the callback to null.";
        }
    };

    if (!mDeviceLostCallback) {
        mDeviceLostCallback = [](WGPUDeviceLostReason, char const*, void*) {
            static bool calledOnce = false;
            if (!calledOnce) {
                calledOnce = true;
                dawn::WarningLog() << "No Dawn device lost callback was set. This is probably not "
                                      "intended. If you really want to ignore device lost "
                                      "and suppress this message, set the callback to null.";
            }
        };
    }
#endif  // DAWN_ENABLE_ASSERTS
}

Device::~Device() {
    if (mQueue != nullptr) {
        GetProcs().queueRelease(ToAPI(mQueue));
    }
}

ObjectType Device::GetObjectType() const {
    return ObjectType::Device;
}

bool Device::GetLimits(WGPUSupportedLimits* limits) const {
    return mLimitsAndFeatures.GetLimits(limits);
}

bool Device::HasFeature(WGPUFeatureName feature) const {
    return mLimitsAndFeatures.HasFeature(feature);
}

size_t Device::EnumerateFeatures(WGPUFeatureName* features) const {
    return mLimitsAndFeatures.EnumerateFeatures(features);
}

void Device::SetLimits(const WGPUSupportedLimits* limits) {
    return mLimitsAndFeatures.SetLimits(limits);
}

void Device::SetFeatures(const WGPUFeatureName* features, uint32_t featuresCount) {
    return mLimitsAndFeatures.SetFeatures(features, featuresCount);
}

void Device::HandleError(WGPUErrorType errorType, const char* message) {
    if (mErrorCallback) {
        mErrorCallback(errorType, message, mErrorUserdata);
    }
}

void Device::HandleLogging(WGPULoggingType loggingType, const char* message) {
    if (mLoggingCallback) {
        // Since client always run in single thread, calling the callback directly is safe.
        mLoggingCallback(loggingType, message, mLoggingUserdata);
    }
}

void Device::HandleDeviceLost(WGPUDeviceLostReason reason, const char* message) {
    if (mDeviceLostCallback && !mDidRunLostCallback) {
        mDidRunLostCallback = true;
        mDeviceLostCallback(reason, message, mDeviceLostUserdata);
    }
}

std::weak_ptr<bool> Device::GetAliveWeakPtr() {
    return mIsAlive;
}

void Device::SetUncapturedErrorCallback(WGPUErrorCallback errorCallback, void* errorUserdata) {
    mErrorCallback = errorCallback;
    mErrorUserdata = errorUserdata;
}

void Device::SetLoggingCallback(WGPULoggingCallback callback, void* userdata) {
    mLoggingCallback = callback;
    mLoggingUserdata = userdata;
}

void Device::SetDeviceLostCallback(WGPUDeviceLostCallback callback, void* userdata) {
    mDeviceLostCallback = callback;
    mDeviceLostUserdata = userdata;
}

void Device::PopErrorScope(WGPUErrorCallback callback, void* userdata) {
    WGPUPopErrorScopeCallbackInfo callbackInfo = {};
    callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
    callbackInfo.oldCallback = callback;
    callbackInfo.userdata = userdata;
    PopErrorScopeF(callbackInfo);
}

WGPUFuture Device::PopErrorScopeF(const WGPUPopErrorScopeCallbackInfo& callbackInfo) {
    Client* client = GetClient();
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(std::make_unique<PopErrorScopeEvent>(callbackInfo));
    if (!tracked) {
        return {futureIDInternal};
    }

    DevicePopErrorScopeCmd cmd;
    cmd.deviceId = GetWireId();
    cmd.eventManagerHandle = GetEventManagerHandle();
    cmd.future = {futureIDInternal};
    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

WireResult Client::DoDevicePopErrorScopeCallback(ObjectHandle eventManager,
                                                 WGPUFuture future,
                                                 WGPUErrorType errorType,
                                                 const char* message) {
    return GetEventManager(eventManager)
        .SetFutureReady<PopErrorScopeEvent>(future.id, errorType, message);
}

void Device::InjectError(WGPUErrorType type, const char* message) {
    DeviceInjectErrorCmd cmd;
    cmd.self = ToAPI(this);
    cmd.type = type;
    cmd.message = message;
    GetClient()->SerializeCommand(cmd);
}

WGPUBuffer Device::CreateBuffer(const WGPUBufferDescriptor* descriptor) {
    return Buffer::Create(this, descriptor);
}

WGPUQueue Device::GetQueue() {
    // The queue is lazily created because if a Device is created by
    // Reserve/Inject, we cannot send the GetQueue message until
    // it has been injected on the Server. It cannot happen immediately
    // on construction.
    if (mQueue == nullptr) {
        // Get the primary queue for this device.
        Client* client = GetClient();
        mQueue = client->Make<Queue>(GetEventManagerHandle());

        DeviceGetQueueCmd cmd;
        cmd.self = ToAPI(this);
        cmd.result = mQueue->GetWireHandle();

        client->SerializeCommand(cmd);
    }

    mQueue->Reference();
    return ToAPI(mQueue);
}

template <typename Event, typename Cmd, typename CallbackInfo, typename Descriptor>
WGPUFuture Device::CreatePipelineAsyncF(Descriptor const* descriptor,
                                        const CallbackInfo& callbackInfo) {
    using Pipeline = typename Event::Pipeline;

    Client* client = GetClient();
    Pipeline* pipeline = client->Make<Pipeline>();
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(std::make_unique<Event>(callbackInfo, pipeline));
    if (!tracked) {
        return {futureIDInternal};
    }

    Cmd cmd;
    cmd.deviceId = GetWireId();
    cmd.descriptor = descriptor;
    cmd.eventManagerHandle = GetEventManagerHandle();
    cmd.future = {futureIDInternal};
    cmd.pipelineObjectHandle = pipeline->GetWireHandle();

    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

void Device::CreateComputePipelineAsync(WGPUComputePipelineDescriptor const* descriptor,
                                        WGPUCreateComputePipelineAsyncCallback callback,
                                        void* userdata) {
    WGPUCreateComputePipelineAsyncCallbackInfo callbackInfo = {};
    callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
    callbackInfo.callback = callback;
    callbackInfo.userdata = userdata;
    CreateComputePipelineAsyncF(descriptor, callbackInfo);
}

WGPUFuture Device::CreateComputePipelineAsyncF(
    WGPUComputePipelineDescriptor const* descriptor,
    const WGPUCreateComputePipelineAsyncCallbackInfo& callbackInfo) {
    return CreatePipelineAsyncF<CreateComputePipelineEvent, DeviceCreateComputePipelineAsyncCmd>(
        descriptor, callbackInfo);
}

WireResult Client::DoDeviceCreateComputePipelineAsyncCallback(ObjectHandle eventManager,
                                                              WGPUFuture future,
                                                              WGPUCreatePipelineAsyncStatus status,
                                                              const char* message) {
    return GetEventManager(eventManager)
        .SetFutureReady<CreateComputePipelineEvent>(future.id, status, message);
}

void Device::CreateRenderPipelineAsync(WGPURenderPipelineDescriptor const* descriptor,
                                       WGPUCreateRenderPipelineAsyncCallback callback,
                                       void* userdata) {
    WGPUCreateRenderPipelineAsyncCallbackInfo callbackInfo = {};
    callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
    callbackInfo.callback = callback;
    callbackInfo.userdata = userdata;
    CreateRenderPipelineAsyncF(descriptor, callbackInfo);
}

WGPUFuture Device::CreateRenderPipelineAsyncF(
    WGPURenderPipelineDescriptor const* descriptor,
    const WGPUCreateRenderPipelineAsyncCallbackInfo& callbackInfo) {
    return CreatePipelineAsyncF<CreateRenderPipelineEvent, DeviceCreateRenderPipelineAsyncCmd>(
        descriptor, callbackInfo);
}

WireResult Client::DoDeviceCreateRenderPipelineAsyncCallback(ObjectHandle eventManager,
                                                             WGPUFuture future,
                                                             WGPUCreatePipelineAsyncStatus status,
                                                             const char* message) {
    return GetEventManager(eventManager)
        .SetFutureReady<CreateRenderPipelineEvent>(future.id, status, message);
}

}  // namespace dawn::wire::client
