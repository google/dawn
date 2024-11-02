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
#include "dawn/common/StringViewUtils.h"
#include "dawn/wire/client/ApiObjects_autogen.h"
#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/EventManager.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire::client {
namespace {

class PopErrorScopeEvent final : public TrackedEvent {
  public:
    static constexpr EventType kType = EventType::PopErrorScope;

    explicit PopErrorScopeEvent(const WGPUPopErrorScopeCallbackInfo2& callbackInfo)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2) {}

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID, WGPUErrorType errorType, WGPUStringView message) {
        mType = errorType;
        mMessage = ToString(message);
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPUPopErrorScopeStatus_InstanceDropped;
            mMessage = "";
        }
        if (mCallback) {
            mCallback(mStatus, mType, ToOutputStringView(mMessage), mUserdata1.ExtractAsDangling(),
                      mUserdata2.ExtractAsDangling());
        }
    }

    WGPUPopErrorScopeCallback2 mCallback;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;

    WGPUPopErrorScopeStatus mStatus = WGPUPopErrorScopeStatus_Success;
    WGPUErrorType mType = WGPUErrorType_Unknown;
    std::string mMessage;
};

template <typename PipelineT, EventType Type, typename CallbackInfoT>
class CreatePipelineEventBase : public TrackedEvent {
  public:
    // Export these types upwards for ease of use.
    using Pipeline = PipelineT;
    using CallbackInfo = CallbackInfoT;

    static constexpr EventType kType = Type;

    CreatePipelineEventBase(const CallbackInfo& callbackInfo, Ref<Pipeline> pipeline)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2),
          mPipeline(std::move(pipeline)) {
        DAWN_ASSERT(mPipeline != nullptr);
    }

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID,
                         WGPUCreatePipelineAsyncStatus status,
                         WGPUStringView message) {
        DAWN_ASSERT(mPipeline != nullptr);
        mStatus = status;
        mMessage = ToString(message);
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        auto userdata1 = mUserdata1.ExtractAsDangling();
        auto userdata2 = mUserdata2.ExtractAsDangling();

        if (mCallback == nullptr) {
            return;
        }

        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPUCreatePipelineAsyncStatus_InstanceDropped;
            mMessage = "A valid external Instance reference no longer exists.";
        }

        mCallback(mStatus,
                  mStatus == WGPUCreatePipelineAsyncStatus_Success
                      ? ReturnToAPI(std::move(mPipeline))
                      : nullptr,
                  ToOutputStringView(mMessage), userdata1, userdata2);
    }

    using Callback = decltype(std::declval<CallbackInfo>().callback);
    Callback mCallback;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;

    // Note that the message is optional because we want to return nullptr when it wasn't set
    // instead of a pointer to an empty string.
    WGPUCreatePipelineAsyncStatus mStatus = WGPUCreatePipelineAsyncStatus_Success;
    std::string mMessage;

    Ref<Pipeline> mPipeline;
};

using CreateComputePipelineEvent =
    CreatePipelineEventBase<ComputePipeline,
                            EventType::CreateComputePipeline,
                            WGPUCreateComputePipelineAsyncCallbackInfo2>;
using CreateRenderPipelineEvent =
    CreatePipelineEventBase<RenderPipeline,
                            EventType::CreateRenderPipeline,
                            WGPUCreateRenderPipelineAsyncCallbackInfo2>;

void LegacyDeviceLostCallback(WGPUDevice const*,
                              WGPUDeviceLostReason reason,
                              WGPUStringView message,
                              void* callback,
                              void* userdata) {
    if (callback == nullptr) {
        return;
    }
    auto cb = reinterpret_cast<WGPUDeviceLostCallback>(callback);
    cb(reason, message, userdata);
}

void LegacyDeviceLostCallback2(WGPUDevice const* device,
                               WGPUDeviceLostReason reason,
                               WGPUStringView message,
                               void* callback,
                               void* userdata) {
    if (callback == nullptr) {
        return;
    }
    auto cb = reinterpret_cast<WGPUDeviceLostCallbackNew>(callback);
    cb(device, reason, message, userdata);
}

void LegacyUncapturedErrorCallback(WGPUDevice const*,
                                   WGPUErrorType type,
                                   WGPUStringView message,
                                   void* callback,
                                   void* userdata) {
    if (callback == nullptr) {
        return;
    }
    auto cb = reinterpret_cast<WGPUErrorCallback>(callback);
    cb(type, message, userdata);
}

static constexpr WGPUUncapturedErrorCallbackInfo2 kEmptyUncapturedErrorCallbackInfo = {
    nullptr, nullptr, nullptr, nullptr};

}  // namespace

class Device::DeviceLostEvent : public TrackedEvent {
  public:
    static constexpr EventType kType = EventType::DeviceLost;

    DeviceLostEvent(const WGPUDeviceLostCallbackInfo2& callbackInfo, Ref<Device> device)
        : TrackedEvent(callbackInfo.mode), mDevice(std::move(device)) {
        DAWN_ASSERT(mDevice != nullptr);

        mDevice->mDeviceLostInfo.callback = callbackInfo.callback;
        mDevice->mDeviceLostInfo.userdata1 = callbackInfo.userdata1;
        mDevice->mDeviceLostInfo.userdata2 = callbackInfo.userdata2;
    }

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID, WGPUDeviceLostReason reason, WGPUStringView message) {
        mReason = reason;
        mMessage = ToString(message);
        mDevice->mDeviceLostInfo.futureID = kNullFutureID;
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        if (completionType == EventCompletionType::Shutdown) {
            mReason = WGPUDeviceLostReason_InstanceDropped;
            mMessage = "A valid external Instance reference no longer exists.";
        }

        void* userdata1 = mDevice->mDeviceLostInfo.userdata1.ExtractAsDangling();
        void* userdata2 = mDevice->mDeviceLostInfo.userdata2.ExtractAsDangling();

        if (mDevice->mDeviceLostInfo.callback != nullptr) {
            const auto device =
                mReason != WGPUDeviceLostReason_FailedCreation ? ToAPI(mDevice.Get()) : nullptr;
            mDevice->mDeviceLostInfo.callback(&device, mReason, ToOutputStringView(mMessage),
                                              userdata1, userdata2);
        }
        mDevice->mUncapturedErrorCallbackInfo = kEmptyUncapturedErrorCallbackInfo;
    }

    WGPUDeviceLostReason mReason;
    // Note that the message is optional because we want to return nullptr when it wasn't set
    // instead of a pointer to an empty string.
    std::string mMessage;

    // Strong reference to the device so that when we call the callback we can pass the device.
    Ref<Device> mDevice;
};

Device::Device(const ObjectBaseParams& params,
               const ObjectHandle& eventManagerHandle,
               Adapter* adapter,
               const WGPUDeviceDescriptor* descriptor)
    : RefCountedWithExternalCount<ObjectWithEventsBase>(params, eventManagerHandle),
      mAdapter(adapter) {
#if defined(DAWN_ENABLE_ASSERTS)
    static constexpr WGPUDeviceLostCallbackInfo2 kDefaultDeviceLostCallbackInfo = {
        nullptr, WGPUCallbackMode_AllowSpontaneous,
        [](WGPUDevice const*, WGPUDeviceLostReason, WGPUStringView, void*, void*) {
            static bool calledOnce = false;
            if (!calledOnce) {
                calledOnce = true;
                dawn::WarningLog() << "No Dawn device lost callback was set. This is probably not "
                                      "intended. If you really want to ignore device lost "
                                      "and suppress this message, set the callback explicitly.";
            }
        },
        nullptr, nullptr};
    static constexpr WGPUUncapturedErrorCallbackInfo2 kDefaultUncapturedErrorCallbackInfo = {
        nullptr,
        [](WGPUDevice const*, WGPUErrorType, WGPUStringView, void*, void*) {
            static bool calledOnce = false;
            if (!calledOnce) {
                calledOnce = true;
                dawn::WarningLog() << "No Dawn device uncaptured error callback was set. This is "
                                      "probably not intended. If you really want to ignore errors "
                                      "and suppress this message, set the callback explicitly.";
            }
        },
        nullptr, nullptr};
#else
    static constexpr WGPUDeviceLostCallbackInfo2 kDefaultDeviceLostCallbackInfo = {
        nullptr, WGPUCallbackMode_AllowSpontaneous, nullptr, nullptr, nullptr};
    static constexpr WGPUUncapturedErrorCallbackInfo2 kDefaultUncapturedErrorCallbackInfo =
        kEmptyUncapturedErrorCallbackInfo;
#endif  // DAWN_ENABLE_ASSERTS

    WGPUDeviceLostCallbackInfo2 deviceLostCallbackInfo = kDefaultDeviceLostCallbackInfo;
    if (descriptor != nullptr) {
        if (descriptor->deviceLostCallbackInfo2.callback != nullptr) {
            deviceLostCallbackInfo = descriptor->deviceLostCallbackInfo2;
        } else if (descriptor->deviceLostCallbackInfo.callback != nullptr) {
            auto& callbackInfo = descriptor->deviceLostCallbackInfo;
            deviceLostCallbackInfo = {
                callbackInfo.nextInChain, callbackInfo.mode, &LegacyDeviceLostCallback2,
                reinterpret_cast<void*>(callbackInfo.callback), callbackInfo.userdata};
        } else if (descriptor->deviceLostCallback != nullptr) {
            deviceLostCallbackInfo = {nullptr, WGPUCallbackMode_AllowSpontaneous,
                                      &LegacyDeviceLostCallback,
                                      reinterpret_cast<void*>(descriptor->deviceLostCallback),
                                      descriptor->deviceLostUserdata};
        }
    }
    mDeviceLostInfo.event = std::make_unique<DeviceLostEvent>(deviceLostCallbackInfo, this);

    mUncapturedErrorCallbackInfo = kDefaultUncapturedErrorCallbackInfo;
    if (descriptor != nullptr) {
        if (descriptor->uncapturedErrorCallbackInfo2.callback != nullptr) {
            mUncapturedErrorCallbackInfo = descriptor->uncapturedErrorCallbackInfo2;
        } else if (descriptor->uncapturedErrorCallbackInfo.callback != nullptr) {
            auto& callbackInfo = descriptor->uncapturedErrorCallbackInfo;
            mUncapturedErrorCallbackInfo = {
                callbackInfo.nextInChain, &LegacyUncapturedErrorCallback,
                reinterpret_cast<void*>(callbackInfo.callback), callbackInfo.userdata};
        }
    }
}

ObjectType Device::GetObjectType() const {
    return ObjectType::Device;
}

bool Device::IsAlive() const {
    return mIsAlive;
}

void Device::WillDropLastExternalRef() {
    if (IsRegistered()) {
        HandleDeviceLost(WGPUDeviceLostReason_Destroyed,
                         ToOutputStringView("Device was destroyed."));
    }
    Unregister();
}

WGPUStatus Device::GetLimits(WGPUSupportedLimits* limits) const {
    return mLimitsAndFeatures.GetLimits(limits);
}

bool Device::HasFeature(WGPUFeatureName feature) const {
    return mLimitsAndFeatures.HasFeature(feature);
}

size_t Device::EnumerateFeatures(WGPUFeatureName* features) const {
    return mLimitsAndFeatures.EnumerateFeatures(features);
}

void Device::GetFeatures(WGPUSupportedFeatures* features) const {
    mLimitsAndFeatures.ToSupportedFeatures(features);
}

void Device::SetLimits(const WGPUSupportedLimits* limits) {
    return mLimitsAndFeatures.SetLimits(limits);
}

void Device::SetFeatures(const WGPUFeatureName* features, uint32_t featuresCount) {
    return mLimitsAndFeatures.SetFeatures(features, featuresCount);
}

void Device::HandleError(WGPUErrorType errorType, WGPUStringView message) {
    if (mUncapturedErrorCallbackInfo.callback) {
        const auto device = ToAPI(this);
        mUncapturedErrorCallbackInfo.callback(&device, errorType, message,
                                              mUncapturedErrorCallbackInfo.userdata1,
                                              mUncapturedErrorCallbackInfo.userdata2);
    }
}

void Device::HandleLogging(WGPULoggingType loggingType, WGPUStringView message) {
    if (mLoggingCallback) {
        // Since client always run in single thread, calling the callback directly is safe.
        mLoggingCallback(loggingType, message, mLoggingUserdata);
    }
}

void Device::HandleDeviceLost(WGPUDeviceLostReason reason, WGPUStringView message) {
    FutureID futureID = GetDeviceLostFuture().id;
    if (futureID != kNullFutureID) {
        DAWN_CHECK(GetEventManager().SetFutureReady<DeviceLostEvent>(futureID, reason, message) ==
                   WireResult::Success);
    }
    mIsAlive = false;
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

void Device::SetUncapturedErrorCallback(WGPUErrorCallback errorCallback, void* errorUserdata) {
    if (mDeviceLostInfo.futureID != kNullFutureID) {
        mUncapturedErrorCallbackInfo = {nullptr, &LegacyUncapturedErrorCallback,
                                        reinterpret_cast<void*>(errorCallback), errorUserdata};
    }
}

void Device::SetLoggingCallback(WGPULoggingCallback callback, void* userdata) {
    mLoggingCallback = callback;
    mLoggingUserdata = userdata;
}

void Device::SetDeviceLostCallback(WGPUDeviceLostCallback callback, void* userdata) {
    if (mDeviceLostInfo.futureID != kNullFutureID) {
        mDeviceLostInfo.callback = &LegacyDeviceLostCallback;
        mDeviceLostInfo.userdata1 = reinterpret_cast<void*>(callback);
        mDeviceLostInfo.userdata2 = userdata;
    }
}

WireResult Client::DoDeviceLostCallback(ObjectHandle eventManager,
                                        WGPUFuture future,
                                        WGPUDeviceLostReason reason,
                                        WGPUStringView message) {
    return GetEventManager(eventManager)
        .SetFutureReady<Device::DeviceLostEvent>(future.id, reason, message);
}

void Device::PopErrorScope(WGPUErrorCallback callback, void* userdata) {
    static WGPUErrorCallback kDefaultCallback = [](WGPUErrorType, WGPUStringView, void*) {};

    PopErrorScope2({nullptr, WGPUCallbackMode_AllowSpontaneous,
                    [](WGPUPopErrorScopeStatus, WGPUErrorType type, WGPUStringView message,
                       void* callback, void* userdata) {
                        auto cb = reinterpret_cast<WGPUErrorCallback>(callback);
                        cb(type, message, userdata);
                    },
                    reinterpret_cast<void*>(callback != nullptr ? callback : kDefaultCallback),
                    userdata});
}

WGPUFuture Device::PopErrorScopeF(const WGPUPopErrorScopeCallbackInfo& callbackInfo) {
    return PopErrorScope2({callbackInfo.nextInChain, callbackInfo.mode,
                           [](WGPUPopErrorScopeStatus status, WGPUErrorType type,
                              WGPUStringView message, void* callback, void* userdata) {
                               auto cb = reinterpret_cast<WGPUPopErrorScopeCallback>(callback);
                               cb(status, type, message, userdata);
                           },
                           reinterpret_cast<void*>(callbackInfo.callback), callbackInfo.userdata});
}

WGPUFuture Device::PopErrorScope2(const WGPUPopErrorScopeCallbackInfo2& callbackInfo) {
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
                                                 WGPUStringView message) {
    return GetEventManager(eventManager)
        .SetFutureReady<PopErrorScopeEvent>(future.id, errorType, message);
}

void Device::InjectError(WGPUErrorType type, WGPUStringView message) {
    DeviceInjectErrorCmd cmd;
    cmd.self = ToAPI(this);
    cmd.type = type;
    cmd.message = message;
    GetClient()->SerializeCommand(cmd);
}

WGPUBuffer Device::CreateBuffer(const WGPUBufferDescriptor* descriptor) {
    return Buffer::Create(this, descriptor);
}

WGPUBuffer Device::CreateErrorBuffer(const WGPUBufferDescriptor* descriptor) {
    return Buffer::CreateError(this, descriptor);
}

WGPUAdapter Device::GetAdapter() const {
    Ref<Adapter> adapter = mAdapter;
    return ReturnToAPI(std::move(adapter));
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

    Ref<Queue> queue = mQueue;
    return ReturnToAPI(std::move(queue));
}

template <typename Event, typename Cmd, typename CallbackInfo, typename Descriptor>
WGPUFuture Device::CreatePipelineAsyncF(Descriptor const* descriptor,
                                        const CallbackInfo& callbackInfo) {
    using Pipeline = typename Event::Pipeline;

    Client* client = GetClient();
    Ref<Pipeline> pipeline = client->Make<Pipeline>();
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
    CreateComputePipelineAsync2(
        descriptor, {nullptr, WGPUCallbackMode_AllowSpontaneous,
                     [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline pipeline,
                        WGPUStringView message, void* callback, void* userdata) {
                         auto cb =
                             reinterpret_cast<WGPUCreateComputePipelineAsyncCallback>(callback);
                         cb(status, pipeline, message, userdata);
                     },
                     reinterpret_cast<void*>(callback), userdata});
}

WGPUFuture Device::CreateComputePipelineAsyncF(
    WGPUComputePipelineDescriptor const* descriptor,
    const WGPUCreateComputePipelineAsyncCallbackInfo& callbackInfo) {
    return CreateComputePipelineAsync2(
        descriptor, {callbackInfo.nextInChain, callbackInfo.mode,
                     [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline pipeline,
                        WGPUStringView message, void* callback, void* userdata) {
                         auto cb =
                             reinterpret_cast<WGPUCreateComputePipelineAsyncCallback>(callback);
                         cb(status, pipeline, message, userdata);
                     },
                     reinterpret_cast<void*>(callbackInfo.callback), callbackInfo.userdata});
}

WGPUFuture Device::CreateComputePipelineAsync2(
    WGPUComputePipelineDescriptor const* descriptor,
    const WGPUCreateComputePipelineAsyncCallbackInfo2& callbackInfo) {
    return CreatePipelineAsyncF<CreateComputePipelineEvent, DeviceCreateComputePipelineAsyncCmd>(
        descriptor, callbackInfo);
}

WireResult Client::DoDeviceCreateComputePipelineAsyncCallback(ObjectHandle eventManager,
                                                              WGPUFuture future,
                                                              WGPUCreatePipelineAsyncStatus status,
                                                              WGPUStringView message) {
    return GetEventManager(eventManager)
        .SetFutureReady<CreateComputePipelineEvent>(future.id, status, message);
}

void Device::CreateRenderPipelineAsync(WGPURenderPipelineDescriptor const* descriptor,
                                       WGPUCreateRenderPipelineAsyncCallback callback,
                                       void* userdata) {
    CreateRenderPipelineAsync2(
        descriptor, {nullptr, WGPUCallbackMode_AllowSpontaneous,
                     [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline pipeline,
                        WGPUStringView message, void* callback, void* userdata) {
                         auto cb =
                             reinterpret_cast<WGPUCreateRenderPipelineAsyncCallback>(callback);
                         cb(status, pipeline, message, userdata);
                     },
                     reinterpret_cast<void*>(callback), userdata});
}

WGPUFuture Device::CreateRenderPipelineAsyncF(
    WGPURenderPipelineDescriptor const* descriptor,
    const WGPUCreateRenderPipelineAsyncCallbackInfo& callbackInfo) {
    return CreateRenderPipelineAsync2(
        descriptor, {callbackInfo.nextInChain, callbackInfo.mode,
                     [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline pipeline,
                        WGPUStringView message, void* callback, void* userdata) {
                         auto cb =
                             reinterpret_cast<WGPUCreateRenderPipelineAsyncCallback>(callback);
                         cb(status, pipeline, message, userdata);
                     },
                     reinterpret_cast<void*>(callbackInfo.callback), callbackInfo.userdata});
}

WGPUFuture Device::CreateRenderPipelineAsync2(
    WGPURenderPipelineDescriptor const* descriptor,
    const WGPUCreateRenderPipelineAsyncCallbackInfo2& callbackInfo) {
    return CreatePipelineAsyncF<CreateRenderPipelineEvent, DeviceCreateRenderPipelineAsyncCmd>(
        descriptor, callbackInfo);
}

WireResult Client::DoDeviceCreateRenderPipelineAsyncCallback(ObjectHandle eventManager,
                                                             WGPUFuture future,
                                                             WGPUCreatePipelineAsyncStatus status,
                                                             WGPUStringView message) {
    return GetEventManager(eventManager)
        .SetFutureReady<CreateRenderPipelineEvent>(future.id, status, message);
}

void Device::Destroy() {
    DeviceDestroyCmd cmd;
    cmd.self = ToAPI(this);
    GetClient()->SerializeCommand(cmd);

    mIsAlive = false;
}

}  // namespace dawn::wire::client
