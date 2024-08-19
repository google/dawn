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

// Future/async operation that need to be forwarded to JS.
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
  RefCounted() = default;
  virtual ~RefCounted() = default;

  void AddRef() {
    assert(mRefCount.fetch_add(1u, std::memory_order_relaxed) >= 1);
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
  X(SwapChain)           \
  X(Texture)             \
  X(TextureView)

// X Macro to help generate boilerplate code for all passthrough object types.
// Passthrough objects refer to objects that are implemented via JS objects.
#define WGPU_PASSTHROUGH_OBJECTS(X) \
  X(Adapter)             \
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
  X(SwapChain)           \
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
  RequestAdapter,
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
// Future events.
// ----------------------------------------------------------------------------

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
    mAdapter = adapter;
    mMessage = message;
  }

  void Complete(FutureID futureId, EventCompletionType type) override {
    if (type == EventCompletionType::Shutdown) {
      mStatus = WGPURequestAdapterStatus_InstanceDropped;
      mMessage = "A valid external Instance reference no longer exists.";
    }
    if (mCallback) {
      mCallback(
          mStatus,
          mStatus == WGPURequestAdapterStatus_Success ? mAdapter : nullptr,
          mMessage ? mMessage->c_str() : nullptr, mUserdata1, mUserdata2);
    }
  }

 private:
  WGPURequestAdapterCallback2 mCallback = nullptr;
  void* mUserdata1 = nullptr;
  void* mUserdata2 = nullptr;

  WGPURequestAdapterStatus mStatus;
  WGPUAdapter mAdapter = nullptr;
  std::optional<std::string> mMessage = std::nullopt;
};

// ----------------------------------------------------------------------------
// WGPU struct implementations.
// ----------------------------------------------------------------------------

// Default struct implementations.
#define DEFINE_WGPU_DEFAULT_STRUCT(Name) \
  struct WGPU##Name##Impl : public RefCounted {};
WGPU_PASSTHROUGH_OBJECTS(DEFINE_WGPU_DEFAULT_STRUCT)

// Instance is specially implemented in order to handle Futures implementation.
struct WGPUInstanceImpl : public RefCounted {
 public:
  WGPUInstanceImpl() {
    mId = GetNextInstanceId();
    GetEventManager().RegisterInstance(mId);
  }
  ~WGPUInstanceImpl() override { GetEventManager().UnregisterInstance(mId); }
  InstanceID GetId() const { return mId; }

  void ProcessEvents() { GetEventManager().ProcessEvents(mId); }

  WGPUWaitStatus WaitAny(size_t count,
                         WGPUFutureWaitInfo* infos,
                         uint64_t timeoutNS) {
    return GetEventManager().WaitAny(mId, count, infos, timeoutNS);
  }

 private:
  static InstanceID GetNextInstanceId() {
    static std::atomic<InstanceID> kNextInstanceId = 1;
    return kNextInstanceId++;
  }

  InstanceID mId;
};

// Device is specially implemented in order to handle refcounting the Queue.
struct WGPUDeviceImpl : public RefCounted {
 public:
  WGPUDeviceImpl(WGPUQueue queue) : mQueue(queue) {
    // TODO(lokokung) Currently we are manually doing the ref counting for
    //                the Queue. We should probably have some RAII helpers.
    mQueue->AddRef();
  }
  ~WGPUDeviceImpl() override { mQueue->Release(); }

  WGPUQueue GetQueue() { return mQueue; }

 private:
  WGPUQueue mQueue;
};

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

WGPUDevice emwgpuCreateDevice(WGPUQueue queue) {
  return new WGPUDeviceImpl(queue);
}

// Future event callbacks.
void emwgpuOnRequestAdapterCompleted(FutureID futureId,
                                     WGPURequestAdapterStatus status,
                                     WGPUAdapter adapter,
                                     const char* message) {
  GetEventManager().SetFutureReady<RequestAdapterEvent>(futureId, status,
                                                        adapter, message);
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

WGPUQueue wgpuDeviceGetQueue(WGPUDevice device) {
  device->GetQueue()->AddRef();
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
  auto [futureId, tracked] = GetEventManager().TrackEvent(
      std::make_unique<RequestAdapterEvent>(instance->GetId(), callbackInfo));
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
  WGPUTextureFormat preferredFormat = wgpuSurfaceGetPreferredFormat(surface, adapter);
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
// Methods of SwapChain
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Texture
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of TextureView
// ----------------------------------------------------------------------------
