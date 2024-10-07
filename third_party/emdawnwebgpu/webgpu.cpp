// Copyright 2024 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

//
// This file and library_webgpu.js together implement <webgpu/webgpu.h>.
//

#include <emscripten/emscripten.h>
#include <webgpu/webgpu.h>

#include <array>
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

using FutureID = uint64_t;
static constexpr FutureID kNullFutureId = 0;
using InstanceID = uint64_t;

// ----------------------------------------------------------------------------
// Declarations for JS emwgpu functions (defined in library_webgpu.js)
// ----------------------------------------------------------------------------
extern "C" {
void emwgpuDelete(void* id);

// Note that for the JS entry points, we pass uint64_t as pointer and decode it
// on the other side.
FutureID emwgpuWaitAny(FutureID const* futurePtr,
                       size_t futureCount,
                       uint64_t const* timeoutNSPtr);
WGPUTextureFormat emwgpuGetPreferredFormat();

// Future/async operation that need to be forwarded to JS.
void emwgpuAdapterRequestDevice(WGPUAdapter adapter,
                                FutureID futureId,
                                FutureID deviceLostFutureId,
                                WGPUDevice device,
                                WGPUQueue queue,
                                const WGPUDeviceDescriptor* descriptor);
void emwgpuDeviceCreateComputePipelineAsync(
    WGPUDevice device,
    FutureID futureId,
    const WGPUComputePipelineDescriptor* descriptor);
void emwgpuDeviceCreateRenderPipelineAsync(
    WGPUDevice device,
    FutureID futureId,
    const WGPURenderPipelineDescriptor* descriptor);
void emwgpuInstanceRequestAdapter(WGPUInstance instance,
                                  FutureID futureId,
                                  const WGPURequestAdapterOptions* options);
}  // extern "C"

// ----------------------------------------------------------------------------
// Implementation details that are not exposed upwards in the API.
// ----------------------------------------------------------------------------

class NonCopyable {
 protected:
  constexpr NonCopyable() = default;
  ~NonCopyable() = default;

  NonCopyable(NonCopyable&&) = default;
  NonCopyable& operator=(NonCopyable&&) = default;

 private:
  NonCopyable(const NonCopyable&) = delete;
  void operator=(const NonCopyable&) = delete;
};

class NonMovable : NonCopyable {
 protected:
  constexpr NonMovable() = default;
  ~NonMovable() = default;

 private:
  NonMovable(NonMovable&&) = delete;
  void operator=(NonMovable&&) = delete;
};

class RefCounted : NonMovable {
 public:
  static constexpr bool HasExternalRefCount = false;

  void AddRef() {
    [[maybe_unused]] uint64_t oldRefCount =
        mRefCount.fetch_add(1u, std::memory_order_relaxed);
    assert(oldRefCount >= 1);
  }

  void Release() {
    if (mRefCount.fetch_sub(1u, std::memory_order_release) == 1u) {
      std::atomic_thread_fence(std::memory_order_acquire);
      emwgpuDelete(this);
      delete this;
    }
  }

 private:
  std::atomic<uint64_t> mRefCount = 1;
};

class RefCountedWithExternalCount : public RefCounted {
 public:
  static constexpr bool HasExternalRefCount = true;

  virtual ~RefCountedWithExternalCount() = default;

  void AddRef() {
    AddExternalRef();
    RefCounted::AddRef();
  }

  void Release() {
    if (mExternalRefCount.fetch_sub(1u, std::memory_order_release) == 1u) {
      std::atomic_thread_fence(std::memory_order_acquire);
      WillDropLastExternalRef();
    }
    RefCounted::Release();
  }

  void AddExternalRef() {
    mExternalRefCount.fetch_add(1u, std::memory_order_relaxed);
  }

 private:
  virtual void WillDropLastExternalRef() = 0;

  std::atomic<uint64_t> mExternalRefCount = 0;
};

template <typename T>
class Ref {
 public:
  static_assert(std::is_convertible_v<T, RefCounted*>,
                "Cannot make a Ref<T> when T is not a Refcounted type.");

  Ref() : mValue(nullptr) {}
  ~Ref() { Release(mValue); }

  // Constructors from nullptr.
  // NOLINTNEXTLINE(runtime/explicit)
  constexpr Ref(std::nullptr_t) : Ref() {}

  // Constructors from T.
  // NOLINTNEXTLINE(runtime/explicit)
  Ref(T value) : mValue(value) { AddRef(value); }
  Ref<T>& operator=(const T& value) {
    Set(value);
    return *this;
  }

  // Constructors from a Ref<T>.
  Ref(const Ref<T>& other) : mValue(other.mValue) { AddRef(other.mValue); }
  Ref<T>& operator=(const Ref<T>& other) {
    Set(other.mValue);
    return *this;
  }
  Ref(Ref<T>&& other) { mValue = other.Detach(); }
  Ref<T>& operator=(Ref<T>&& other) {
    if (&other != this) {
      Release(mValue);
      mValue = other.Detach();
    }
    return *this;
  }

  explicit operator bool() const { return !!mValue; }

  // Smart pointer methods.
  const T& Get() const { return mValue; }
  T& Get() { return mValue; }
  const T operator->() const { return mValue; }
  T operator->() { return mValue; }

  [[nodiscard]] T Detach() {
    T value = mValue;
    mValue = nullptr;
    return value;
  }

  void Acquire(T value) {
    Release(mValue);
    mValue = value;
  }

 private:
  static void AddRef(T value) {
    if (value != nullptr) {
      value->RefCounted::AddRef();
    }
  }
  static void Release(T value) {
    if (value != nullptr) {
      value->RefCounted::Release();
    }
  }

  void Set(T value) {
    if (mValue != value) {
      // Ensure that the new value is referenced before the old is released to
      // prevent any transitive frees that may affect the new value.
      AddRef(value);
      Release(mValue);
      mValue = value;
    }
  }

  T mValue;
};

template <typename T>
auto ReturnToAPI(Ref<T*>&& object) {
  if constexpr (T::HasExternalRefCount) {
    // For an object which has external ref count, just need to increase the
    // external ref count, and keep the total ref count unchanged.
    object->AddExternalRef();
  }
  return object.Detach();
}

// clang-format off
// X Macro to help generate boilerplate code for all refcounted object types.
#define WGPU_REFCOUNTED_OBJECTS(X) \
  X(Adapter)             \
  X(BindGroup)           \
  X(BindGroupLayout)     \
  X(Buffer)              \
  X(CommandBuffer)       \
  X(CommandEncoder)      \
  X(ComputePassEncoder)  \
  X(ComputePipeline)     \
  X(Device)              \
  X(Instance)            \
  X(PipelineLayout)      \
  X(QuerySet)            \
  X(Queue)               \
  X(RenderBundle)        \
  X(RenderBundleEncoder) \
  X(RenderPassEncoder)   \
  X(RenderPipeline)      \
  X(Sampler)             \
  X(ShaderModule)        \
  X(Surface)             \
  X(Texture)             \
  X(TextureView)

// X Macro to help generate boilerplate code for all passthrough object types.
// Passthrough objects refer to objects that are implemented via JS objects.
#define WGPU_PASSTHROUGH_OBJECTS(X) \
  X(BindGroup)           \
  X(BindGroupLayout)     \
  X(Buffer)              \
  X(CommandBuffer)       \
  X(CommandEncoder)      \
  X(ComputePassEncoder)  \
  X(ComputePipeline)     \
  X(PipelineLayout)      \
  X(QuerySet)            \
  X(Queue)               \
  X(RenderBundle)        \
  X(RenderBundleEncoder) \
  X(RenderPassEncoder)   \
  X(RenderPipeline)      \
  X(Sampler)             \
  X(ShaderModule)        \
  X(Surface)             \
  X(Texture)             \
  X(TextureView)
// clang-format on

// ----------------------------------------------------------------------------
// Future related structures and helpers.
// ----------------------------------------------------------------------------

enum class EventCompletionType {
  Ready,
  Shutdown,
};
enum class EventType {
  CreateComputePipeline,
  CreateRenderPipeline,
  DeviceLost,
  RequestAdapter,
  RequestDevice,
};

class EventManager;
class TrackedEvent;

class TrackedEvent : NonMovable {
 public:
  virtual ~TrackedEvent() = default;
  virtual EventType GetType() = 0;
  virtual void Complete(FutureID futureId, EventCompletionType type) = 0;

 protected:
  TrackedEvent(InstanceID instance, WGPUCallbackMode mode)
      : mInstanceId(instance), mMode(mode) {}

 private:
  friend class EventManager;

  // Events need to keep track of the instance they came from for validation.
  const InstanceID mInstanceId;
  const WGPUCallbackMode mMode;
  bool mIsReady = false;
};

// Compositable class for objects that provide entry point(s) that produce
// Events, i.e. returns a Future.
//
// Note that while it would be nice to make it so that C++ entry points
// implemented in here, and called from JS could use this abstraction in
// signatures, pointers passed between JS and C++ in WASM do not cast properly
// and results in undefined behavior. As an example, given:
//   (1) WGPUAdapter emwgpuCreateAdapter(const EventSource* source);
//   (2) WGPUAdapter emwgpuCreateAdapter(WGPUInstance instance);
// I tried to use (1), but when calling from JS, the pointer is not correctly
// adjusted so the value we end up getting when calling GetInstanceId() is some
// garbage.
class EventSource {
 public:
  explicit EventSource(InstanceID instanceId) : mInstanceId(instanceId) {}
  InstanceID GetInstanceId() const { return mInstanceId; }

 private:
  const InstanceID mInstanceId = 0;
};

// Thread-safe EventManager class that tracks all events.
//
// Note that there is a single global EventManager that should be accessed via
// GetEventManager(). The EventManager needs to outlive all WGPUInstances in
// order to handle Spontaneous events.
class EventManager : NonMovable {
 public:
  void RegisterInstance(InstanceID instance) {
    assert(instance);
    std::unique_lock<std::mutex> lock(mMutex);
    mPerInstanceEvents.try_emplace(instance);
  }

  void UnregisterInstance(InstanceID instance) {
    assert(instance);
    std::unique_lock<std::mutex> lock(mMutex);
    auto it = mPerInstanceEvents.find(instance);
    assert(it != mPerInstanceEvents.end());

    // When unregistering the Instance, resolve all non-spontaneous callbacks
    // with Shutdown.
    for (const FutureID futureId : it->second) {
      if (auto it = mEvents.find(futureId); it != mEvents.end()) {
        it->second->Complete(futureId, EventCompletionType::Shutdown);
        mEvents.erase(it);
      }
    }
    mPerInstanceEvents.erase(instance);
  }

  void ProcessEvents(InstanceID instance) {
    assert(instance);
    std::vector<std::pair<FutureID, std::unique_ptr<TrackedEvent>>> completable;
    {
      std::unique_lock<std::mutex> lock(mMutex);
      auto instanceIt = mPerInstanceEvents.find(instance);
      assert(instanceIt != mPerInstanceEvents.end());
      auto& instanceFutureIds = instanceIt->second;

      // Note that we are only currently handling AllowProcessEvents events,
      // i.e. we are not handling AllowSpontaneous events in this loop.
      for (auto futureIdsIt = instanceFutureIds.begin();
           futureIdsIt != instanceFutureIds.end();) {
        FutureID futureId = *futureIdsIt;
        auto eventIt = mEvents.find(futureId);
        if (eventIt == mEvents.end()) {
          ++futureIdsIt;
          continue;
        }
        auto& event = eventIt->second;

        if (event->mMode == WGPUCallbackMode_AllowProcessEvents &&
            event->mIsReady) {
          completable.emplace_back(futureId, std::move(event));
          mEvents.erase(eventIt);
          futureIdsIt = instanceFutureIds.erase(futureIdsIt);
        } else {
          ++futureIdsIt;
        }
      }
    }

    // Since the sets are ordered, the events must already be ordered by
    // FutureID.
    for (auto& [futureId, event] : completable) {
      event->Complete(futureId, EventCompletionType::Ready);
    }
  }

  WGPUWaitStatus WaitAny(InstanceID instance,
                         size_t count,
                         WGPUFutureWaitInfo* infos,
                         uint64_t timeoutNS) {
    assert(instance);

    if (count == 0) {
      return WGPUWaitStatus_Success;
    }

    // To handle timeouts, use Asyncify and proxy back into JS.
    if (timeoutNS > 0) {
      // Cannot handle timeouts if we are not using Asyncify.
      if (!emscripten_has_asyncify()) {
        return WGPUWaitStatus_UnsupportedTimeout;
      }

      std::vector<FutureID> futures;
      std::unordered_map<FutureID, WGPUFutureWaitInfo*> futureIdToInfo;
      for (size_t i = 0; i < count; ++i) {
        futures.push_back(infos[i].future.id);
        futureIdToInfo.emplace(infos[i].future.id, &infos[i]);
      }

      bool hasTimeout = timeoutNS != UINT64_MAX;
      FutureID completedId = emwgpuWaitAny(futures.data(), count,
                                           hasTimeout ? &timeoutNS : nullptr);
      if (completedId == kNullFutureId) {
        return WGPUWaitStatus_TimedOut;
      }
      futureIdToInfo[completedId]->completed = true;

      std::unique_ptr<TrackedEvent> completed;
      {
        std::unique_lock<std::mutex> lock(mMutex);
        auto eventIt = mEvents.find(completedId);
        if (eventIt == mEvents.end()) {
          return WGPUWaitStatus_Success;
        }

        completed = std::move(eventIt->second);
        mEvents.erase(eventIt);
        if (auto instanceIt = mPerInstanceEvents.find(instance);
            instanceIt != mPerInstanceEvents.end()) {
          instanceIt->second.erase(completedId);
        }
      }

      if (completed) {
        completed->Complete(completedId, EventCompletionType::Ready);
      }
      return WGPUWaitStatus_Success;
    }

    std::map<FutureID, std::unique_ptr<TrackedEvent>> completable;
    bool anyCompleted = false;
    {
      std::unique_lock<std::mutex> lock(mMutex);
      auto instanceIt = mPerInstanceEvents.find(instance);
      assert(instanceIt != mPerInstanceEvents.end());
      auto& instanceFutureIds = instanceIt->second;

      for (size_t i = 0; i < count; ++i) {
        FutureID futureId = infos[i].future.id;
        auto eventIt = mEvents.find(futureId);
        if (eventIt == mEvents.end()) {
          infos[i].completed = true;
          continue;
        }

        auto& event = eventIt->second;
        assert(event->mInstanceId == instance);
        infos[i].completed = event->mIsReady;
        if (event->mIsReady) {
          anyCompleted = true;
          completable.emplace(futureId, std::move(event));
          mEvents.erase(eventIt);
          instanceFutureIds.erase(futureId);
        }
      }
    }

    // We used an ordered map to collect the events, so they must be ordered.
    for (auto& [futureId, event] : completable) {
      event->Complete(futureId, EventCompletionType::Ready);
    }
    return anyCompleted ? WGPUWaitStatus_Success : WGPUWaitStatus_TimedOut;
  }

  std::pair<FutureID, bool> TrackEvent(std::unique_ptr<TrackedEvent> event) {
    FutureID futureId = mNextFutureId++;
    InstanceID instance = event->mInstanceId;
    std::unique_lock<std::mutex> lock(mMutex);
    switch (event->mMode) {
      case WGPUCallbackMode_WaitAnyOnly:
      case WGPUCallbackMode_AllowProcessEvents: {
        auto it = mPerInstanceEvents.find(instance);
        if (it == mPerInstanceEvents.end()) {
          // The instance has already been unregistered so just complete this
          // event as shutdown now.
          event->Complete(futureId, EventCompletionType::Shutdown);
          return {futureId, false};
        }
        it->second.insert(futureId);
        mEvents.try_emplace(futureId, std::move(event));
        break;
      }
      case WGPUCallbackMode_AllowSpontaneous: {
        mEvents.try_emplace(futureId, std::move(event));
        break;
      }
      default: {
        // Invalid callback mode, so we just return kNullFutureId.
        return {kNullFutureId, false};
      }
    }
    return {futureId, true};
  }

  template <typename Event, typename... ReadyArgs>
  void SetFutureReady(FutureID futureId, ReadyArgs&&... readyArgs) {
    std::unique_ptr<TrackedEvent> spontaneousEvent;
    {
      std::unique_lock<std::mutex> lock(mMutex);
      auto eventIt = mEvents.find(futureId);
      if (eventIt == mEvents.end()) {
        return;
      }

      auto& event = eventIt->second;
      assert(event->GetType() == Event::kType);
      static_cast<Event*>(event.get())
          ->ReadyHook(std::forward<ReadyArgs>(readyArgs)...);
      event->mIsReady = true;

      // If the event can be spontaneously completed, prepare to do so now.
      if (event->mMode == WGPUCallbackMode_AllowSpontaneous) {
        spontaneousEvent = std::move(event);
        mEvents.erase(futureId);
      }
    }

    if (spontaneousEvent) {
      spontaneousEvent->Complete(futureId, EventCompletionType::Ready);
    }
  }

 private:
  std::mutex mMutex;
  std::atomic<FutureID> mNextFutureId = 1;

  // The EventManager separates events based on the WGPUInstance that the event
  // stems from.
  std::unordered_map<InstanceID, std::set<FutureID>> mPerInstanceEvents;
  std::unordered_map<FutureID, std::unique_ptr<TrackedEvent>> mEvents;
};

static EventManager& GetEventManager() {
  static EventManager kEventManager;
  return kEventManager;
}

// ----------------------------------------------------------------------------
// WGPU struct declarations.
// ----------------------------------------------------------------------------

// Default struct declarations.
#define DEFINE_WGPU_DEFAULT_STRUCT(Name) \
  struct WGPU##Name##Impl final : public RefCounted {};
WGPU_PASSTHROUGH_OBJECTS(DEFINE_WGPU_DEFAULT_STRUCT)

struct WGPUAdapterImpl final : public RefCounted, public EventSource {
 public:
  WGPUAdapterImpl(const EventSource* source);
};

// Device is specially implemented in order to handle refcounting the Queue.
struct WGPUDeviceImpl final : public RefCountedWithExternalCount,
                              public EventSource {
 public:
  // Reservation constructor used when calling RequestDevice.
  WGPUDeviceImpl(const EventSource* source,
                 const WGPUDeviceDescriptor* descriptor,
                 WGPUQueue queue);
  // Injection constructor used when we already have a backing Device.
  WGPUDeviceImpl(const EventSource* source, WGPUQueue queue);

  WGPUQueue GetQueue() const;

  void OnDeviceLost(WGPUDeviceLostReason reason, const char* message);
  void OnUncapturedError(WGPUErrorType type, char const* message);

 private:
  void WillDropLastExternalRef() override;

  Ref<WGPUQueue> mQueue;
  WGPUUncapturedErrorCallbackInfo2 mUncapturedErrorCallbackInfo =
      WGPU_UNCAPTURED_ERROR_CALLBACK_INFO_2_INIT;
  FutureID mDeviceLostFutureId = kNullFutureId;
};

// Instance is specially implemented in order to handle Futures implementation.
struct WGPUInstanceImpl final : public RefCounted, public EventSource {
 public:
  WGPUInstanceImpl();
  ~WGPUInstanceImpl();

  void ProcessEvents();
  WGPUWaitStatus WaitAny(size_t count,
                         WGPUFutureWaitInfo* infos,
                         uint64_t timeoutNS);

 private:
  static InstanceID GetNextInstanceId();
};

// ----------------------------------------------------------------------------
// Future events.
// ----------------------------------------------------------------------------

template <typename Pipeline, EventType Type, typename CallbackInfo>
class CreatePipelineEventBase final : public TrackedEvent {
 public:
  static constexpr EventType kType = Type;

  CreatePipelineEventBase(InstanceID instance, const CallbackInfo& callbackInfo)
      : TrackedEvent(instance, callbackInfo.mode),
        mCallback(callbackInfo.callback),
        mUserdata1(callbackInfo.userdata1),
        mUserdata2(callbackInfo.userdata2) {}

  EventType GetType() override { return kType; }

  void ReadyHook(WGPUCreatePipelineAsyncStatus status,
                 Pipeline pipeline,
                 const char* message) {
    mStatus = status;
    mPipeline.Acquire(pipeline);
    if (message) {
      mMessage = message;
    }
  }

  void Complete(FutureID, EventCompletionType type) override {
    if (type == EventCompletionType::Shutdown) {
      mStatus = WGPUCreatePipelineAsyncStatus_InstanceDropped;
      mMessage = "A valid external Instance reference no longer exists.";
    }
    if (mCallback) {
      mCallback(mStatus,
                mStatus == WGPUCreatePipelineAsyncStatus_Success
                    ? ReturnToAPI(std::move(mPipeline))
                    : nullptr,
                mMessage ? mMessage->c_str() : nullptr, mUserdata1, mUserdata2);
    }
  }

 private:
  using Callback = decltype(std::declval<CallbackInfo>().callback);
  Callback mCallback = nullptr;
  void* mUserdata1 = nullptr;
  void* mUserdata2 = nullptr;

  WGPUCreatePipelineAsyncStatus mStatus = WGPUCreatePipelineAsyncStatus_Success;
  Ref<Pipeline> mPipeline;
  std::optional<std::string> mMessage = std::nullopt;
};
using CreateComputePipelineEvent =
    CreatePipelineEventBase<WGPUComputePipeline,
                            EventType::CreateComputePipeline,
                            WGPUCreateComputePipelineAsyncCallbackInfo2>;
using CreateRenderPipelineEvent =
    CreatePipelineEventBase<WGPURenderPipeline,
                            EventType::CreateRenderPipeline,
                            WGPUCreateRenderPipelineAsyncCallbackInfo2>;

class DeviceLostEvent final : public TrackedEvent {
 public:
  static constexpr EventType kType = EventType::DeviceLost;

  DeviceLostEvent(InstanceID instance,
                  WGPUDevice device,
                  const WGPUDeviceLostCallbackInfo2& callbackInfo)
      : TrackedEvent(instance, callbackInfo.mode),
        mCallback(callbackInfo.callback),
        mUserdata1(callbackInfo.userdata1),
        mUserdata2(callbackInfo.userdata2),
        mDevice(device) {
    assert(mDevice);
  }

  EventType GetType() override { return kType; }

  void ReadyHook(WGPUDeviceLostReason reason, const char* message) {
    mReason = reason;
    if (message) {
      mMessage = message;
    }
  }

  void Complete(FutureID, EventCompletionType type) override {
    if (type == EventCompletionType::Shutdown) {
      mReason = WGPUDeviceLostReason_InstanceDropped;
      mMessage = "A valid external Instance reference no longer exists.";
    }
    if (mCallback) {
      WGPUDevice device = mReason != WGPUDeviceLostReason_FailedCreation
                              ? mDevice.Get()
                              : nullptr;
      mCallback(&device, mReason, mMessage ? mMessage->c_str() : nullptr,
                mUserdata1, mUserdata2);
    }
  }

 private:
  WGPUDeviceLostCallback2 mCallback = nullptr;
  void* mUserdata1 = nullptr;
  void* mUserdata2 = nullptr;

  Ref<WGPUDevice> mDevice;

  WGPUDeviceLostReason mReason;
  std::optional<std::string> mMessage;
};

class RequestAdapterEvent final : public TrackedEvent {
 public:
  static constexpr EventType kType = EventType::RequestAdapter;

  RequestAdapterEvent(InstanceID instance,
                      const WGPURequestAdapterCallbackInfo2& callbackInfo)
      : TrackedEvent(instance, callbackInfo.mode),
        mCallback(callbackInfo.callback),
        mUserdata1(callbackInfo.userdata1),
        mUserdata2(callbackInfo.userdata2) {}

  EventType GetType() override { return kType; }

  void ReadyHook(WGPURequestAdapterStatus status,
                 WGPUAdapter adapter,
                 const char* message) {
    mStatus = status;
    mAdapter.Acquire(adapter);
    if (message) {
      mMessage = message;
    }
  }

  void Complete(FutureID, EventCompletionType type) override {
    if (type == EventCompletionType::Shutdown) {
      mStatus = WGPURequestAdapterStatus_InstanceDropped;
      mMessage = "A valid external Instance reference no longer exists.";
    }
    if (mCallback) {
      mCallback(mStatus,
                mStatus == WGPURequestAdapterStatus_Success
                    ? ReturnToAPI(std::move(mAdapter))
                    : nullptr,
                mMessage ? mMessage->c_str() : nullptr, mUserdata1, mUserdata2);
    }
  }

 private:
  WGPURequestAdapterCallback2 mCallback = nullptr;
  void* mUserdata1 = nullptr;
  void* mUserdata2 = nullptr;

  WGPURequestAdapterStatus mStatus;
  Ref<WGPUAdapter> mAdapter;
  std::optional<std::string> mMessage = std::nullopt;
};

class RequestDeviceEvent final : public TrackedEvent {
 public:
  static constexpr EventType kType = EventType::RequestDevice;

  RequestDeviceEvent(InstanceID instance,
                     const WGPURequestDeviceCallbackInfo2& callbackInfo)
      : TrackedEvent(instance, callbackInfo.mode),
        mCallback(callbackInfo.callback),
        mUserdata1(callbackInfo.userdata1),
        mUserdata2(callbackInfo.userdata2) {}

  EventType GetType() override { return kType; }

  void ReadyHook(WGPURequestDeviceStatus status,
                 WGPUDevice device,
                 const char* message) {
    mStatus = status;
    mDevice.Acquire(device);
    if (message) {
      mMessage = message;
    }
  }

  void Complete(FutureID, EventCompletionType type) override {
    if (type == EventCompletionType::Shutdown) {
      mStatus = WGPURequestDeviceStatus_InstanceDropped;
      mMessage = "A valid external Instance reference no longer exists.";
    }
    if (mCallback) {
      mCallback(mStatus,
                mStatus == WGPURequestDeviceStatus_Success
                    ? ReturnToAPI(std::move(mDevice))
                    : nullptr,
                mMessage ? mMessage->c_str() : nullptr, mUserdata1, mUserdata2);
    }
  }

 private:
  WGPURequestDeviceCallback2 mCallback = nullptr;
  void* mUserdata1 = nullptr;
  void* mUserdata2 = nullptr;

  WGPURequestDeviceStatus mStatus;
  Ref<WGPUDevice> mDevice;
  std::optional<std::string> mMessage = std::nullopt;
};

// ----------------------------------------------------------------------------
// WGPU struct implementations.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// WGPUAdapterImpl implementations.
// ----------------------------------------------------------------------------

WGPUAdapterImpl::WGPUAdapterImpl(const EventSource* source)
    : EventSource(source->GetInstanceId()) {}

// ----------------------------------------------------------------------------
// WGPUDeviceImpl implementations.
// ----------------------------------------------------------------------------

WGPUDeviceImpl::WGPUDeviceImpl(const EventSource* source,
                               const WGPUDeviceDescriptor* descriptor,
                               WGPUQueue queue)
    : EventSource(source->GetInstanceId()),
      mUncapturedErrorCallbackInfo(descriptor->uncapturedErrorCallbackInfo2) {
  // Create the DeviceLostEvent now.
  std::tie(mDeviceLostFutureId, std::ignore) =
      GetEventManager().TrackEvent(std::make_unique<DeviceLostEvent>(
          source->GetInstanceId(), this, descriptor->deviceLostCallbackInfo2));
  mQueue.Acquire(queue);
}

WGPUDeviceImpl::WGPUDeviceImpl(const EventSource* source, WGPUQueue queue)
    : EventSource(source->GetInstanceId()) {
  mQueue.Acquire(queue);
}

WGPUQueue WGPUDeviceImpl::GetQueue() const {
  auto queue = mQueue;
  return ReturnToAPI(std::move(queue));
}

void WGPUDeviceImpl::OnDeviceLost(WGPUDeviceLostReason reason,
                                  const char* message) {
  if (mDeviceLostFutureId != kNullFutureId) {
    GetEventManager().SetFutureReady<DeviceLostEvent>(mDeviceLostFutureId,
                                                      reason, message);
  }
  mDeviceLostFutureId = kNullFutureId;
}

void WGPUDeviceImpl::OnUncapturedError(WGPUErrorType type,
                                       char const* message) {
  if (mUncapturedErrorCallbackInfo.callback) {
    WGPUDeviceImpl* device = this;
    mUncapturedErrorCallbackInfo.callback(
        &device, type, message, mUncapturedErrorCallbackInfo.userdata1,
        mUncapturedErrorCallbackInfo.userdata2);
  }
}

void WGPUDeviceImpl::WillDropLastExternalRef() {
  OnDeviceLost(WGPUDeviceLostReason_Destroyed, "Device was destroyed.");
}

// ----------------------------------------------------------------------------
// WGPUInstanceImpl implementations.
// ----------------------------------------------------------------------------

WGPUInstanceImpl::WGPUInstanceImpl() : EventSource(GetNextInstanceId()) {
  GetEventManager().RegisterInstance(GetInstanceId());
}
WGPUInstanceImpl::~WGPUInstanceImpl() {
  GetEventManager().UnregisterInstance(GetInstanceId());
}

void WGPUInstanceImpl::ProcessEvents() {
  GetEventManager().ProcessEvents(GetInstanceId());
}

WGPUWaitStatus WGPUInstanceImpl::WaitAny(size_t count,
                                         WGPUFutureWaitInfo* infos,
                                         uint64_t timeoutNS) {
  return GetEventManager().WaitAny(GetInstanceId(), count, infos, timeoutNS);
}

InstanceID WGPUInstanceImpl::GetNextInstanceId() {
  static std::atomic<InstanceID> kNextInstanceId = 1;
  return kNextInstanceId++;
}

// ----------------------------------------------------------------------------
// Definitions for C++ emwgpu functions (callable from library_webgpu.js)
// ----------------------------------------------------------------------------
extern "C" {

// Object creation helpers that all return a pointer which is used as a key
// in the JS object table in library_webgpu.js.
#define DEFINE_EMWGPU_DEFAULT_CREATE(Name) \
  WGPU##Name emwgpuCreate##Name() {        \
    return new WGPU##Name##Impl();         \
  }
WGPU_PASSTHROUGH_OBJECTS(DEFINE_EMWGPU_DEFAULT_CREATE)

WGPUAdapter emwgpuCreateAdapter(WGPUInstance instance) {
  return new WGPUAdapterImpl(instance);
}

WGPUDevice emwgpuCreateDevice(WGPUInstance instance, WGPUQueue queue) {
  return new WGPUDeviceImpl(instance, queue);
}

// Future event callbacks.
void emwgpuOnDeviceCreateComputePipelineCompleted(
    FutureID futureId,
    WGPUCreatePipelineAsyncStatus status,
    WGPUComputePipeline pipeline,
    const char* message) {
  GetEventManager().SetFutureReady<CreateComputePipelineEvent>(
      futureId, status, pipeline, message);
}
void emwgpuOnDeviceCreateRenderPipelineCompleted(
    FutureID futureId,
    WGPUCreatePipelineAsyncStatus status,
    WGPURenderPipeline pipeline,
    const char* message) {
  GetEventManager().SetFutureReady<CreateRenderPipelineEvent>(
      futureId, status, pipeline, message);
}
void emwgpuOnDeviceLostCompleted(FutureID futureId,
                                 WGPUDeviceLostReason reason,
                                 const char* message) {
  GetEventManager().SetFutureReady<DeviceLostEvent>(futureId, reason, message);
}
void emwgpuOnRequestAdapterCompleted(FutureID futureId,
                                     WGPURequestAdapterStatus status,
                                     WGPUAdapter adapter,
                                     const char* message) {
  GetEventManager().SetFutureReady<RequestAdapterEvent>(futureId, status,
                                                        adapter, message);
}
void emwgpuOnRequestDeviceCompleted(FutureID futureId,
                                    WGPURequestDeviceStatus status,
                                    WGPUDevice device,
                                    const char* message) {
  // This handler should always have a device since we pre-allocate it before
  // calling out to JS.
  assert(device);
  if (status == WGPURequestDeviceStatus_Success) {
    GetEventManager().SetFutureReady<RequestDeviceEvent>(futureId, status,
                                                         device, message);
  } else {
    // If the request failed, we need to resolve the DeviceLostEvent.
    device->OnDeviceLost(WGPUDeviceLostReason_FailedCreation,
                         "Device failed at creation.");
    GetEventManager().SetFutureReady<RequestDeviceEvent>(futureId, status,
                                                         nullptr, message);
  }
}

// Uncaptured error handler is similar to the Future event callbacks, but it
// doesn't go through the EventManager and just calls the callback on the Device
// immediately.
void emwgpuOnUncapturedError(WGPUDevice device,
                             WGPUErrorType type,
                             char const* message) {
  device->OnUncapturedError(type, message);
}

}  // extern "C"

// ----------------------------------------------------------------------------
// WebGPU function definitions, with methods organized by "class". Note these
// don't need to be extern "C" because they are already declared in webgpu.h.
//
// Also note that the full set of functions declared in webgpu.h are only
// partially implemeted here. The remaining ones are implemented via
// library_webgpu.js.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Common AddRef/Release APIs are batch generated via X macros for all objects.
// ----------------------------------------------------------------------------

#define DEFINE_WGPU_DEFAULT_ADDREF_RELEASE(Name) \
  void wgpu##Name##AddRef(WGPU##Name o) {        \
    o->AddRef();                                 \
  }                                              \
  void wgpu##Name##Release(WGPU##Name o) {       \
    o->Release();                                \
  }
WGPU_REFCOUNTED_OBJECTS(DEFINE_WGPU_DEFAULT_ADDREF_RELEASE)

// ----------------------------------------------------------------------------
// Standalone (non-method) functions
// ----------------------------------------------------------------------------

void wgpuAdapterInfoFreeMembers(WGPUAdapterInfo value) {
  free(const_cast<char*>(value.vendor));
  free(const_cast<char*>(value.architecture));
  free(const_cast<char*>(value.device));
  free(const_cast<char*>(value.description));
}

WGPUInstance wgpuCreateInstance([[maybe_unused]] const WGPUInstanceDescriptor* descriptor) {
  assert(descriptor == nullptr);  // descriptor not implemented yet
  return new WGPUInstanceImpl();
}

void wgpuSurfaceCapabilitiesFreeMembers(WGPUSurfaceCapabilities) {
  // wgpuSurfaceCapabilities doesn't currently allocate anything.
}

// ----------------------------------------------------------------------------
// Methods of Adapter
// ----------------------------------------------------------------------------

void wgpuAdapterRequestDevice(WGPUAdapter adapter,
                              const WGPUDeviceDescriptor* descriptor,
                              WGPURequestDeviceCallback callback,
                              void* userdata) {
  WGPURequestDeviceCallbackInfo2 callbackInfo = {};
  callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
  callbackInfo.callback = [](WGPURequestDeviceStatus status, WGPUDevice device,
                             char const* message, void* callback,
                             void* userdata) {
    auto cb = reinterpret_cast<WGPURequestDeviceCallback>(callback);
    cb(status, device, message, userdata);
  };
  callbackInfo.userdata1 = reinterpret_cast<void*>(callback);
  callbackInfo.userdata2 = userdata;
  wgpuAdapterRequestDevice2(adapter, descriptor, callbackInfo);
}

WGPUFuture wgpuAdapterRequestDevice2(
    WGPUAdapter adapter,
    const WGPUDeviceDescriptor* descriptor,
    WGPURequestDeviceCallbackInfo2 callbackInfo) {
  auto [futureId, tracked] =
      GetEventManager().TrackEvent(std::make_unique<RequestDeviceEvent>(
          adapter->GetInstanceId(), callbackInfo));
  if (!tracked) {
    return WGPUFuture{kNullFutureId};
  }

  static const WGPUDeviceDescriptor kDefaultDescriptor =
      WGPU_DEVICE_DESCRIPTOR_INIT;
  if (descriptor == nullptr) {
    descriptor = &kDefaultDescriptor;
  }

  // For RequestDevice, we always create a Device and Queue up front. The
  // Device is also immediately associated with the DeviceLostEvent.
  WGPUQueue queue = new WGPUQueueImpl();
  WGPUDevice device = new WGPUDeviceImpl(adapter, descriptor, queue);

  auto [deviceLostFutureId, _] = GetEventManager().TrackEvent(
      std::make_unique<DeviceLostEvent>(adapter->GetInstanceId(), device,
                                        descriptor->deviceLostCallbackInfo2));

  emwgpuAdapterRequestDevice(adapter, futureId, deviceLostFutureId, device,
                             queue, descriptor);
  return WGPUFuture{futureId};
}

// ----------------------------------------------------------------------------
// Methods of BindGroup
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of BindGroupLayout
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Buffer
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of CommandBuffer
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of CommandEncoder
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of ComputePassEncoder
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of ComputePipeline
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Device
// ----------------------------------------------------------------------------

void wgpuDeviceCreateComputePipelineAsync(
    WGPUDevice device,
    const WGPUComputePipelineDescriptor* descriptor,
    WGPUCreateComputePipelineAsyncCallback callback,
    void* userdata) {
  WGPUCreateComputePipelineAsyncCallbackInfo2 callbackInfo = {};
  callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
  callbackInfo.callback = [](WGPUCreatePipelineAsyncStatus status,
                             WGPUComputePipeline pipeline, char const* message,
                             void* callback, void* userdata) {
    auto cb =
        reinterpret_cast<WGPUCreateComputePipelineAsyncCallback>(callback);
    cb(status, pipeline, message, userdata);
  };
  callbackInfo.userdata1 = reinterpret_cast<void*>(callback);
  callbackInfo.userdata2 = userdata;
  wgpuDeviceCreateComputePipelineAsync2(device, descriptor, callbackInfo);
}

WGPUFuture wgpuDeviceCreateComputePipelineAsync2(
    WGPUDevice device,
    const WGPUComputePipelineDescriptor* descriptor,
    WGPUCreateComputePipelineAsyncCallbackInfo2 callbackInfo) {
  auto [futureId, tracked] =
      GetEventManager().TrackEvent(std::make_unique<CreateComputePipelineEvent>(
          device->GetInstanceId(), callbackInfo));
  if (!tracked) {
    return WGPUFuture{kNullFutureId};
  }

  emwgpuDeviceCreateComputePipelineAsync(device, futureId, descriptor);
  return WGPUFuture{futureId};
}

void wgpuDeviceCreateRenderPipelineAsync(
    WGPUDevice device,
    const WGPURenderPipelineDescriptor* descriptor,
    WGPUCreateRenderPipelineAsyncCallback callback,
    void* userdata) {
  WGPUCreateRenderPipelineAsyncCallbackInfo2 callbackInfo = {};
  callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
  callbackInfo.callback = [](WGPUCreatePipelineAsyncStatus status,
                             WGPURenderPipeline pipeline, char const* message,
                             void* callback, void* userdata) {
    auto cb = reinterpret_cast<WGPUCreateRenderPipelineAsyncCallback>(callback);
    cb(status, pipeline, message, userdata);
  };
  callbackInfo.userdata1 = reinterpret_cast<void*>(callback);
  callbackInfo.userdata2 = userdata;
  wgpuDeviceCreateRenderPipelineAsync2(device, descriptor, callbackInfo);
}

WGPUFuture wgpuDeviceCreateRenderPipelineAsync2(
    WGPUDevice device,
    const WGPURenderPipelineDescriptor* descriptor,
    WGPUCreateRenderPipelineAsyncCallbackInfo2 callbackInfo) {
  auto [futureId, tracked] =
      GetEventManager().TrackEvent(std::make_unique<CreateRenderPipelineEvent>(
          device->GetInstanceId(), callbackInfo));
  if (!tracked) {
    return WGPUFuture{kNullFutureId};
  }

  emwgpuDeviceCreateRenderPipelineAsync(device, futureId, descriptor);
  return WGPUFuture{futureId};
}

WGPUQueue wgpuDeviceGetQueue(WGPUDevice device) {
  return device->GetQueue();
}

// ----------------------------------------------------------------------------
// Methods of Instance
// ----------------------------------------------------------------------------

void wgpuInstanceProcessEvents(WGPUInstance instance) {
  instance->ProcessEvents();
}

void wgpuInstanceRequestAdapter(WGPUInstance instance,
                                WGPURequestAdapterOptions const* options,
                                WGPURequestAdapterCallback callback,
                                void* userdata) {
  WGPURequestAdapterCallbackInfo2 callbackInfo = {};
  callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
  callbackInfo.callback = [](WGPURequestAdapterStatus status,
                             WGPUAdapter adapter, char const* message,
                             void* callback, void* userdata) {
    auto cb = reinterpret_cast<WGPURequestAdapterCallback>(callback);
    cb(status, adapter, message, userdata);
  };
  callbackInfo.userdata1 = reinterpret_cast<void*>(callback);
  callbackInfo.userdata2 = userdata;
  wgpuInstanceRequestAdapter2(instance, options, callbackInfo);
}

WGPUFuture wgpuInstanceRequestAdapter2(
    WGPUInstance instance,
    WGPURequestAdapterOptions const* options,
    WGPURequestAdapterCallbackInfo2 callbackInfo) {
  auto [futureId, tracked] =
      GetEventManager().TrackEvent(std::make_unique<RequestAdapterEvent>(
          instance->GetInstanceId(), callbackInfo));
  if (!tracked) {
    return WGPUFuture{kNullFutureId};
  }

  emwgpuInstanceRequestAdapter(instance, futureId, options);
  return WGPUFuture{futureId};
}

WGPUWaitStatus wgpuInstanceWaitAny(WGPUInstance instance,
                                   size_t futureCount,
                                   WGPUFutureWaitInfo* futures,
                                   uint64_t timeoutNS) {
  return instance->WaitAny(futureCount, futures, timeoutNS);
}

// ----------------------------------------------------------------------------
// Methods of PipelineLayout
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of QuerySet
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Queue
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of RenderBundle
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of RenderBundleEncoder
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of RenderPassEncoder
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of RenderPipeline
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Sampler
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of ShaderModule
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Surface
// ----------------------------------------------------------------------------

WGPUStatus wgpuSurfaceGetCapabilities(WGPUSurface surface,
                                      WGPUAdapter adapter,
                                      WGPUSurfaceCapabilities* capabilities) {
  assert(capabilities->nextInChain == nullptr); // TODO: Return WGPUStatus_Error

  static constexpr std::array<WGPUTextureFormat, 3> kSurfaceFormatsRGBAFirst = {
    WGPUTextureFormat_RGBA8Unorm,
    WGPUTextureFormat_BGRA8Unorm,
    WGPUTextureFormat_RGBA16Float,
  };
  static constexpr std::array<WGPUTextureFormat, 3> kSurfaceFormatsBGRAFirst = {
    WGPUTextureFormat_BGRA8Unorm,
    WGPUTextureFormat_RGBA8Unorm,
    WGPUTextureFormat_RGBA16Float,
  };
  WGPUTextureFormat preferredFormat = emwgpuGetPreferredFormat();
  switch (preferredFormat) {
    case WGPUTextureFormat_RGBA8Unorm:
      capabilities->formatCount = kSurfaceFormatsRGBAFirst.size();
      capabilities->formats = kSurfaceFormatsRGBAFirst.data();
      break;
    case WGPUTextureFormat_BGRA8Unorm:
      capabilities->formatCount = kSurfaceFormatsBGRAFirst.size();
      capabilities->formats = kSurfaceFormatsBGRAFirst.data();
      break;
    default:
      assert(false);
      return WGPUStatus_Error;
  }

  {
    static constexpr WGPUPresentMode kPresentMode = WGPUPresentMode_Fifo;
    capabilities->presentModeCount = 1;
    capabilities->presentModes = &kPresentMode;
  }

  {
    static constexpr std::array<WGPUCompositeAlphaMode, 2> kAlphaModes = {
      WGPUCompositeAlphaMode_Opaque,
      WGPUCompositeAlphaMode_Premultiplied,
    };
    capabilities->alphaModeCount = kAlphaModes.size();
    capabilities->alphaModes = kAlphaModes.data();
  }

  return WGPUStatus_Success;
}

// ----------------------------------------------------------------------------
// Methods of Texture
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of TextureView
// ----------------------------------------------------------------------------
