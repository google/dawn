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
        void* userdata = mUserdata.ExtractAsDangling();
        if (mOldCallback) {
            mOldCallback(mType, mMessage ? mMessage->c_str() : nullptr, userdata);
        }
        if (mCallback) {
            mCallback(mStatus, mType, mMessage ? mMessage->c_str() : nullptr, userdata);
        }
    }

    // TODO(crbug.com/dawn/2021) Remove the old callback type.
    WGPUPopErrorScopeCallback mCallback;
    WGPUErrorCallback mOldCallback;
    raw_ptr<void> mUserdata;

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
        if (mCallback == nullptr) {
            // If there's no callback, just clean up the resources.
            mPipeline.ExtractAsDangling()->Release();
            mUserdata.ExtractAsDangling();
            return;
        }

        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPUCreatePipelineAsyncStatus_InstanceDropped;
            mMessage = "A valid external Instance reference no longer exists.";
        }

        Pipeline* pipeline = mPipeline.ExtractAsDangling();
        mCallback(mStatus,
                  ToAPI(mStatus == WGPUCreatePipelineAsyncStatus_Success ? pipeline : nullptr),
                  mMessage ? mMessage->c_str() : nullptr, mUserdata.ExtractAsDangling());
    }

    using Callback = decltype(std::declval<CallbackInfo>().callback);
    Callback mCallback;
    raw_ptr<void> mUserdata;

    // Note that the message is optional because we want to return nullptr when it wasn't set
    // instead of a pointer to an empty string.
    WGPUCreatePipelineAsyncStatus mStatus = WGPUCreatePipelineAsyncStatus_Success;
    std::optional<std::string> mMessage;

    raw_ptr<Pipeline> mPipeline = nullptr;
};

using CreateComputePipelineEvent =
    CreatePipelineEventBase<ComputePipeline,
                            EventType::CreateComputePipeline,
                            WGPUCreateComputePipelineAsyncCallbackInfo>;
using CreateRenderPipelineEvent =
    CreatePipelineEventBase<RenderPipeline,
                            EventType::CreateRenderPipeline,
                            WGPUCreateRenderPipelineAsyncCallbackInfo>;

static constexpr WGPUUncapturedErrorCallbackInfo kEmptyUncapturedErrorCallbackInfo = {
    nullptr, nullptr, nullptr};

}  // namespace

class Device::DeviceLostEvent : public TrackedEvent {
  public:
    static constexpr EventType kType = EventType::DeviceLost;

    DeviceLostEvent(const WGPUDeviceLostCallbackInfo& callbackInfo, Device* device)
        : TrackedEvent(callbackInfo.mode), mDevice(device) {
        DAWN_ASSERT(device != nullptr);
        mDevice->AddRef();
    }

    ~DeviceLostEvent() override { mDevice.ExtractAsDangling()->Release(); }

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID, WGPUDeviceLostReason reason, const char* message) {
        mReason = reason;
        if (message != nullptr) {
            mMessage = message;
        }
        mDevice->mDeviceLostInfo.futureID = kNullFutureID;
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        if (completionType == EventCompletionType::Shutdown) {
            mReason = WGPUDeviceLostReason_InstanceDropped;
            mMessage = "A valid external Instance reference no longer exists.";
        }

        void* userdata = mDevice->mDeviceLostInfo.userdata.ExtractAsDangling();
        if (mDevice->mDeviceLostInfo.oldCallback != nullptr) {
            mDevice->mDeviceLostInfo.oldCallback(mReason, mMessage ? mMessage->c_str() : nullptr,
                                                 userdata);
        } else if (mDevice->mDeviceLostInfo.callback != nullptr) {
            auto device = mReason != WGPUDeviceLostReason_FailedCreation ? ToAPI(mDevice) : nullptr;
            mDevice->mDeviceLostInfo.callback(&device, mReason,
                                              mMessage ? mMessage->c_str() : nullptr, userdata);
        }
        mDevice->mUncapturedErrorCallbackInfo = kEmptyUncapturedErrorCallbackInfo;
    }

    WGPUDeviceLostReason mReason;
    // Note that the message is optional because we want to return nullptr when it wasn't set
    // instead of a pointer to an empty string.
    std::optional<std::string> mMessage;

    // Strong reference to the device so that when we call the callback we can pass the device.
    raw_ptr<Device> mDevice;
};

Device::Device(const ObjectBaseParams& params,
               const ObjectHandle& eventManagerHandle,
               const WGPUDeviceDescriptor* descriptor)
    : ObjectWithEventsBase(params, eventManagerHandle), mIsAlive(std::make_shared<bool>(true)) {
#if defined(DAWN_ENABLE_ASSERTS)
    static constexpr WGPUDeviceLostCallbackInfo kDefaultDeviceLostCallbackInfo = {
        nullptr, WGPUCallbackMode_AllowSpontaneous,
        [](WGPUDevice const*, WGPUDeviceLostReason, char const*, void*) {
            static bool calledOnce = false;
            if (!calledOnce) {
                calledOnce = true;
                dawn::WarningLog() << "No Dawn device lost callback was set. This is probably not "
                                      "intended. If you really want to ignore device lost "
                                      "and suppress this message, set the callback explicitly.";
            }
        },
        nullptr};
    static constexpr WGPUUncapturedErrorCallbackInfo kDefaultUncapturedErrorCallbackInfo = {
        nullptr,
        [](WGPUErrorType, char const*, void*) {
            static bool calledOnce = false;
            if (!calledOnce) {
                calledOnce = true;
                dawn::WarningLog() << "No Dawn device uncaptured error callback was set. This is "
                                      "probably not intended. If you really want to ignore errors "
                                      "and suppress this message, set the callback explicitly.";
            }
        },
        nullptr};
#else
    static constexpr WGPUDeviceLostCallbackInfo kDefaultDeviceLostCallbackInfo = {
        nullptr, WGPUCallbackMode_AllowSpontaneous, nullptr, nullptr};
    static constexpr WGPUUncapturedErrorCallbackInfo kDefaultUncapturedErrorCallbackInfo =
        kEmptyUncapturedErrorCallbackInfo;
#endif  // DAWN_ENABLE_ASSERTS

    WGPUDeviceLostCallbackInfo deviceLostCallbackInfo = kDefaultDeviceLostCallbackInfo;
    if (descriptor != nullptr) {
        if (descriptor->deviceLostCallbackInfo.callback != nullptr) {
            deviceLostCallbackInfo = descriptor->deviceLostCallbackInfo;
            if (deviceLostCallbackInfo.mode == WGPUCallbackMode_WaitAnyOnly) {
                // TODO(dawn:2458) Currently we default the callback mode to Spontaneous if not
                // passed for backwards compatibility. We should add warning logging for it though
                // when available. Update this when we have WGPUCallbackMode_Undefined.
                deviceLostCallbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
            }
            mDeviceLostInfo.callback = deviceLostCallbackInfo.callback;
            mDeviceLostInfo.userdata = deviceLostCallbackInfo.userdata;
        } else if (descriptor->deviceLostCallback != nullptr) {
            deviceLostCallbackInfo = {nullptr, WGPUCallbackMode_AllowSpontaneous, nullptr, nullptr};
            mDeviceLostInfo.oldCallback = descriptor->deviceLostCallback;
            mDeviceLostInfo.userdata = descriptor->deviceLostUserdata;
        }
    }
    mDeviceLostInfo.event = std::make_unique<DeviceLostEvent>(deviceLostCallbackInfo, this);

    mUncapturedErrorCallbackInfo = kDefaultUncapturedErrorCallbackInfo;
    if (descriptor && descriptor->uncapturedErrorCallbackInfo.callback != nullptr) {
        mUncapturedErrorCallbackInfo = descriptor->uncapturedErrorCallbackInfo;
    }
}

Device::~Device() {
    if (mQueue != nullptr) {
        mQueue.ExtractAsDangling()->Release();
    }
}

ObjectType Device::GetObjectType() const {
    return ObjectType::Device;
}

uint32_t Device::Release() {
    // The device always has a reference in it's DeviceLossEvent which is created at construction,
    // so when we drop to 1, we want to set the event so that the device can be loss according to
    // the callback mode.
    uint32_t refCount = ObjectBase::Release();
    if (refCount == 1) {
        HandleDeviceLost(WGPUDeviceLostReason_Destroyed, "Device was destroyed.");
    }
    return refCount;
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
    if (mUncapturedErrorCallbackInfo.callback) {
        mUncapturedErrorCallbackInfo.callback(errorType, message,
                                              mUncapturedErrorCallbackInfo.userdata);
    }
}

void Device::HandleLogging(WGPULoggingType loggingType, const char* message) {
    if (mLoggingCallback) {
        // Since client always run in single thread, calling the callback directly is safe.
        mLoggingCallback(loggingType, message, mLoggingUserdata);
    }
}

void Device::HandleDeviceLost(WGPUDeviceLostReason reason, const char* message) {
    FutureID futureID = GetDeviceLostFuture().id;
    if (futureID != kNullFutureID) {
        DAWN_CHECK(GetEventManager().SetFutureReady<DeviceLostEvent>(futureID, reason, message) ==
                   WireResult::Success);
    }
}

WGPUFuture Device::GetDeviceLostFuture() {
    // Lazily track the device lost event so that event ordering w.r.t RequestDevice is correct.
    if (mDeviceLostInfo.event != nullptr) {
        auto [deviceLostFutureIDInternal, tracked] =
            GetEventManager().TrackEvent(std::move(mDeviceLostInfo.event));
        if (tracked) {
            mDeviceLostInfo.futureID = deviceLostFutureIDInternal;
        }
    }
    return {mDeviceLostInfo.futureID};
}

std::weak_ptr<bool> Device::GetAliveWeakPtr() {
    return mIsAlive;
}

void Device::SetUncapturedErrorCallback(WGPUErrorCallback errorCallback, void* errorUserdata) {
    if (mDeviceLostInfo.futureID != kNullFutureID) {
        mUncapturedErrorCallbackInfo = {nullptr, errorCallback, errorUserdata};
    }
}

void Device::SetLoggingCallback(WGPULoggingCallback callback, void* userdata) {
    mLoggingCallback = callback;
    mLoggingUserdata = userdata;
}

void Device::SetDeviceLostCallback(WGPUDeviceLostCallback callback, void* userdata) {
    if (mDeviceLostInfo.futureID != kNullFutureID) {
        mDeviceLostInfo.oldCallback = callback;
        mDeviceLostInfo.userdata = userdata;
    }
}

WireResult Client::DoDeviceLostCallback(ObjectHandle eventManager,
                                        WGPUFuture future,
                                        WGPUDeviceLostReason reason,
                                        char const* message) {
    return GetEventManager(eventManager)
        .SetFutureReady<Device::DeviceLostEvent>(future.id, reason, message);
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

    mQueue->AddRef();
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
