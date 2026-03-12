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
#include <mutex>
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

    explicit PopErrorScopeEvent(const WGPUPopErrorScopeCallbackInfo& callbackInfo)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2) {}

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID,
                         WGPUPopErrorScopeStatus status,
                         WGPUErrorType errorType,
                         WGPUStringView message) {
        mStatus = status;
        mType = errorType;
        mMessage = ToString(message);
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPUPopErrorScopeStatus_CallbackCancelled;
            mMessage = "";
        }
        if (mCallback) {
            mCallback(mStatus, mType, ToOutputStringView(mMessage), mUserdata1.ExtractAsDangling(),
                      mUserdata2.ExtractAsDangling());
        }
    }

    WGPUPopErrorScopeCallback mCallback;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;

    WGPUPopErrorScopeStatus mStatus = {};
    WGPUErrorType mType = WGPUErrorType_NoError;
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
            mStatus = WGPUCreatePipelineAsyncStatus_CallbackCancelled;
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

    WGPUCreatePipelineAsyncStatus mStatus = WGPUCreatePipelineAsyncStatus_Success;
    std::string mMessage;

    Ref<Pipeline> mPipeline;
};

using CreateComputePipelineEvent =
    CreatePipelineEventBase<ComputePipeline,
                            EventType::CreateComputePipeline,
                            WGPUCreateComputePipelineAsyncCallbackInfo>;
using CreateRenderPipelineEvent =
    CreatePipelineEventBase<RenderPipeline,
                            EventType::CreateRenderPipeline,
                            WGPUCreateRenderPipelineAsyncCallbackInfo>;

// Default callback infos depending on the build type.
#ifdef DAWN_ENABLE_ASSERTS
static constexpr WGPUDeviceLostCallbackInfo kDefaultDeviceLostCallbackInfo = {
    nullptr, WGPUCallbackMode_AllowSpontaneous,
    [](WGPUDevice const*, WGPUDeviceLostReason, WGPUStringView, void*, void*) {
        static std::once_flag flag;
        std::call_once(flag, []() {
            dawn::WarningLog() << "No Dawn device lost callback was set. This is probably not "
                                  "intended. If you really want to ignore device lost "
                                  "and suppress this message, set the callback explicitly.";
        });
    },
    nullptr, nullptr};
static constexpr WGPUUncapturedErrorCallbackInfo kDefaultUncapturedErrorCallbackInfo = {
    nullptr,
    [](WGPUDevice const*, WGPUErrorType, WGPUStringView, void*, void*) {
        static std::once_flag flag;
        std::call_once(flag, []() {
            dawn::WarningLog() << "No Dawn device uncaptured error callback was set. This is "
                                  "probably not intended. If you really want to ignore errors "
                                  "and suppress this message, set the callback explicitly.";
        });
    },
    nullptr, nullptr};
static constexpr WGPULoggingCallbackInfo kDefaultLoggingCallbackInfo = {
    nullptr,
    [](WGPULoggingType, WGPUStringView, void*, void*) {
        static std::once_flag flag;
        std::call_once(flag, []() {
            dawn::WarningLog() << "No Dawn device logging callback callback was set. This is "
                                  "probably not intended. If you really want to ignore logs "
                                  "and suppress this message, set the callback explicitly.";
        });
    },
    nullptr, nullptr};
#else
static constexpr WGPUDeviceLostCallbackInfo kDefaultDeviceLostCallbackInfo = {
    nullptr, WGPUCallbackMode_AllowSpontaneous, nullptr, nullptr, nullptr};
static constexpr WGPUUncapturedErrorCallbackInfo kDefaultUncapturedErrorCallbackInfo = {
    nullptr, nullptr, nullptr, nullptr};
static constexpr WGPULoggingCallbackInfo kDefaultLoggingCallbackInfo = {nullptr, nullptr, nullptr,
                                                                        nullptr};
#endif  // DAWN_ENABLE_ASSERTS

const WGPUDeviceLostCallbackInfo& GetDeviceLostCallbackInfo(
    const WGPUDeviceDescriptor* descriptor) {
    if (descriptor != nullptr && descriptor->deviceLostCallbackInfo.callback != nullptr) {
        return descriptor->deviceLostCallbackInfo;
    }
    return kDefaultDeviceLostCallbackInfo;
}

const WGPUUncapturedErrorCallbackInfo& GetUncapturedErrorCallbackInfo(
    const WGPUDeviceDescriptor* descriptor) {
    if (descriptor != nullptr && descriptor->uncapturedErrorCallbackInfo.callback != nullptr) {
        return descriptor->uncapturedErrorCallbackInfo;
    }
    return kDefaultUncapturedErrorCallbackInfo;
}

}  // namespace

Device::CallbackInfos::CallbackInfos(const WGPUUncapturedErrorCallbackInfo& error,
                                     const WGPULoggingCallbackInfo& logging) {
    if (error.callback != nullptr) {
        this->error = error;
    }
    if (logging.callback != nullptr) {
        this->logging = logging;
    }
}

class Device::DeviceLostEvent : public TrackedEvent {
  public:
    static constexpr EventType kType = EventType::DeviceLost;

    DeviceLostEvent(const WGPUDeviceLostCallbackInfo& callbackInfo, Ref<Device> device)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2),
          mDevice(std::move(device)) {
        DAWN_ASSERT(mDevice != nullptr);
    }

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID, WGPUDeviceLostReason reason, WGPUStringView message) {
        if (mMessage.empty()) {
            mReason = reason;
            mMessage = ToString(message);
        }
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        if (completionType == EventCompletionType::Shutdown) {
            mReason = WGPUDeviceLostReason_CallbackCancelled;
            mMessage = "A valid external Instance reference no longer exists.";
        }

        mDevice->mCallbackInfos.Use<NotifyType::None>([](auto callbackInfos) {
            callbackInfos->error = std::nullopt;
            callbackInfos->logging = std::nullopt;

            // The uncaptured error and logging callbacks are spontaneous and must not be called
            // after we call the device lost's |mCallback| below. Although we have cleared those
            // callbacks, we need to wait for any remaining outstanding callbacks to finish before
            // continuing.
            callbackInfos.Wait([](auto& x) { return x.semaphore == 0; });
        });

        void* userdata1 = mUserdata1.ExtractAsDangling();
        void* userdata2 = mUserdata2.ExtractAsDangling();

        if (mCallback != nullptr) {
            const auto device =
                mReason != WGPUDeviceLostReason_FailedCreation ? ToAPI(mDevice.Get()) : nullptr;
            mCallback(&device, mReason, ToOutputStringView(mMessage), userdata1, userdata2);
        }
    }

    WGPUDeviceLostCallback mCallback = nullptr;
    raw_ptr<void> mUserdata1 = nullptr;
    raw_ptr<void> mUserdata2 = nullptr;

    WGPUDeviceLostReason mReason;
    std::string mMessage;

    // Strong reference to the device so that when we call the callback we can pass the device.
    Ref<Device> mDevice;
};

Device::Device(const ObjectBaseParams& params,
               const ObjectHandle& eventManagerHandle,
               Adapter* adapter,
               const WGPUDeviceDescriptor* descriptor)
    : RefCountedWithExternalCount<ObjectWithEventsBase>(params, eventManagerHandle),
      mDeviceLostInfo(AcquireRef(new DeviceLostEvent(GetDeviceLostCallbackInfo(descriptor), this))),
      mCallbackInfos(GetUncapturedErrorCallbackInfo(descriptor), kDefaultLoggingCallbackInfo),
      mAdapter(adapter) {}

ObjectType Device::GetObjectType() const {
    return ObjectType::Device;
}

bool Device::IsAlive() const {
    return mIsAlive;
}

Queue* Device::GetQueue() {
    // The queue is lazily created because if a Device is created by Reserve/Inject, we cannot send
    // the GetQueue message until it has been injected on the Server. It cannot happen immediately
    // on construction.
    if (mQueue == nullptr) {
        // Get the primary queue for this device.
        Client* client = GetClient();
        mQueue = client->Make<Queue>(GetEventManagerHandle());

        DeviceGetQueueCmd cmd;
        cmd.self = ToAPI(this);
        cmd.result = mQueue->GetWireHandle(client);
        client->SerializeCommand(cmd);
    }
    return mQueue.Get();
}

const LimitsAndFeatures& Device::GetLimitsAndFeatures() const {
    return mLimitsAndFeatures;
}

void Device::WillDropLastExternalRef() {
    if (IsRegistered()) {
        HandleDeviceLost(WGPUDeviceLostReason_Destroyed,
                         ToOutputStringView("Device was destroyed."));
    }
    Unregister();
}

WGPUStatus Device::APIGetLimits(WGPULimits* limits) const {
    return mLimitsAndFeatures.GetLimits(limits);
}

bool Device::APIHasFeature(WGPUFeatureName feature) const {
    return mLimitsAndFeatures.HasFeature(feature);
}

void Device::APIGetFeatures(WGPUSupportedFeatures* features) const {
    mLimitsAndFeatures.ToSupportedFeatures(features);
}

WGPUStatus Device::APIGetAdapterInfo(WGPUAdapterInfo* adapterInfo) const {
    return mAdapter->APIGetInfo(adapterInfo);
}

void Device::SetLimits(const WGPULimits* limits) {
    mLimitsAndFeatures.SetLimits(limits);
}

void Device::SetFeatures(const WGPUFeatureName* features, uint32_t featuresCount) {
    mLimitsAndFeatures.SetFeatures(features, featuresCount);
}

void Device::HandleError(WGPUErrorType errorType, WGPUStringView message) {
    std::optional<WGPUUncapturedErrorCallbackInfo> callbackInfo;
    mCallbackInfos.Use<NotifyType::None>([&](auto callbackInfos) {
        callbackInfo = callbackInfos->error;
        if (callbackInfo) {
            callbackInfos->semaphore += 1;
        }
    });

    // If we don't have a callback info, we can just return.
    if (!callbackInfo) {
        return;
    }

    // Call the callback without holding the lock to prevent any re-entrant issues.
    DAWN_ASSERT(callbackInfo->callback != nullptr);
    const auto device = ToAPI(this);
    callbackInfo->callback(&device, errorType, message, callbackInfo->userdata1,
                           callbackInfo->userdata2);

    mCallbackInfos.Use([&](auto callbackInfos) {
        DAWN_ASSERT(callbackInfos->semaphore > 0);
        callbackInfos->semaphore -= 1;
    });
}

void Device::HandleLogging(WGPULoggingType loggingType, WGPUStringView message) {
    std::optional<WGPULoggingCallbackInfo> callbackInfo;
    mCallbackInfos.Use<NotifyType::None>([&](auto callbackInfos) {
        callbackInfo = callbackInfos->logging;
        if (callbackInfo) {
            callbackInfos->semaphore += 1;
        }
    });

    // If we don't have a callback info, we can just return.
    if (!callbackInfo) {
        return;
    }

    // Call the callback without holding the lock to prevent any re-entrant issues.
    DAWN_ASSERT(callbackInfo->callback != nullptr);
    callbackInfo->callback(loggingType, message, callbackInfo->userdata1, callbackInfo->userdata2);

    mCallbackInfos.Use([&](auto callbackInfos) {
        DAWN_ASSERT(callbackInfos->semaphore > 0);
        callbackInfos->semaphore -= 1;
    });
}

void Device::HandleDeviceLost(WGPUDeviceLostReason reason, WGPUStringView message) {
    FutureID futureID = APIGetLostFuture().id;
    DAWN_CHECK(GetEventManager().SetFutureReady<DeviceLostEvent>(futureID, reason, message) ==
               WireResult::Success);
    mIsAlive = false;
}

WGPUFuture Device::APIGetLostFuture() {
    // Lazily track the device lost event so that event ordering w.r.t RequestDevice is correct.
    if (const auto* e = std::get_if<Ref<TrackedEvent>>(&mDeviceLostInfo)) {
        Ref<TrackedEvent> event = *e;
        auto [futureID, _] = GetEventManager().TrackEvent(std::move(event));
        mDeviceLostInfo = futureID;
    }
    return {std::get<FutureID>(mDeviceLostInfo)};
}

void Device::APISetLoggingCallback(const WGPULoggingCallbackInfo& callbackInfo) {
    if (mIsAlive) {
        mCallbackInfos.Use<NotifyType::None>(
            [&](auto callbackInfos) { callbackInfos->logging = callbackInfo; });
    }
}

WireResult Client::DoDeviceLostCallback(ObjectHandle eventManager,
                                        WGPUFuture future,
                                        WGPUDeviceLostReason reason,
                                        WGPUStringView message) {
    return SetFutureReady<Device::DeviceLostEvent>(eventManager, future.id, reason, message);
}

WGPUFuture Device::APIPopErrorScope(const WGPUPopErrorScopeCallbackInfo& callbackInfo) {
    Client* client = GetClient();
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(AcquireRef(new PopErrorScopeEvent(callbackInfo)));
    if (!tracked) {
        return {futureIDInternal};
    }

    DevicePopErrorScopeCmd cmd;
    cmd.deviceId = GetWireHandle(client).id;
    cmd.eventManagerHandle = GetEventManagerHandle();
    cmd.future = {futureIDInternal};
    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

WireResult Client::DoDevicePopErrorScopeCallback(ObjectHandle eventManager,
                                                 WGPUFuture future,
                                                 WGPUPopErrorScopeStatus status,
                                                 WGPUErrorType errorType,
                                                 WGPUStringView message) {
    return SetFutureReady<PopErrorScopeEvent>(eventManager, future.id, status, errorType, message);
}

void Device::APIInjectError(WGPUErrorType type, WGPUStringView message) {
    DeviceInjectErrorCmd cmd;
    cmd.self = ToAPI(this);
    cmd.type = type;
    cmd.message = message;
    GetClient()->SerializeCommand(cmd);
}

WGPUBuffer Device::APICreateBuffer(const WGPUBufferDescriptor* descriptor) {
    return Buffer::Create(this, descriptor);
}

WGPUBuffer Device::APICreateErrorBuffer(const WGPUBufferDescriptor* descriptor) {
    return Buffer::CreateError(this, descriptor);
}

WGPUResourceTable Device::APICreateResourceTable(const WGPUResourceTableDescriptor* descriptor) {
    return ResourceTable::Create(this, descriptor);
}

WGPUTexture Device::APICreateTexture(const WGPUTextureDescriptor* descriptor) {
    return Texture::Create(this, descriptor);
}

WGPUTexture Device::APICreateErrorTexture(const WGPUTextureDescriptor* descriptor) {
    return Texture::CreateError(this, descriptor);
}

WGPUAdapter Device::APIGetAdapter() const {
    Ref<Adapter> adapter = mAdapter;
    return ReturnToAPI(std::move(adapter));
}

WGPUQueue Device::APIGetQueue() {
    Ref<Queue> queue = GetQueue();
    return ReturnToAPI(std::move(queue));
}

template <typename Event, typename Cmd, typename CallbackInfo, typename Descriptor>
WGPUFuture Device::CreatePipelineAsync(Descriptor const* descriptor,
                                       const CallbackInfo& callbackInfo) {
    using Pipeline = typename Event::Pipeline;

    Client* client = GetClient();
    Ref<Pipeline> pipeline = client->Make<Pipeline>();
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(AcquireRef(new Event(callbackInfo, pipeline)));
    if (!tracked) {
        return {futureIDInternal};
    }

    Cmd cmd;
    cmd.deviceId = GetWireHandle(client).id;
    cmd.descriptor = descriptor;
    cmd.eventManagerHandle = GetEventManagerHandle();
    cmd.future = {futureIDInternal};
    cmd.pipelineObjectHandle = pipeline->GetWireHandle(client);

    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

WGPUFuture Device::APICreateComputePipelineAsync(
    WGPUComputePipelineDescriptor const* descriptor,
    const WGPUCreateComputePipelineAsyncCallbackInfo& callbackInfo) {
    return CreatePipelineAsync<CreateComputePipelineEvent, DeviceCreateComputePipelineAsyncCmd>(
        descriptor, callbackInfo);
}

WireResult Client::DoDeviceCreateComputePipelineAsyncCallback(ObjectHandle eventManager,
                                                              WGPUFuture future,
                                                              WGPUCreatePipelineAsyncStatus status,
                                                              WGPUStringView message) {
    return SetFutureReady<CreateComputePipelineEvent>(eventManager, future.id, status, message);
}

WGPUFuture Device::APICreateRenderPipelineAsync(
    WGPURenderPipelineDescriptor const* descriptor,
    const WGPUCreateRenderPipelineAsyncCallbackInfo& callbackInfo) {
    return CreatePipelineAsync<CreateRenderPipelineEvent, DeviceCreateRenderPipelineAsyncCmd>(
        descriptor, callbackInfo);
}

WireResult Client::DoDeviceCreateRenderPipelineAsyncCallback(ObjectHandle eventManager,
                                                             WGPUFuture future,
                                                             WGPUCreatePipelineAsyncStatus status,
                                                             WGPUStringView message) {
    return SetFutureReady<CreateRenderPipelineEvent>(eventManager, future.id, status, message);
}

void Device::APIDestroy() {
    DeviceDestroyCmd cmd;
    cmd.self = ToAPI(this);
    GetClient()->SerializeCommand(cmd);

    mIsAlive = false;
}

}  // namespace dawn::wire::client
