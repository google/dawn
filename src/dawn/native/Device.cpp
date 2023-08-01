// Copyright 2017 The Dawn Authors
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

#include "dawn/native/Device.h"

#include <algorithm>
#include <array>
#include <mutex>
#include <utility>

#include "dawn/common/Log.h"
#include "dawn/common/Version_autogen.h"
#include "dawn/native/AsyncTask.h"
#include "dawn/native/AttachmentState.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/BlitBufferToDepthStencil.h"
#include "dawn/native/BlobCache.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/CompilationMessages.h"
#include "dawn/native/CreatePipelineAsyncTask.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/ErrorInjector.h"
#include "dawn/native/ErrorScope.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/Instance.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/PipelineCache.h"
#include "dawn/native/QuerySet.h"
#include "dawn/native/Queue.h"
#include "dawn/native/RenderBundleEncoder.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/Surface.h"
#include "dawn/native/SwapChain.h"
#include "dawn/native/Texture.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/native/utils/WGPUHelpers.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/metrics/HistogramMacros.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native {

// DeviceBase sub-structures

struct DeviceBase::Caches {
    ContentLessObjectCache<AttachmentState> attachmentStates;
    ContentLessObjectCache<BindGroupLayoutInternalBase> bindGroupLayouts;
    ContentLessObjectCache<ComputePipelineBase> computePipelines;
    ContentLessObjectCache<PipelineLayoutBase> pipelineLayouts;
    ContentLessObjectCache<RenderPipelineBase> renderPipelines;
    ContentLessObjectCache<SamplerBase> samplers;
    ContentLessObjectCache<ShaderModuleBase> shaderModules;
};

// Tries to find an object in the cache, creating and inserting into the cache if not found.
template <typename RefCountedT, typename CreateFn>
auto GetOrCreate(ContentLessObjectCache<RefCountedT>& cache,
                 RefCountedT* blueprint,
                 CreateFn createFn) {
    using ReturnType = decltype(createFn());

    // If we find the blueprint in the cache we can just return it.
    Ref<RefCountedT> result = cache.Find(blueprint);
    if (result != nullptr) {
        return ReturnType(result);
    }

    using UnwrappedReturnType = typename detail::UnwrapResultOrError<ReturnType>::type;
    static_assert(std::is_same_v<UnwrappedReturnType, Ref<RefCountedT>>,
                  "CreateFn should return an unwrapped type that is the same as Ref<RefCountedT>.");

    // Create the result and try inserting it. Note that inserts can race because the critical
    // sections here is disjoint, hence the checks to verify whether this thread inserted.
    if constexpr (!detail::IsResultOrError<ReturnType>::value) {
        result = createFn();
    } else {
        auto resultOrError = createFn();
        if (DAWN_UNLIKELY(resultOrError.IsError())) {
            return ReturnType(std::move(resultOrError.AcquireError()));
        }
        result = resultOrError.AcquireSuccess();
    }
    ASSERT(result.Get() != nullptr);

    bool inserted = false;
    std::tie(result, inserted) = cache.Insert(result.Get());
    return ReturnType(result);
}

struct DeviceBase::DeprecationWarnings {
    std::unordered_set<std::string> emitted;
    size_t count = 0;
};

namespace {
struct LoggingCallbackTask : CallbackTask {
  public:
    LoggingCallbackTask() = delete;
    LoggingCallbackTask(wgpu::LoggingCallback loggingCallback,
                        WGPULoggingType loggingType,
                        const char* message,
                        void* userdata)
        : mCallback(loggingCallback),
          mLoggingType(loggingType),
          mMessage(message),
          mUserdata(userdata) {
        // Since the FinishImpl() will be called in uncertain future in which time the message
        // may already disposed, we must keep a local copy in the CallbackTask.
    }

  private:
    void FinishImpl() override { mCallback(mLoggingType, mMessage.c_str(), mUserdata); }

    void HandleShutDownImpl() override {
        // Do the logging anyway
        mCallback(mLoggingType, mMessage.c_str(), mUserdata);
    }

    void HandleDeviceLossImpl() override { mCallback(mLoggingType, mMessage.c_str(), mUserdata); }

    // As all deferred callback tasks will be triggered before modifying the registered
    // callback or shutting down, we are ensured that callback function and userdata pointer
    // stored in tasks is valid when triggered.
    wgpu::LoggingCallback mCallback;
    WGPULoggingType mLoggingType;
    std::string mMessage;
    void* mUserdata;
};
}  // anonymous namespace

ResultOrError<Ref<PipelineLayoutBase>> ValidateLayoutAndGetComputePipelineDescriptorWithDefaults(
    DeviceBase* device,
    const ComputePipelineDescriptor& descriptor,
    ComputePipelineDescriptor* outDescriptor) {
    Ref<PipelineLayoutBase> layoutRef;
    *outDescriptor = descriptor;

    if (outDescriptor->layout == nullptr) {
        DAWN_TRY_ASSIGN(layoutRef, PipelineLayoutBase::CreateDefault(
                                       device, {{
                                                   SingleShaderStage::Compute,
                                                   outDescriptor->compute.module,
                                                   outDescriptor->compute.entryPoint,
                                                   outDescriptor->compute.constantCount,
                                                   outDescriptor->compute.constants,
                                               }}));
        outDescriptor->layout = layoutRef.Get();
    }

    return layoutRef;
}

ResultOrError<Ref<PipelineLayoutBase>> ValidateLayoutAndGetRenderPipelineDescriptorWithDefaults(
    DeviceBase* device,
    const RenderPipelineDescriptor& descriptor,
    RenderPipelineDescriptor* outDescriptor) {
    Ref<PipelineLayoutBase> layoutRef;
    *outDescriptor = descriptor;

    if (descriptor.layout == nullptr) {
        // Ref will keep the pipeline layout alive until the end of the function where
        // the pipeline will take another reference.
        DAWN_TRY_ASSIGN(layoutRef,
                        PipelineLayoutBase::CreateDefault(
                            device, GetRenderStagesAndSetPlaceholderShader(device, &descriptor)));
        outDescriptor->layout = layoutRef.Get();
    }

    return layoutRef;
}

// DeviceBase

DeviceBase::DeviceBase(AdapterBase* adapter,
                       const DeviceDescriptor* descriptor,
                       const TogglesState& deviceToggles)
    : mAdapter(adapter), mToggles(deviceToggles), mNextPipelineCompatibilityToken(1) {
    ASSERT(descriptor != nullptr);

    mDeviceLostCallback = descriptor->deviceLostCallback;
    mDeviceLostUserdata = descriptor->deviceLostUserdata;

    AdapterProperties adapterProperties;
    adapter->APIGetProperties(&adapterProperties);

    ApplyFeatures(descriptor);

    DawnCacheDeviceDescriptor defaultCacheDesc = {};
    const DawnCacheDeviceDescriptor* cacheDesc = nullptr;
    FindInChain(descriptor->nextInChain, &cacheDesc);
    if (cacheDesc == nullptr) {
        cacheDesc = &defaultCacheDesc;
    }

    if (descriptor->requiredLimits != nullptr) {
        mLimits.v1 = ReifyDefaultLimits(descriptor->requiredLimits->limits);
    } else {
        GetDefaultLimits(&mLimits.v1);
    }

    mFormatTable = BuildFormatTable(this);

    if (descriptor->label != nullptr && strlen(descriptor->label) != 0) {
        mLabel = descriptor->label;
    }

    // Record the cache key from the properties. Note that currently, if a new extension
    // descriptor is added (and probably handled here), the cache key recording needs to be
    // updated.
    StreamIn(&mDeviceCacheKey, kDawnVersion, adapterProperties, mEnabledFeatures.featuresBitSet,
             mToggles, cacheDesc);
}

DeviceBase::DeviceBase() : mState(State::Alive), mToggles(ToggleStage::Device) {
    GetDefaultLimits(&mLimits.v1);
    mFormatTable = BuildFormatTable(this);
}

DeviceBase::~DeviceBase() {
    // We need to explicitly release the Queue before we complete the destructor so that the
    // Queue does not get destroyed after the Device.
    mQueue = nullptr;
}

MaybeError DeviceBase::Initialize(Ref<QueueBase> defaultQueue) {
    SetWGSLExtensionAllowList();

    mQueue = std::move(defaultQueue);

#if defined(DAWN_ENABLE_ASSERTS)
    mUncapturedErrorCallback = [](WGPUErrorType, char const*, void*) {
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

    mCaches = std::make_unique<DeviceBase::Caches>();
    mErrorScopeStack = std::make_unique<ErrorScopeStack>();
    mDynamicUploader = std::make_unique<DynamicUploader>(this);
    mCallbackTaskManager = AcquireRef(new CallbackTaskManager());
    mDeprecationWarnings = std::make_unique<DeprecationWarnings>();
    mInternalPipelineStore = std::make_unique<InternalPipelineStore>(this);

    ASSERT(GetPlatform() != nullptr);
    mWorkerTaskPool = GetPlatform()->CreateWorkerTaskPool();
    mAsyncTaskManager = std::make_unique<AsyncTaskManager>(mWorkerTaskPool.get());

    // Starting from now the backend can start doing reentrant calls so the device is marked as
    // alive.
    mState = State::Alive;

    DAWN_TRY_ASSIGN(mEmptyBindGroupLayout, CreateEmptyBindGroupLayout());
    DAWN_TRY_ASSIGN(mEmptyPipelineLayout, CreateEmptyPipelineLayout());

    // If placeholder fragment shader module is needed, initialize it
    if (IsToggleEnabled(Toggle::UsePlaceholderFragmentInVertexOnlyPipeline)) {
        // The empty fragment shader, used as a work around for vertex-only render pipeline
        constexpr char kEmptyFragmentShader[] = R"(
                @fragment fn fs_empty_main() {}
            )";
        ShaderModuleDescriptor descriptor;
        ShaderModuleWGSLDescriptor wgslDesc;
        wgslDesc.code = kEmptyFragmentShader;
        descriptor.nextInChain = &wgslDesc;

        DAWN_TRY_ASSIGN(mInternalPipelineStore->placeholderFragmentShader,
                        CreateShaderModule(&descriptor));
    }

    if (HasFeature(Feature::ImplicitDeviceSynchronization)) {
        mMutex = AcquireRef(new Mutex);
    } else {
        mMutex = nullptr;
    }

    // mAdapter is not set for mock test devices.
    // TODO(crbug.com/dawn/1702): using a mock adapter could avoid the null checking.
    if (mAdapter != nullptr) {
        mAdapter->GetPhysicalDevice()->GetInstance()->AddDevice(this);
    }

    return {};
}

void DeviceBase::WillDropLastExternalRef() {
    {
        // This will be invoked by API side, so we need to lock.
        // Note: we cannot hold the lock when flushing the callbacks so have to limit the scope of
        // the lock.
        auto deviceLock(GetScopedLock());

        // DeviceBase uses RefCountedWithExternalCount to break refcycles.
        //
        // DeviceBase holds multiple Refs to various API objects (pipelines, buffers, etc.) which
        // are used to implement various device-level facilities. These objects are cached on the
        // device, so we want to keep them around instead of making transient allocations. However,
        // many of the objects also hold a Ref<Device> back to their parent device.
        //
        // In order to break this cycle and prevent leaks, when the application drops the last
        // external ref and WillDropLastExternalRef is called, the device clears out any member refs
        // to API objects that hold back-refs to the device - thus breaking any reference cycles.
        //
        // Currently, this is done by calling Destroy on the device to cease all in-flight work and
        // drop references to internal objects. We may want to lift this in the future, but it would
        // make things more complex because there might be pending tasks which hold a ref back to
        // the device - either directly or indirectly. We would need to ensure those tasks don't
        // create new reference cycles, and we would need to continuously try draining the pending
        // tasks to clear out all remaining refs.
        Destroy();
    }

    // Flush last remaining callback tasks.
    do {
        FlushCallbackTaskQueue();
    } while (!mCallbackTaskManager->IsEmpty());

    auto deviceLock(GetScopedLock());
    // Drop te device's reference to the queue. Because the application dropped the last external
    // references, they can no longer get the queue from APIGetQueue().
    mQueue = nullptr;

    // Reset callbacks since after this, since after dropping the last external reference, the
    // application may have freed any device-scope memory needed to run the callback.
    mUncapturedErrorCallback = [](WGPUErrorType, char const* message, void*) {
        dawn::WarningLog() << "Uncaptured error after last external device reference dropped.\n"
                           << message;
    };

    mDeviceLostCallback = [](WGPUDeviceLostReason, char const* message, void*) {
        dawn::WarningLog() << "Device lost after last external device reference dropped.\n"
                           << message;
    };

    // mAdapter is not set for mock test devices.
    // TODO(crbug.com/dawn/1702): using a mock adapter could avoid the null checking.
    if (mAdapter != nullptr) {
        mAdapter->GetPhysicalDevice()->GetInstance()->RemoveDevice(this);

        // Once last external ref dropped, all callbacks should be forwarded to Instance's callback
        // queue instead.
        mCallbackTaskManager =
            mAdapter->GetPhysicalDevice()->GetInstance()->GetCallbackTaskManager();
    }
}

void DeviceBase::DestroyObjects() {
    // List of object types in reverse "dependency" order so we can iterate and delete the
    // objects safely. We define dependent here such that if B has a ref to A, then B depends on
    // A. We therefore try to destroy B before destroying A. Note that this only considers the
    // immediate frontend dependencies, while backend objects could add complications and extra
    // dependencies.
    //
    // Note that AttachmentState is not an ApiObject so it cannot be eagerly destroyed. However,
    // since AttachmentStates are cached by the device, objects that hold references to
    // AttachmentStates should make sure to un-ref them in their Destroy operation so that we
    // can destroy the frontend cache.

    // clang-format off
        static constexpr std::array<ObjectType, 18> kObjectTypeDependencyOrder = {
            ObjectType::ComputePassEncoder,
            ObjectType::RenderPassEncoder,
            ObjectType::RenderBundleEncoder,
            ObjectType::RenderBundle,
            ObjectType::CommandEncoder,
            ObjectType::CommandBuffer,
            ObjectType::RenderPipeline,
            ObjectType::ComputePipeline,
            ObjectType::PipelineLayout,
            ObjectType::SwapChain,
            ObjectType::BindGroup,
            ObjectType::BindGroupLayout,
            ObjectType::ShaderModule,
            ObjectType::ExternalTexture,
            ObjectType::Texture,  // Note that Textures own the TextureViews.
            ObjectType::QuerySet,
            ObjectType::Sampler,
            ObjectType::Buffer,
        };
    // clang-format on

    for (ObjectType type : kObjectTypeDependencyOrder) {
        mObjectLists[type].Destroy();
    }
}

void DeviceBase::Destroy() {
    // Skip if we are already destroyed.
    if (mState == State::Destroyed) {
        return;
    }

    // This function may be called re-entrantly inside APITick(). Tick triggers callbacks
    // inside which the application may destroy the device. Thus, we should be careful not
    // to delete objects that are needed inside Tick after callbacks have been called.
    //  - mCallbackTaskManager is not deleted since we flush the callback queue at the end
    // of Tick(). Note: that flush should always be emtpy since all callbacks are drained
    // inside Destroy() so there should be no outstanding tasks holding objects alive.
    //  - Similiarly, mAsyncTaskManager is not deleted since we use it to return a status
    // from Tick() whether or not there is any more pending work.

    // Skip handling device facilities if they haven't even been created (or failed doing so)
    if (mState != State::BeingCreated) {
        // The device is being destroyed so it will be lost, call the application callback.
        if (mDeviceLostCallback != nullptr) {
            mCallbackTaskManager->AddCallbackTask(
                std::bind(mDeviceLostCallback, WGPUDeviceLostReason_Destroyed,
                          "Device was destroyed.", mDeviceLostUserdata));
            mDeviceLostCallback = nullptr;
        }

        // Call all the callbacks immediately as the device is about to shut down.
        // TODO(crbug.com/dawn/826): Cancel the tasks that are in flight if possible.
        mAsyncTaskManager->WaitAllPendingTasks();
        mCallbackTaskManager->HandleShutDown();
    }

    // Disconnect the device, depending on which state we are currently in.
    switch (mState) {
        case State::BeingCreated:
            // The GPU timeline was never started so we don't have to wait.
            break;

        case State::Alive:
            // Alive is the only state which can have GPU work happening. Wait for all of it to
            // complete before proceeding with destruction.
            // Ignore errors so that we can continue with destruction
            IgnoreErrors(WaitForIdleForDestruction());
            AssumeCommandsComplete();
            break;

        case State::BeingDisconnected:
            // Getting disconnected is a transient state happening in a single API call so there
            // is always an external reference keeping the Device alive, which means the
            // destructor cannot run while BeingDisconnected.
            UNREACHABLE();
            break;

        case State::Disconnected:
            break;

        case State::Destroyed:
            // If we are already destroyed we should've skipped this work entirely.
            UNREACHABLE();
            break;
    }
    ASSERT(GetCompletedCommandSerial() == GetLastSubmittedCommandSerial());

    if (mState != State::BeingCreated) {
        // The GPU timeline is finished.
        // Finish destroying all objects owned by the device and tick the queue-related tasks
        // since they should be complete. This must be done before DestroyImpl() it may
        // relinquish resources that will be freed by backends in the DestroyImpl() call.
        DestroyObjects();
        mQueue->Tick(GetCompletedCommandSerial());
        // Call TickImpl once last time to clean up resources
        // Ignore errors so that we can continue with destruction
        IgnoreErrors(TickImpl());
    }

    // At this point GPU operations are always finished, so we are in the disconnected state.
    // Note that currently this state change is required because some of the backend
    // implementations of DestroyImpl checks that we are disconnected before doing work.
    mState = State::Disconnected;

    // Note: mQueue is not released here since the application may still get it after calling
    // Destroy() via APIGetQueue.
    mDynamicUploader = nullptr;
    mEmptyBindGroupLayout = nullptr;
    mEmptyPipelineLayout = nullptr;
    mInternalPipelineStore = nullptr;
    mExternalTexturePlaceholderView = nullptr;

    AssumeCommandsComplete();

    // Now that the GPU timeline is empty, destroy the backend device.
    DestroyImpl();

    mCaches = nullptr;
    mState = State::Destroyed;
}

void DeviceBase::APIDestroy() {
    Destroy();
}

void DeviceBase::HandleError(std::unique_ptr<ErrorData> error,
                             InternalErrorType additionalAllowedErrors,
                             WGPUDeviceLostReason lost_reason) {
    AppendDebugLayerMessages(error.get());
    InternalErrorType allowedErrors =
        InternalErrorType::Validation | InternalErrorType::DeviceLost | additionalAllowedErrors;
    InternalErrorType type = error->GetType();
    if (type == InternalErrorType::DeviceLost) {
        mState = State::Disconnected;

        // If the ErrorInjector is enabled, then the device loss might be fake and the device
        // still be executing commands. Force a wait for idle in this case, with State being
        // Disconnected so we can detect this case in WaitForIdleForDestruction.
        if (ErrorInjectorEnabled()) {
            IgnoreErrors(WaitForIdleForDestruction());
        }

        // A real device lost happened. Set the state to disconnected as the device cannot be
        // used. Also tags all commands as completed since the device stopped running.
        AssumeCommandsComplete();
    } else if (!(allowedErrors & type)) {
        // If we receive an error which we did not explicitly allow, assume the backend can't
        // recover and proceed with device destruction. We first wait for all previous commands to
        // be completed so that backend objects can be freed immediately, before handling the loss.
        error->AppendContext("handling unexpected error type %s when allowed errors are %s.", type,
                             allowedErrors);

        // Move away from the Alive state so that the application cannot use this device
        // anymore.
        // TODO(crbug.com/dawn/831): Do we need atomics for this to become visible to other
        // threads in a multithreaded scenario?
        mState = State::BeingDisconnected;

        // Ignore errors so that we can continue with destruction
        // Assume all commands are complete after WaitForIdleForDestruction (because they were)
        IgnoreErrors(WaitForIdleForDestruction());
        IgnoreErrors(TickImpl());
        AssumeCommandsComplete();
        mState = State::Disconnected;

        // Now everything is as if the device was lost.
        type = InternalErrorType::DeviceLost;
    }

    // TODO(lokokung) Update call sites that take the c-string to take string_view.
    const std::string messageStr = error->GetFormattedMessage();
    if (type == InternalErrorType::DeviceLost) {
        // The device was lost, schedule the application callback's executation.
        // Note: we don't invoke the callbacks directly here because it could cause re-entrances ->
        // possible deadlock.
        if (mDeviceLostCallback != nullptr) {
            mCallbackTaskManager->AddCallbackTask([callback = mDeviceLostCallback, lost_reason,
                                                   messageStr, userdata = mDeviceLostUserdata] {
                callback(lost_reason, messageStr.c_str(), userdata);
            });
            mDeviceLostCallback = nullptr;
        }

        mQueue->HandleDeviceLoss();

        // TODO(crbug.com/dawn/826): Cancel the tasks that are in flight if possible.
        mAsyncTaskManager->WaitAllPendingTasks();
        mCallbackTaskManager->HandleDeviceLoss();

        // Still forward device loss errors to the error scopes so they all reject.
        mErrorScopeStack->HandleError(ToWGPUErrorType(type), messageStr.c_str());
    } else {
        // Pass the error to the error scope stack and call the uncaptured error callback
        // if it isn't handled. DeviceLost is not handled here because it should be
        // handled by the lost callback.
        bool captured = mErrorScopeStack->HandleError(ToWGPUErrorType(type), messageStr.c_str());
        if (!captured && mUncapturedErrorCallback != nullptr) {
            mCallbackTaskManager->AddCallbackTask([callback = mUncapturedErrorCallback, type,
                                                   messageStr,
                                                   userdata = mUncapturedErrorUserdata] {
                callback(static_cast<WGPUErrorType>(ToWGPUErrorType(type)), messageStr.c_str(),
                         userdata);
            });
        }
    }
}

void DeviceBase::ConsumeError(std::unique_ptr<ErrorData> error,
                              InternalErrorType additionalAllowedErrors) {
    ASSERT(error != nullptr);
    HandleError(std::move(error), additionalAllowedErrors);
}

void DeviceBase::APISetLoggingCallback(wgpu::LoggingCallback callback, void* userdata) {
    // The registered callback function and userdata pointer are stored and used by deferred
    // callback tasks, and after setting a different callback (especially in the case of
    // resetting) the resources pointed by such pointer may be freed. Flush all deferred
    // callback tasks to guarantee we are never going to use the previous callback after
    // this call.
    FlushCallbackTaskQueue();
    auto deviceLock(GetScopedLock());
    if (IsLost()) {
        return;
    }
    mLoggingCallback = callback;
    mLoggingUserdata = userdata;
}

void DeviceBase::APISetUncapturedErrorCallback(wgpu::ErrorCallback callback, void* userdata) {
    // The registered callback function and userdata pointer are stored and used by deferred
    // callback tasks, and after setting a different callback (especially in the case of
    // resetting) the resources pointed by such pointer may be freed. Flush all deferred
    // callback tasks to guarantee we are never going to use the previous callback after
    // this call.
    FlushCallbackTaskQueue();
    auto deviceLock(GetScopedLock());
    if (IsLost()) {
        return;
    }
    mUncapturedErrorCallback = callback;
    mUncapturedErrorUserdata = userdata;
}

void DeviceBase::APISetDeviceLostCallback(wgpu::DeviceLostCallback callback, void* userdata) {
    // TODO(chromium:1234617): Add a deprecation warning.

    // The registered callback function and userdata pointer are stored and used by deferred
    // callback tasks, and after setting a different callback (especially in the case of
    // resetting) the resources pointed by such pointer may be freed. Flush all deferred
    // callback tasks to guarantee we are never going to use the previous callback after
    // this call.
    FlushCallbackTaskQueue();
    auto deviceLock(GetScopedLock());
    if (IsLost()) {
        return;
    }
    mDeviceLostCallback = callback;
    mDeviceLostUserdata = userdata;
}

void DeviceBase::APIPushErrorScope(wgpu::ErrorFilter filter) {
    if (ConsumedError(ValidateErrorFilter(filter))) {
        return;
    }
    mErrorScopeStack->Push(filter);
}

void DeviceBase::APIPopErrorScope(wgpu::ErrorCallback callback, void* userdata) {
    if (callback == nullptr) {
        static wgpu::ErrorCallback defaultCallback = [](WGPUErrorType, char const*, void*) {};
        callback = defaultCallback;
    }
    if (IsLost()) {
        mCallbackTaskManager->AddCallbackTask(
            std::bind(callback, WGPUErrorType_DeviceLost, "GPU device disconnected", userdata));
        return;
    }
    if (mErrorScopeStack->Empty()) {
        mCallbackTaskManager->AddCallbackTask(
            std::bind(callback, WGPUErrorType_Unknown, "No error scopes to pop", userdata));
        return;
    }
    ErrorScope scope = mErrorScopeStack->Pop();
    mCallbackTaskManager->AddCallbackTask(
        [callback, errorType = static_cast<WGPUErrorType>(scope.GetErrorType()),
         message = scope.GetErrorMessage(),
         userdata] { callback(errorType, message.c_str(), userdata); });
}

BlobCache* DeviceBase::GetBlobCache() {
#if TINT_BUILD_WGSL_WRITER
    // TODO(crbug.com/dawn/1481): Shader caching currently has a dependency on the WGSL writer to
    // generate cache keys. We can lift the dependency once we also cache frontend parsing,
    // transformations, and reflection.
    return mAdapter->GetPhysicalDevice()->GetInstance()->GetBlobCache(
        !IsToggleEnabled(Toggle::DisableBlobCache));
#else
    return mAdapter->GetPhysicalDevice()->GetInstance()->GetBlobCache(false);
#endif
}

Blob DeviceBase::LoadCachedBlob(const CacheKey& key) {
    return GetBlobCache()->Load(key);
}

void DeviceBase::StoreCachedBlob(const CacheKey& key, const Blob& blob) {
    if (!blob.Empty()) {
        GetBlobCache()->Store(key, blob);
    }
}

MaybeError DeviceBase::ValidateObject(const ApiObjectBase* object) const {
    ASSERT(object != nullptr);
    DAWN_INVALID_IF(object->GetDevice() != this,
                    "%s is associated with %s, and cannot be used with %s.", object,
                    object->GetDevice(), this);

    // TODO(dawn:563): Preserve labels for error objects.
    DAWN_INVALID_IF(object->IsError(), "%s is invalid.", object);

    return {};
}

MaybeError DeviceBase::ValidateIsAlive() const {
    DAWN_INVALID_IF(mState != State::Alive, "%s is lost.", this);
    return {};
}

void DeviceBase::APIForceLoss(wgpu::DeviceLostReason reason, const char* message) {
    if (mState != State::Alive) {
        return;
    }
    // Note that since we are passing None as the allowedErrors, an additional message will be
    // appended noting that the error was unexpected. Since this call is for testing only it is not
    // too important, but useful for users to understand where the extra message is coming from.
    HandleError(DAWN_INTERNAL_ERROR(message), InternalErrorType::None, ToAPI(reason));
}

DeviceBase::State DeviceBase::GetState() const {
    return mState;
}

bool DeviceBase::IsLost() const {
    ASSERT(mState != State::BeingCreated);
    return mState != State::Alive;
}

ApiObjectList* DeviceBase::GetObjectTrackingList(ObjectType type) {
    return &mObjectLists[type];
}

AdapterBase* DeviceBase::GetAdapter() const {
    return mAdapter.Get();
}

PhysicalDeviceBase* DeviceBase::GetPhysicalDevice() const {
    return mAdapter->GetPhysicalDevice();
}

dawn::platform::Platform* DeviceBase::GetPlatform() const {
    return GetPhysicalDevice()->GetInstance()->GetPlatform();
}

InternalPipelineStore* DeviceBase::GetInternalPipelineStore() {
    return mInternalPipelineStore.get();
}

bool DeviceBase::HasPendingTasks() {
    return mAsyncTaskManager->HasPendingTasks() || !mCallbackTaskManager->IsEmpty();
}

bool DeviceBase::IsDeviceIdle() {
    if (HasPendingTasks()) {
        return false;
    }
    return !HasScheduledCommands();
}

ResultOrError<const Format*> DeviceBase::GetInternalFormat(wgpu::TextureFormat format) const {
    FormatIndex index = ComputeFormatIndex(format);
    DAWN_INVALID_IF(index >= mFormatTable.size(), "Unknown texture format %s.", format);

    const Format* internalFormat = &mFormatTable[index];
    DAWN_INVALID_IF(!internalFormat->IsSupported(), "Unsupported texture format %s, reason: %s.",
                    format, internalFormat->unsupportedReason);

    return internalFormat;
}

const Format& DeviceBase::GetValidInternalFormat(wgpu::TextureFormat format) const {
    FormatIndex index = ComputeFormatIndex(format);
    ASSERT(index < mFormatTable.size());
    ASSERT(mFormatTable[index].IsSupported());
    return mFormatTable[index];
}

const Format& DeviceBase::GetValidInternalFormat(FormatIndex index) const {
    ASSERT(index < mFormatTable.size());
    ASSERT(mFormatTable[index].IsSupported());
    return mFormatTable[index];
}

ResultOrError<Ref<BindGroupLayoutBase>> DeviceBase::GetOrCreateBindGroupLayout(
    const BindGroupLayoutDescriptor* descriptor,
    PipelineCompatibilityToken pipelineCompatibilityToken) {
    BindGroupLayoutInternalBase blueprint(this, descriptor, ApiObjectBase::kUntrackedByDevice);

    const size_t blueprintHash = blueprint.ComputeContentHash();
    blueprint.SetContentHash(blueprintHash);

    Ref<BindGroupLayoutInternalBase> internal;
    DAWN_TRY_ASSIGN(internal, GetOrCreate(mCaches->bindGroupLayouts, &blueprint,
                                          [&]() -> ResultOrError<Ref<BindGroupLayoutInternalBase>> {
                                              Ref<BindGroupLayoutInternalBase> result;
                                              DAWN_TRY_ASSIGN(
                                                  result, CreateBindGroupLayoutImpl(descriptor));
                                              result->SetContentHash(blueprintHash);
                                              return result;
                                          }));
    return AcquireRef(
        new BindGroupLayoutBase(this, descriptor->label, internal, pipelineCompatibilityToken));
}

// Private function used at initialization
ResultOrError<Ref<BindGroupLayoutBase>> DeviceBase::CreateEmptyBindGroupLayout() {
    BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 0;
    desc.entries = nullptr;

    return GetOrCreateBindGroupLayout(&desc);
}

ResultOrError<Ref<PipelineLayoutBase>> DeviceBase::CreateEmptyPipelineLayout() {
    PipelineLayoutDescriptor desc = {};
    desc.bindGroupLayoutCount = 0;
    desc.bindGroupLayouts = nullptr;

    return GetOrCreatePipelineLayout(&desc);
}

BindGroupLayoutBase* DeviceBase::GetEmptyBindGroupLayout() {
    ASSERT(mEmptyBindGroupLayout != nullptr);
    return mEmptyBindGroupLayout.Get();
}

PipelineLayoutBase* DeviceBase::GetEmptyPipelineLayout() {
    ASSERT(mEmptyPipelineLayout != nullptr);
    return mEmptyPipelineLayout.Get();
}

Ref<ComputePipelineBase> DeviceBase::GetCachedComputePipeline(
    ComputePipelineBase* uninitializedComputePipeline) {
    return mCaches->computePipelines.Find(uninitializedComputePipeline);
}

Ref<RenderPipelineBase> DeviceBase::GetCachedRenderPipeline(
    RenderPipelineBase* uninitializedRenderPipeline) {
    return mCaches->renderPipelines.Find(uninitializedRenderPipeline);
}

Ref<ComputePipelineBase> DeviceBase::AddOrGetCachedComputePipeline(
    Ref<ComputePipelineBase> computePipeline) {
    ASSERT(IsLockedByCurrentThreadIfNeeded());
    auto [pipeline, _] = mCaches->computePipelines.Insert(computePipeline.Get());
    return std::move(pipeline);
}

Ref<RenderPipelineBase> DeviceBase::AddOrGetCachedRenderPipeline(
    Ref<RenderPipelineBase> renderPipeline) {
    ASSERT(IsLockedByCurrentThreadIfNeeded());
    auto [pipeline, _] = mCaches->renderPipelines.Insert(renderPipeline.Get());
    return std::move(pipeline);
}

ResultOrError<Ref<TextureViewBase>> DeviceBase::CreateImplicitMSAARenderTextureViewFor(
    const TextureBase* singleSampledTexture,
    uint32_t sampleCount) {
    ASSERT(IsLockedByCurrentThreadIfNeeded());

    TextureDescriptor desc = {};
    desc.dimension = wgpu::TextureDimension::e2D;
    desc.format = singleSampledTexture->GetFormat().format;
    desc.size = {singleSampledTexture->GetWidth(), singleSampledTexture->GetHeight(), 1};
    desc.sampleCount = sampleCount;
    desc.usage = wgpu::TextureUsage::RenderAttachment;
    if (HasFeature(Feature::TransientAttachments)) {
        desc.usage = desc.usage | wgpu::TextureUsage::TransientAttachment;
    }

    Ref<TextureBase> msaaTexture;
    Ref<TextureViewBase> msaaTextureView;

    DAWN_TRY_ASSIGN(msaaTexture, CreateTexture(&desc));

    DAWN_TRY_ASSIGN(msaaTextureView, msaaTexture->CreateView());

    return std::move(msaaTextureView);
}

ResultOrError<Ref<TextureViewBase>>
DeviceBase::GetOrCreatePlaceholderTextureViewForExternalTexture() {
    if (!mExternalTexturePlaceholderView.Get()) {
        Ref<TextureBase> externalTexturePlaceholder;
        TextureDescriptor textureDesc;
        textureDesc.dimension = wgpu::TextureDimension::e2D;
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        textureDesc.label = "Dawn_External_Texture_Placeholder_Texture";
        textureDesc.size = {1, 1, 1};
        textureDesc.usage = wgpu::TextureUsage::TextureBinding;

        DAWN_TRY_ASSIGN(externalTexturePlaceholder, CreateTexture(&textureDesc));

        TextureViewDescriptor textureViewDesc;
        textureViewDesc.arrayLayerCount = 1;
        textureViewDesc.aspect = wgpu::TextureAspect::All;
        textureViewDesc.baseArrayLayer = 0;
        textureViewDesc.dimension = wgpu::TextureViewDimension::e2D;
        textureViewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        textureViewDesc.label = "Dawn_External_Texture_Placeholder_Texture_View";
        textureViewDesc.mipLevelCount = 1;

        DAWN_TRY_ASSIGN(mExternalTexturePlaceholderView,
                        CreateTextureView(externalTexturePlaceholder.Get(), &textureViewDesc));
    }

    return mExternalTexturePlaceholderView;
}

ResultOrError<Ref<PipelineLayoutBase>> DeviceBase::GetOrCreatePipelineLayout(
    const PipelineLayoutDescriptor* descriptor) {
    PipelineLayoutBase blueprint(this, descriptor, ApiObjectBase::kUntrackedByDevice);

    const size_t blueprintHash = blueprint.ComputeContentHash();
    blueprint.SetContentHash(blueprintHash);

    return GetOrCreate(mCaches->pipelineLayouts, &blueprint,
                       [&]() -> ResultOrError<Ref<PipelineLayoutBase>> {
                           Ref<PipelineLayoutBase> result;
                           DAWN_TRY_ASSIGN(result, CreatePipelineLayoutImpl(descriptor));
                           result->SetContentHash(blueprintHash);
                           return result;
                       });
}

ResultOrError<Ref<SamplerBase>> DeviceBase::GetOrCreateSampler(
    const SamplerDescriptor* descriptor) {
    SamplerBase blueprint(this, descriptor, ApiObjectBase::kUntrackedByDevice);

    const size_t blueprintHash = blueprint.ComputeContentHash();
    blueprint.SetContentHash(blueprintHash);

    return GetOrCreate(mCaches->samplers, &blueprint, [&]() -> ResultOrError<Ref<SamplerBase>> {
        Ref<SamplerBase> result;
        DAWN_TRY_ASSIGN(result, CreateSamplerImpl(descriptor));
        result->SetContentHash(blueprintHash);
        return result;
    });
}

ResultOrError<Ref<ShaderModuleBase>> DeviceBase::GetOrCreateShaderModule(
    const ShaderModuleDescriptor* descriptor,
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* compilationMessages) {
    ASSERT(parseResult != nullptr);

    ShaderModuleBase blueprint(this, descriptor, ApiObjectBase::kUntrackedByDevice);

    const size_t blueprintHash = blueprint.ComputeContentHash();
    blueprint.SetContentHash(blueprintHash);

    return GetOrCreate(
        mCaches->shaderModules, &blueprint, [&]() -> ResultOrError<Ref<ShaderModuleBase>> {
            if (!parseResult->HasParsedShader()) {
                // We skip the parse on creation if validation isn't enabled which let's us quickly
                // lookup in the cache without validating and parsing. We need the parsed module
                // now.
                ASSERT(!IsValidationEnabled());
                DAWN_TRY(ValidateAndParseShaderModule(this, descriptor, parseResult,
                                                      compilationMessages));
            }

            ResultOrError<Ref<ShaderModuleBase>> result_or_error = [&] {
                SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(GetPlatform(), "CreateShaderModuleUS");
                return CreateShaderModuleImpl(descriptor, parseResult, compilationMessages);
            }();
            DAWN_HISTOGRAM_BOOLEAN(GetPlatform(), "CreateShaderModuleSuccess",
                                   result_or_error.IsSuccess());

            Ref<ShaderModuleBase> result;
            DAWN_TRY_ASSIGN(result, std::move(result_or_error));
            result->SetContentHash(blueprintHash);
            return result;
        });
}

Ref<AttachmentState> DeviceBase::GetOrCreateAttachmentState(AttachmentState* blueprint) {
    return GetOrCreate(mCaches->attachmentStates, blueprint, [&]() -> Ref<AttachmentState> {
        return AcquireRef(new AttachmentState(*blueprint));
    });
}

Ref<AttachmentState> DeviceBase::GetOrCreateAttachmentState(
    const RenderBundleEncoderDescriptor* descriptor) {
    AttachmentState blueprint(this, descriptor);
    return GetOrCreateAttachmentState(&blueprint);
}

Ref<AttachmentState> DeviceBase::GetOrCreateAttachmentState(
    const RenderPipelineDescriptor* descriptor) {
    AttachmentState blueprint(this, descriptor);
    return GetOrCreateAttachmentState(&blueprint);
}

Ref<AttachmentState> DeviceBase::GetOrCreateAttachmentState(
    const RenderPassDescriptor* descriptor) {
    AttachmentState blueprint(this, descriptor);
    return GetOrCreateAttachmentState(&blueprint);
}

Ref<PipelineCacheBase> DeviceBase::GetOrCreatePipelineCache(const CacheKey& key) {
    return GetOrCreatePipelineCacheImpl(key);
}

// Object creation API methods

BindGroupBase* DeviceBase::APICreateBindGroup(const BindGroupDescriptor* descriptor) {
    Ref<BindGroupBase> result;
    if (ConsumedError(CreateBindGroup(descriptor), &result, "calling %s.CreateBindGroup(%s).", this,
                      descriptor)) {
        return BindGroupBase::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    return result.Detach();
}
BindGroupLayoutBase* DeviceBase::APICreateBindGroupLayout(
    const BindGroupLayoutDescriptor* descriptor) {
    Ref<BindGroupLayoutBase> result;
    if (ConsumedError(CreateBindGroupLayout(descriptor), &result,
                      "calling %s.CreateBindGroupLayout(%s).", this, descriptor)) {
        return BindGroupLayoutBase::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    return result.Detach();
}
BufferBase* DeviceBase::APICreateBuffer(const BufferDescriptor* descriptor) {
    Ref<BufferBase> result = nullptr;
    if (ConsumedError(CreateBuffer(descriptor), &result, InternalErrorType::OutOfMemory,
                      "calling %s.CreateBuffer(%s).", this, descriptor)) {
        ASSERT(result == nullptr);
        return BufferBase::MakeError(this, descriptor);
    }
    return result.Detach();
}
CommandEncoder* DeviceBase::APICreateCommandEncoder(const CommandEncoderDescriptor* descriptor) {
    Ref<CommandEncoder> result;
    if (ConsumedError(CreateCommandEncoder(descriptor), &result,
                      "calling %s.CreateCommandEncoder(%s).", this, descriptor)) {
        return CommandEncoder::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    return result.Detach();
}
ComputePipelineBase* DeviceBase::APICreateComputePipeline(
    const ComputePipelineDescriptor* descriptor) {
    TRACE_EVENT1(GetPlatform(), General, "DeviceBase::APICreateComputePipeline", "label",
                 utils::GetLabelForTrace(descriptor->label));

    Ref<ComputePipelineBase> result;
    if (ConsumedError(CreateComputePipeline(descriptor), &result, InternalErrorType::Internal,
                      "calling %s.CreateComputePipeline(%s).", this, descriptor)) {
        return ComputePipelineBase::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    return result.Detach();
}
void DeviceBase::APICreateComputePipelineAsync(const ComputePipelineDescriptor* descriptor,
                                               WGPUCreateComputePipelineAsyncCallback callback,
                                               void* userdata) {
    TRACE_EVENT1(GetPlatform(), General, "DeviceBase::APICreateComputePipelineAsync", "label",
                 utils::GetLabelForTrace(descriptor->label));

    auto resultOrError = CreateUninitializedComputePipeline(descriptor);
    // Enqueue the callback directly when an error has been found in the front-end
    // validation.
    if (resultOrError.IsError()) {
        AddComputePipelineAsyncCallbackTask(resultOrError.AcquireError(), descriptor->label,
                                            callback, userdata);
        return;
    }

    Ref<ComputePipelineBase> uninitializedComputePipeline = resultOrError.AcquireSuccess();

    // Call the callback directly when we can get a cached compute pipeline object.
    Ref<ComputePipelineBase> cachedComputePipeline =
        GetCachedComputePipeline(uninitializedComputePipeline.Get());
    if (cachedComputePipeline.Get() != nullptr) {
        mCallbackTaskManager->AddCallbackTask(
            std::bind(callback, WGPUCreatePipelineAsyncStatus_Success,
                      ToAPI(cachedComputePipeline.Detach()), "", userdata));
    } else {
        // Otherwise we will create the pipeline object in InitializeComputePipelineAsyncImpl(),
        // where the pipeline object may be initialized asynchronously and the result will be
        // saved to mCreatePipelineAsyncTracker.
        InitializeComputePipelineAsyncImpl(std::move(uninitializedComputePipeline), callback,
                                           userdata);
    }
}
PipelineLayoutBase* DeviceBase::APICreatePipelineLayout(
    const PipelineLayoutDescriptor* descriptor) {
    Ref<PipelineLayoutBase> result;
    if (ConsumedError(CreatePipelineLayout(descriptor), &result,
                      "calling %s.CreatePipelineLayout(%s).", this, descriptor)) {
        return PipelineLayoutBase::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    return result.Detach();
}
QuerySetBase* DeviceBase::APICreateQuerySet(const QuerySetDescriptor* descriptor) {
    Ref<QuerySetBase> result;
    if (ConsumedError(CreateQuerySet(descriptor), &result, InternalErrorType::OutOfMemory,
                      "calling %s.CreateQuerySet(%s).", this, descriptor)) {
        return QuerySetBase::MakeError(this, descriptor);
    }
    return result.Detach();
}
SamplerBase* DeviceBase::APICreateSampler(const SamplerDescriptor* descriptor) {
    Ref<SamplerBase> result;
    if (ConsumedError(CreateSampler(descriptor), &result, "calling %s.CreateSampler(%s).", this,
                      descriptor)) {
        return SamplerBase::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    return result.Detach();
}
void DeviceBase::APICreateRenderPipelineAsync(const RenderPipelineDescriptor* descriptor,
                                              WGPUCreateRenderPipelineAsyncCallback callback,
                                              void* userdata) {
    TRACE_EVENT1(GetPlatform(), General, "DeviceBase::APICreateRenderPipelineAsync", "label",
                 utils::GetLabelForTrace(descriptor->label));

    auto resultOrError = CreateUninitializedRenderPipeline(descriptor);

    // Enqueue the callback directly when an error has been found in the front-end
    // validation.
    if (resultOrError.IsError()) {
        AddRenderPipelineAsyncCallbackTask(resultOrError.AcquireError(), descriptor->label,
                                           callback, userdata);
        return;
    }

    Ref<RenderPipelineBase> uninitializedRenderPipeline = resultOrError.AcquireSuccess();

    // Call the callback directly when we can get a cached render pipeline object.
    Ref<RenderPipelineBase> cachedRenderPipeline =
        GetCachedRenderPipeline(uninitializedRenderPipeline.Get());
    if (cachedRenderPipeline != nullptr) {
        mCallbackTaskManager->AddCallbackTask(
            std::bind(callback, WGPUCreatePipelineAsyncStatus_Success,
                      ToAPI(cachedRenderPipeline.Detach()), "", userdata));
    } else {
        // Otherwise we will create the pipeline object in InitializeRenderPipelineAsyncImpl(),
        // where the pipeline object may be initialized asynchronously and the result will be
        // saved to mCreatePipelineAsyncTracker.
        InitializeRenderPipelineAsyncImpl(std::move(uninitializedRenderPipeline), callback,
                                          userdata);
    }
}

RenderBundleEncoder* DeviceBase::APICreateRenderBundleEncoder(
    const RenderBundleEncoderDescriptor* descriptor) {
    Ref<RenderBundleEncoder> result;
    if (ConsumedError(CreateRenderBundleEncoder(descriptor), &result,
                      "calling %s.CreateRenderBundleEncoder(%s).", this, descriptor)) {
        return RenderBundleEncoder::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    return result.Detach();
}
RenderPipelineBase* DeviceBase::APICreateRenderPipeline(
    const RenderPipelineDescriptor* descriptor) {
    TRACE_EVENT1(GetPlatform(), General, "DeviceBase::APICreateRenderPipeline", "label",
                 utils::GetLabelForTrace(descriptor->label));

    Ref<RenderPipelineBase> result;
    if (ConsumedError(CreateRenderPipeline(descriptor), &result, InternalErrorType::Internal,
                      "calling %s.CreateRenderPipeline(%s).", this, descriptor)) {
        return RenderPipelineBase::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    return result.Detach();
}
ShaderModuleBase* DeviceBase::APICreateShaderModule(const ShaderModuleDescriptor* descriptor) {
    TRACE_EVENT1(GetPlatform(), General, "DeviceBase::APICreateShaderModule", "label",
                 utils::GetLabelForTrace(descriptor->label));

    Ref<ShaderModuleBase> result;
    std::unique_ptr<OwnedCompilationMessages> compilationMessages(
        std::make_unique<OwnedCompilationMessages>());
    if (ConsumedError(CreateShaderModule(descriptor, compilationMessages.get()), &result,
                      "calling %s.CreateShaderModule(%s).", this, descriptor)) {
        DAWN_ASSERT(result == nullptr);
        result = ShaderModuleBase::MakeError(this, descriptor ? descriptor->label : nullptr);
    }
    // Move compilation messages into ShaderModuleBase and emit tint errors and warnings
    // after all other operations are finished, even if any of them is failed and result
    // is an error shader module.
    result->InjectCompilationMessages(std::move(compilationMessages));

    return result.Detach();
}
ShaderModuleBase* DeviceBase::APICreateErrorShaderModule(const ShaderModuleDescriptor* descriptor,
                                                         const char* errorMessage) {
    Ref<ShaderModuleBase> result =
        ShaderModuleBase::MakeError(this, descriptor ? descriptor->label : nullptr);
    std::unique_ptr<OwnedCompilationMessages> compilationMessages(
        std::make_unique<OwnedCompilationMessages>());
    compilationMessages->AddMessage(errorMessage, wgpu::CompilationMessageType::Error);
    result->InjectCompilationMessages(std::move(compilationMessages));

    std::unique_ptr<ErrorData> errorData =
        DAWN_VALIDATION_ERROR("Error in calling %s.CreateShaderModule(%s).", this, descriptor);
    ConsumeError(std::move(errorData));

    return result.Detach();
}
SwapChainBase* DeviceBase::APICreateSwapChain(Surface* surface,
                                              const SwapChainDescriptor* descriptor) {
    Ref<SwapChainBase> result;
    if (ConsumedError(CreateSwapChain(surface, descriptor), &result,
                      "calling %s.CreateSwapChain(%s).", this, descriptor)) {
        return SwapChainBase::MakeError(this, descriptor);
    }
    return result.Detach();
}
TextureBase* DeviceBase::APICreateTexture(const TextureDescriptor* descriptor) {
    Ref<TextureBase> result;
    if (ConsumedError(CreateTexture(descriptor), &result, InternalErrorType::OutOfMemory,
                      "calling %s.CreateTexture(%s).", this, descriptor)) {
        return TextureBase::MakeError(this, descriptor);
    }
    return result.Detach();
}

wgpu::TextureUsage DeviceBase::APIGetSupportedSurfaceUsage(Surface* surface) {
    wgpu::TextureUsage result;
    if (ConsumedError(GetSupportedSurfaceUsage(surface), &result,
                      "calling %s.GetSupportedSurfaceUsage().", this)) {
        return wgpu::TextureUsage::None;
    }
    return result;
}

// For Dawn Wire

BufferBase* DeviceBase::APICreateErrorBuffer(const BufferDescriptor* desc) {
    BufferDescriptor fakeDescriptor = *desc;
    fakeDescriptor.nextInChain = nullptr;

    // The validation errors on BufferDescriptor should be prior to any OOM errors when
    // MapppedAtCreation == false.
    MaybeError maybeError = ValidateBufferDescriptor(this, &fakeDescriptor);

    // Set the size of the error buffer to 0 as this function is called only when an OOM happens at
    // the client side.
    fakeDescriptor.size = 0;

    if (maybeError.IsError()) {
        std::unique_ptr<ErrorData> error = maybeError.AcquireError();
        error->AppendContext("calling %s.CreateBuffer(%s).", this, desc);
        HandleError(std::move(error), InternalErrorType::OutOfMemory);
    } else {
        const DawnBufferDescriptorErrorInfoFromWireClient* clientErrorInfo = nullptr;
        FindInChain(desc->nextInChain, &clientErrorInfo);
        if (clientErrorInfo != nullptr && clientErrorInfo->outOfMemory) {
            HandleError(DAWN_OUT_OF_MEMORY_ERROR("Failed to allocate memory for buffer mapping"),
                        InternalErrorType::OutOfMemory);
        }
    }

    return BufferBase::MakeError(this, &fakeDescriptor);
}

ExternalTextureBase* DeviceBase::APICreateErrorExternalTexture() {
    return ExternalTextureBase::MakeError(this);
}

TextureBase* DeviceBase::APICreateErrorTexture(const TextureDescriptor* desc) {
    return TextureBase::MakeError(this, desc);
}

// Other Device API methods

// Returns true if future ticking is needed.
bool DeviceBase::APITick() {
    // Tick may trigger callbacks which drop a ref to the device itself. Hold a Ref to ourselves
    // to avoid deleting |this| in the middle of this function call.
    Ref<DeviceBase> self(this);
    bool tickError;
    {
        // Note: we cannot hold the lock when flushing the callbacks so have to limit the scope of
        // the lock here.
        auto deviceLock(GetScopedLock());
        tickError = ConsumedError(Tick());
    }

    // We have to check callback tasks in every APITick because it is not related to any global
    // serials.
    FlushCallbackTaskQueue();

    if (tickError) {
        return false;
    }

    auto deviceLock(GetScopedLock());
    // We don't throw an error when device is lost. This allows pending callbacks to be
    // executed even after the Device is lost/destroyed.
    if (IsLost()) {
        return HasPendingTasks();
    }

    TRACE_EVENT1(GetPlatform(), General, "DeviceBase::APITick::IsDeviceIdle", "isDeviceIdle",
                 IsDeviceIdle());

    return !IsDeviceIdle();
}

MaybeError DeviceBase::Tick() {
    if (IsLost() || !HasScheduledCommands()) {
        return {};
    }

    // To avoid overly ticking, we only want to tick when:
    // 1. the last submitted serial has moved beyond the completed serial
    // 2. or the backend still has pending commands to submit.
    DAWN_TRY(CheckPassedSerials());
    DAWN_TRY(TickImpl());

    // TODO(crbug.com/dawn/833): decouple TickImpl from updating the serial so that we can
    // tick the dynamic uploader before the backend resource allocators. This would allow
    // reclaiming resources one tick earlier.
    mDynamicUploader->Deallocate(GetCompletedCommandSerial());
    mQueue->Tick(GetCompletedCommandSerial());

    return {};
}

AdapterBase* DeviceBase::APIGetAdapter() {
    mAdapter->Reference();
    return mAdapter.Get();
}

QueueBase* DeviceBase::APIGetQueue() {
    // Backends gave the primary queue during initialization.
    ASSERT(mQueue != nullptr);

    // Returns a new reference to the queue.
    mQueue->Reference();
    return mQueue.Get();
}

ExternalTextureBase* DeviceBase::APICreateExternalTexture(
    const ExternalTextureDescriptor* descriptor) {
    Ref<ExternalTextureBase> result = nullptr;
    if (ConsumedError(CreateExternalTextureImpl(descriptor), &result,
                      "calling %s.CreateExternalTexture(%s).", this, descriptor)) {
        return ExternalTextureBase::MakeError(this);
    }

    return result.Detach();
}

void DeviceBase::ApplyFeatures(const DeviceDescriptor* deviceDescriptor) {
    ASSERT(deviceDescriptor);
    // Validate all required features with device toggles.
    ASSERT(GetPhysicalDevice()->SupportsAllRequiredFeatures(
        {deviceDescriptor->requiredFeatures, deviceDescriptor->requiredFeaturesCount}, mToggles));

    for (uint32_t i = 0; i < deviceDescriptor->requiredFeaturesCount; ++i) {
        mEnabledFeatures.EnableFeature(deviceDescriptor->requiredFeatures[i]);
    }
}

bool DeviceBase::HasFeature(Feature feature) const {
    return mEnabledFeatures.IsEnabled(feature);
}

void DeviceBase::SetWGSLExtensionAllowList() {
    // Set the WGSL extensions allow list based on device's enabled features and other
    // properties.
    if (mEnabledFeatures.IsEnabled(Feature::ChromiumExperimentalDp4a)) {
        mWGSLExtensionAllowList.insert("chromium_experimental_dp4a");
    }
    if (mEnabledFeatures.IsEnabled(Feature::ShaderF16)) {
        mWGSLExtensionAllowList.insert("f16");
    }
    if (IsToggleEnabled(Toggle::AllowUnsafeAPIs)) {
        mWGSLExtensionAllowList.insert("chromium_disable_uniformity_analysis");
    }
}

WGSLExtensionSet DeviceBase::GetWGSLExtensionAllowList() const {
    return mWGSLExtensionAllowList;
}

bool DeviceBase::IsValidationEnabled() const {
    return !IsToggleEnabled(Toggle::SkipValidation);
}

bool DeviceBase::IsRobustnessEnabled() const {
    return !IsToggleEnabled(Toggle::DisableRobustness);
}

bool DeviceBase::IsCompatibilityMode() const {
    return mAdapter != nullptr && mAdapter->GetFeatureLevel() == FeatureLevel::Compatibility;
}

size_t DeviceBase::GetLazyClearCountForTesting() {
    return mLazyClearCountForTesting;
}

void DeviceBase::IncrementLazyClearCountForTesting() {
    ++mLazyClearCountForTesting;
}

size_t DeviceBase::GetDeprecationWarningCountForTesting() {
    return mDeprecationWarnings->count;
}

void DeviceBase::EmitDeprecationWarning(const std::string& message) {
    mDeprecationWarnings->count++;
    if (mDeprecationWarnings->emitted.insert(message).second) {
        dawn::WarningLog() << message;
    }
}

void DeviceBase::EmitWarningOnce(const std::string& message) {
    if (mWarnings.insert(message).second) {
        this->EmitLog(WGPULoggingType_Warning, message.c_str());
    }
}

void DeviceBase::EmitLog(const char* message) {
    this->EmitLog(WGPULoggingType_Info, message);
}

void DeviceBase::EmitLog(WGPULoggingType loggingType, const char* message) {
    if (mLoggingCallback != nullptr) {
        // Use the thread-safe CallbackTaskManager routine
        std::unique_ptr<LoggingCallbackTask> callbackTask = std::make_unique<LoggingCallbackTask>(
            mLoggingCallback, loggingType, message, mLoggingUserdata);
        mCallbackTaskManager->AddCallbackTask(std::move(callbackTask));
    }
}

bool DeviceBase::APIGetLimits(SupportedLimits* limits) const {
    ASSERT(limits != nullptr);
    if (limits->nextInChain != nullptr) {
        return false;
    }
    limits->limits = mLimits.v1;
    return true;
}

bool DeviceBase::APIHasFeature(wgpu::FeatureName feature) const {
    return mEnabledFeatures.IsEnabled(feature);
}

size_t DeviceBase::APIEnumerateFeatures(wgpu::FeatureName* features) const {
    return mEnabledFeatures.EnumerateFeatures(features);
}

void DeviceBase::APIInjectError(wgpu::ErrorType type, const char* message) {
    if (ConsumedError(ValidateErrorType(type))) {
        return;
    }

    // This method should only be used to make error scope reject. For DeviceLost there is the
    // LoseForTesting function that can be used instead.
    if (type != wgpu::ErrorType::Validation && type != wgpu::ErrorType::OutOfMemory) {
        HandleError(
            DAWN_VALIDATION_ERROR("Invalid injected error, must be Validation or OutOfMemory"));
        return;
    }

    HandleError(DAWN_MAKE_ERROR(FromWGPUErrorType(type), message), InternalErrorType::OutOfMemory);
}

void DeviceBase::APIValidateTextureDescriptor(const TextureDescriptor* desc) {
    DAWN_UNUSED(ConsumedError(ValidateTextureDescriptor(this, desc)));
}

QueueBase* DeviceBase::GetQueue() const {
    ASSERT(mQueue != nullptr);
    return mQueue.Get();
}

// Implementation details of object creation

ResultOrError<Ref<BindGroupBase>> DeviceBase::CreateBindGroup(const BindGroupDescriptor* descriptor,
                                                              UsageValidationMode mode) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateBindGroupDescriptor(this, descriptor, mode),
                         "validating %s against %s", descriptor, descriptor->layout);
    }
    return CreateBindGroupImpl(descriptor);
}

ResultOrError<Ref<BindGroupLayoutBase>> DeviceBase::CreateBindGroupLayout(
    const BindGroupLayoutDescriptor* descriptor,
    bool allowInternalBinding) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateBindGroupLayoutDescriptor(this, descriptor, allowInternalBinding),
                         "validating %s", descriptor);
    }
    return GetOrCreateBindGroupLayout(descriptor);
}

ResultOrError<Ref<BufferBase>> DeviceBase::CreateBuffer(const BufferDescriptor* descriptor) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY(ValidateBufferDescriptor(this, descriptor));
    }

    Ref<BufferBase> buffer;
    DAWN_TRY_ASSIGN(buffer, CreateBufferImpl(descriptor));

    if (descriptor->mappedAtCreation) {
        DAWN_TRY(buffer->MapAtCreation());
    }

    return std::move(buffer);
}

ResultOrError<Ref<ComputePipelineBase>> DeviceBase::CreateComputePipeline(
    const ComputePipelineDescriptor* descriptor) {
    Ref<ComputePipelineBase> uninitializedComputePipeline;
    DAWN_TRY_ASSIGN(uninitializedComputePipeline, CreateUninitializedComputePipeline(descriptor));

    Ref<ComputePipelineBase> cachedComputePipeline =
        GetCachedComputePipeline(uninitializedComputePipeline.Get());
    if (cachedComputePipeline.Get() != nullptr) {
        return cachedComputePipeline;
    }

    MaybeError maybeError;
    {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(GetPlatform(), "CreateComputePipelineUS");
        maybeError = uninitializedComputePipeline->Initialize();
    }
    DAWN_HISTOGRAM_BOOLEAN(GetPlatform(), "CreateComputePipelineSuccess", maybeError.IsSuccess());

    DAWN_TRY(std::move(maybeError));
    return AddOrGetCachedComputePipeline(std::move(uninitializedComputePipeline));
}

ResultOrError<Ref<CommandEncoder>> DeviceBase::CreateCommandEncoder(
    const CommandEncoderDescriptor* descriptor) {
    const CommandEncoderDescriptor defaultDescriptor = {};
    if (descriptor == nullptr) {
        descriptor = &defaultDescriptor;
    }

    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY(ValidateCommandEncoderDescriptor(this, descriptor));
    }
    return CommandEncoder::Create(this, descriptor);
}

// Overwritten on the backends to return pipeline caches if supported.
Ref<PipelineCacheBase> DeviceBase::GetOrCreatePipelineCacheImpl(const CacheKey& key) {
    UNREACHABLE();
}

ResultOrError<Ref<ComputePipelineBase>> DeviceBase::CreateUninitializedComputePipeline(
    const ComputePipelineDescriptor* descriptor) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY(ValidateComputePipelineDescriptor(this, descriptor));
    }

    Ref<PipelineLayoutBase> layoutRef;
    ComputePipelineDescriptor appliedDescriptor;
    DAWN_TRY_ASSIGN(layoutRef, ValidateLayoutAndGetComputePipelineDescriptorWithDefaults(
                                   this, *descriptor, &appliedDescriptor));

    return CreateUninitializedComputePipelineImpl(&appliedDescriptor);
}

// This function is overwritten with the async version on the backends that supports
// initializing compute pipelines asynchronously.
void DeviceBase::InitializeComputePipelineAsyncImpl(Ref<ComputePipelineBase> computePipeline,
                                                    WGPUCreateComputePipelineAsyncCallback callback,
                                                    void* userdata) {
    MaybeError maybeError;
    {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(GetPlatform(), "CreateComputePipelineUS");
        maybeError = computePipeline->Initialize();
    }
    DAWN_HISTOGRAM_BOOLEAN(GetPlatform(), "CreateComputePipelineSuccess", maybeError.IsSuccess());

    if (maybeError.IsError()) {
        AddComputePipelineAsyncCallbackTask(
            maybeError.AcquireError(), computePipeline->GetLabel().c_str(), callback, userdata);
    } else {
        AddComputePipelineAsyncCallbackTask(std::move(computePipeline), callback, userdata);
    }
}

// This function is overwritten with the async version on the backends
// that supports initializing render pipeline asynchronously
void DeviceBase::InitializeRenderPipelineAsyncImpl(Ref<RenderPipelineBase> renderPipeline,
                                                   WGPUCreateRenderPipelineAsyncCallback callback,
                                                   void* userdata) {
    MaybeError maybeError;
    {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(GetPlatform(), "CreateRenderPipelineUS");
        maybeError = renderPipeline->Initialize();
    }
    DAWN_HISTOGRAM_BOOLEAN(GetPlatform(), "CreateRenderPipelineSuccess", maybeError.IsSuccess());

    if (maybeError.IsError()) {
        AddRenderPipelineAsyncCallbackTask(maybeError.AcquireError(),
                                           renderPipeline->GetLabel().c_str(), callback, userdata);
    } else {
        AddRenderPipelineAsyncCallbackTask(std::move(renderPipeline), callback, userdata);
    }
}

ResultOrError<Ref<PipelineLayoutBase>> DeviceBase::CreatePipelineLayout(
    const PipelineLayoutDescriptor* descriptor) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY(ValidatePipelineLayoutDescriptor(this, descriptor));
    }
    return GetOrCreatePipelineLayout(descriptor);
}

ResultOrError<Ref<ExternalTextureBase>> DeviceBase::CreateExternalTextureImpl(
    const ExternalTextureDescriptor* descriptor) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateExternalTextureDescriptor(this, descriptor), "validating %s",
                         descriptor);
    }

    return ExternalTextureBase::Create(this, descriptor);
}

ResultOrError<Ref<QuerySetBase>> DeviceBase::CreateQuerySet(const QuerySetDescriptor* descriptor) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateQuerySetDescriptor(this, descriptor), "validating %s", descriptor);
    }
    return CreateQuerySetImpl(descriptor);
}

ResultOrError<Ref<RenderBundleEncoder>> DeviceBase::CreateRenderBundleEncoder(
    const RenderBundleEncoderDescriptor* descriptor) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateRenderBundleEncoderDescriptor(this, descriptor),
                         "validating render bundle encoder descriptor.");
    }
    return RenderBundleEncoder::Create(this, descriptor);
}

ResultOrError<Ref<RenderPipelineBase>> DeviceBase::CreateRenderPipeline(
    const RenderPipelineDescriptor* descriptor) {
    Ref<RenderPipelineBase> uninitializedRenderPipeline;
    DAWN_TRY_ASSIGN(uninitializedRenderPipeline, CreateUninitializedRenderPipeline(descriptor));

    Ref<RenderPipelineBase> cachedRenderPipeline =
        GetCachedRenderPipeline(uninitializedRenderPipeline.Get());
    if (cachedRenderPipeline != nullptr) {
        return cachedRenderPipeline;
    }

    MaybeError maybeError;
    {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(GetPlatform(), "CreateRenderPipelineUS");
        maybeError = uninitializedRenderPipeline->Initialize();
    }
    DAWN_HISTOGRAM_BOOLEAN(GetPlatform(), "CreateRenderPipelineSuccess", maybeError.IsSuccess());

    DAWN_TRY(std::move(maybeError));
    return AddOrGetCachedRenderPipeline(std::move(uninitializedRenderPipeline));
}

ResultOrError<Ref<RenderPipelineBase>> DeviceBase::CreateUninitializedRenderPipeline(
    const RenderPipelineDescriptor* descriptor) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY(ValidateRenderPipelineDescriptor(this, descriptor));

        // Validation for kMaxBindGroupsPlusVertexBuffers is skipped because it is not necessary so
        // far.
        static_assert(kMaxBindGroups + kMaxVertexBuffers <= kMaxBindGroupsPlusVertexBuffers);
    }

    // Ref will keep the pipeline layout alive until the end of the function where
    // the pipeline will take another reference.
    Ref<PipelineLayoutBase> layoutRef;
    RenderPipelineDescriptor appliedDescriptor;
    DAWN_TRY_ASSIGN(layoutRef, ValidateLayoutAndGetRenderPipelineDescriptorWithDefaults(
                                   this, *descriptor, &appliedDescriptor));

    return CreateUninitializedRenderPipelineImpl(&appliedDescriptor);
}

ResultOrError<Ref<SamplerBase>> DeviceBase::CreateSampler(const SamplerDescriptor* descriptor) {
    const SamplerDescriptor defaultDescriptor = {};
    DAWN_TRY(ValidateIsAlive());
    descriptor = descriptor != nullptr ? descriptor : &defaultDescriptor;
    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateSamplerDescriptor(this, descriptor), "validating %s", descriptor);
    }
    return GetOrCreateSampler(descriptor);
}

ResultOrError<Ref<ShaderModuleBase>> DeviceBase::CreateShaderModule(
    const ShaderModuleDescriptor* descriptor,
    OwnedCompilationMessages* compilationMessages) {
    DAWN_TRY(ValidateIsAlive());

    // CreateShaderModule can be called from inside dawn_native. If that's the case handle the
    // error directly in Dawn and no compilationMessages held in the shader module. It is ok as
    // long as dawn_native don't use the compilationMessages of these internal shader modules.
    ShaderModuleParseResult parseResult;

    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(
            ValidateAndParseShaderModule(this, descriptor, &parseResult, compilationMessages),
            "validating %s", descriptor);
    }

    return GetOrCreateShaderModule(descriptor, &parseResult, compilationMessages);
}

ResultOrError<Ref<SwapChainBase>> DeviceBase::CreateSwapChain(
    Surface* surface,
    const SwapChainDescriptor* descriptor) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateSwapChainDescriptor(this, surface, descriptor), "validating %s",
                         descriptor);
    }

    SwapChainBase* previousSwapChain = surface->GetAttachedSwapChain();
    ResultOrError<Ref<SwapChainBase>> maybeNewSwapChain =
        CreateSwapChainImpl(surface, previousSwapChain, descriptor);

    if (previousSwapChain != nullptr) {
        previousSwapChain->DetachFromSurface();
    }

    Ref<SwapChainBase> newSwapChain;
    DAWN_TRY_ASSIGN(newSwapChain, std::move(maybeNewSwapChain));

    newSwapChain->SetIsAttached();
    surface->SetAttachedSwapChain(newSwapChain.Get());
    return newSwapChain;
}

ResultOrError<Ref<TextureBase>> DeviceBase::CreateTexture(const TextureDescriptor* descriptor) {
    DAWN_TRY(ValidateIsAlive());
    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateTextureDescriptor(this, descriptor), "validating %s.", descriptor);
    }
    return CreateTextureImpl(descriptor);
}

ResultOrError<Ref<TextureViewBase>> DeviceBase::CreateTextureView(
    TextureBase* texture,
    const TextureViewDescriptor* descriptor) {
    DAWN_TRY(ValidateIsAlive());
    DAWN_TRY(ValidateObject(texture));

    TextureViewDescriptor desc;
    DAWN_TRY_ASSIGN(desc, GetTextureViewDescriptorWithDefaults(texture, descriptor));

    if (IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateTextureViewDescriptor(this, texture, &desc),
                         "validating %s against %s.", &desc, texture);
    }
    return CreateTextureViewImpl(texture, &desc);
}

ResultOrError<wgpu::TextureUsage> DeviceBase::GetSupportedSurfaceUsage(
    const Surface* surface) const {
    DAWN_TRY(ValidateIsAlive());

    if (IsValidationEnabled()) {
        DAWN_INVALID_IF(!HasFeature(Feature::SurfaceCapabilities), "%s is not enabled.",
                        wgpu::FeatureName::SurfaceCapabilities);
    }

    return GetSupportedSurfaceUsageImpl(surface);
}

// Other implementation details

DynamicUploader* DeviceBase::GetDynamicUploader() const {
    return mDynamicUploader.get();
}

// The Toggle device facility

std::vector<const char*> DeviceBase::GetTogglesUsed() const {
    return mToggles.GetEnabledToggleNames();
}

bool DeviceBase::IsToggleEnabled(Toggle toggle) const {
    return mToggles.IsEnabled(toggle);
}

void DeviceBase::ForceSetToggleForTesting(Toggle toggle, bool isEnabled) {
    mToggles.ForceSet(toggle, isEnabled);
}

void DeviceBase::FlushCallbackTaskQueue() {
    // Callbacks might cause re-entrances. Mutex shouldn't be locked. So we expect there is no
    // locked mutex before entering this method.
    ASSERT(mMutex == nullptr || !mMutex->IsLockedByCurrentThread());

    Ref<CallbackTaskManager> callbackTaskManager;

    {
        // This is a data race with the assignment to InstanceBase's callback queue manager in
        // WillDropLastExternalRef(). Need to protect with a lock and keep the old
        // mCallbackTaskManager alive.
        // TODO(crbug.com/dawn/752): In future, all devices should use InstanceBase's callback queue
        // manager from the start. So we won't need to care about data race at that point.
        auto deviceLock(GetScopedLock());
        callbackTaskManager = mCallbackTaskManager;
    }

    callbackTaskManager->Flush();
}

const CombinedLimits& DeviceBase::GetLimits() const {
    return mLimits;
}

AsyncTaskManager* DeviceBase::GetAsyncTaskManager() const {
    return mAsyncTaskManager.get();
}

CallbackTaskManager* DeviceBase::GetCallbackTaskManager() const {
    return mCallbackTaskManager.Get();
}

dawn::platform::WorkerTaskPool* DeviceBase::GetWorkerTaskPool() const {
    return mWorkerTaskPool.get();
}

void DeviceBase::AddComputePipelineAsyncCallbackTask(
    std::unique_ptr<ErrorData> error,
    const char* label,
    WGPUCreateComputePipelineAsyncCallback callback,
    void* userdata) {
    if (GetState() != State::Alive) {
        // If the device is no longer alive, errors should not be reported anymore.
        // Call the callback with an error pipeline.
        return mCallbackTaskManager->AddCallbackTask(
            [callback, pipeline = ComputePipelineBase::MakeError(this, label), userdata]() {
                callback(WGPUCreatePipelineAsyncStatus_Success, ToAPI(pipeline), "", userdata);
            });
    }

    WGPUCreatePipelineAsyncStatus status = CreatePipelineAsyncStatusFromErrorType(error->GetType());
    mCallbackTaskManager->AddCallbackTask(
        [callback, message = error->GetFormattedMessage(), status, userdata]() {
            callback(status, nullptr, message.c_str(), userdata);
        });
}

void DeviceBase::AddComputePipelineAsyncCallbackTask(
    Ref<ComputePipelineBase> pipeline,
    WGPUCreateComputePipelineAsyncCallback callback,
    void* userdata) {
    mCallbackTaskManager->AddCallbackTask(
        [callback, pipeline = std::move(pipeline), userdata]() mutable {
            // TODO(dawn:529): call AddOrGetCachedComputePipeline() asynchronously in
            // CreateComputePipelineAsyncTaskImpl::Run() when the front-end pipeline cache is
            // thread-safe.
            ASSERT(pipeline != nullptr);
            {
                // This is called inside a callback, and no lock will be held by default so we
                // have to lock now to protect the cache. Note: we don't lock inside
                // AddOrGetCachedComputePipeline() to avoid deadlock because many places calling
                // that method might already have the lock held. For example,
                // APICreateComputePipeline()
                auto deviceLock(pipeline->GetDevice()->GetScopedLock());
                if (pipeline->GetDevice()->GetState() == State::Alive) {
                    pipeline =
                        pipeline->GetDevice()->AddOrGetCachedComputePipeline(std::move(pipeline));
                }
            }
            callback(WGPUCreatePipelineAsyncStatus_Success, ToAPI(pipeline.Detach()), "", userdata);
        });
}

void DeviceBase::AddRenderPipelineAsyncCallbackTask(std::unique_ptr<ErrorData> error,
                                                    const char* label,
                                                    WGPUCreateRenderPipelineAsyncCallback callback,
                                                    void* userdata) {
    if (GetState() != State::Alive) {
        // If the device is no longer alive, errors should not be reported anymore.
        // Call the callback with an error pipeline.
        return mCallbackTaskManager->AddCallbackTask(
            [callback, pipeline = RenderPipelineBase::MakeError(this, label), userdata]() {
                callback(WGPUCreatePipelineAsyncStatus_Success, ToAPI(pipeline), "", userdata);
            });
    }

    WGPUCreatePipelineAsyncStatus status = CreatePipelineAsyncStatusFromErrorType(error->GetType());
    mCallbackTaskManager->AddCallbackTask(
        [callback, message = error->GetFormattedMessage(), status, userdata]() {
            callback(status, nullptr, message.c_str(), userdata);
        });
}

void DeviceBase::AddRenderPipelineAsyncCallbackTask(Ref<RenderPipelineBase> pipeline,
                                                    WGPUCreateRenderPipelineAsyncCallback callback,
                                                    void* userdata) {
    mCallbackTaskManager->AddCallbackTask([callback, pipeline = std::move(pipeline),
                                           userdata]() mutable {
        // TODO(dawn:529): call AddOrGetCachedRenderPipeline() asynchronously in
        // CreateRenderPipelineAsyncTaskImpl::Run() when the front-end pipeline cache is
        // thread-safe.
        ASSERT(pipeline != nullptr);
        {
            // This is called inside a callback, and no lock will be held by default so we have
            // to lock now to protect the cache.
            // Note: we don't lock inside AddOrGetCachedRenderPipeline() to avoid deadlock
            // because many places calling that method might already have the lock held. For
            // example, APICreateRenderPipeline()
            auto deviceLock(pipeline->GetDevice()->GetScopedLock());
            if (pipeline->GetDevice()->GetState() == State::Alive) {
                pipeline = pipeline->GetDevice()->AddOrGetCachedRenderPipeline(std::move(pipeline));
            }
        }
        callback(WGPUCreatePipelineAsyncStatus_Success, ToAPI(pipeline.Detach()), "", userdata);
    });
}

PipelineCompatibilityToken DeviceBase::GetNextPipelineCompatibilityToken() {
    return PipelineCompatibilityToken(mNextPipelineCompatibilityToken++);
}

const CacheKey& DeviceBase::GetCacheKey() const {
    return mDeviceCacheKey;
}

const std::string& DeviceBase::GetLabel() const {
    return mLabel;
}

void DeviceBase::APISetLabel(const char* label) {
    mLabel = label;
    SetLabelImpl();
}

void DeviceBase::SetLabelImpl() {}

bool DeviceBase::ShouldDuplicateNumWorkgroupsForDispatchIndirect(
    ComputePipelineBase* computePipeline) const {
    return false;
}

bool DeviceBase::MayRequireDuplicationOfIndirectParameters() const {
    return false;
}

bool DeviceBase::ShouldDuplicateParametersForDrawIndirect(
    const RenderPipelineBase* renderPipelineBase) const {
    return false;
}

bool DeviceBase::IsResolveTextureBlitWithDrawSupported() const {
    return false;
}

uint64_t DeviceBase::GetBufferCopyOffsetAlignmentForDepthStencil() const {
    // For depth-stencil texture, buffer offset must be a multiple of 4, which is required
    // by WebGPU and Vulkan SPEC.
    return 4u;
}

MaybeError DeviceBase::CopyFromStagingToBuffer(BufferBase* source,
                                               uint64_t sourceOffset,
                                               BufferBase* destination,
                                               uint64_t destinationOffset,
                                               uint64_t size) {
    DAWN_TRY(
        CopyFromStagingToBufferImpl(source, sourceOffset, destination, destinationOffset, size));
    if (GetDynamicUploader()->ShouldFlush()) {
        ForceEventualFlushOfCommands();
    }
    return {};
}

MaybeError DeviceBase::CopyFromStagingToTexture(BufferBase* source,
                                                const TextureDataLayout& src,
                                                const TextureCopy& dst,
                                                const Extent3D& copySizePixels) {
    if (dst.aspect == Aspect::Depth &&
        IsToggleEnabled(Toggle::UseBlitForBufferToDepthTextureCopy)) {
        DAWN_TRY_CONTEXT(BlitStagingBufferToDepth(this, source, src, dst, copySizePixels),
                         "copying from staging buffer to depth aspect of %s using blit workaround.",
                         dst.texture.Get());
    } else if (dst.aspect == Aspect::Stencil &&
               IsToggleEnabled(Toggle::UseBlitForBufferToStencilTextureCopy)) {
        DAWN_TRY_CONTEXT(
            BlitStagingBufferToStencil(this, source, src, dst, copySizePixels),
            "copying from staging buffer to stencil aspect of %s using blit workaround.",
            dst.texture.Get());
    } else {
        DAWN_TRY(CopyFromStagingToTextureImpl(source, src, dst, copySizePixels));
    }

    if (GetDynamicUploader()->ShouldFlush()) {
        ForceEventualFlushOfCommands();
    }
    return {};
}

Mutex::AutoLockAndHoldRef DeviceBase::GetScopedLockSafeForDelete() {
    return Mutex::AutoLockAndHoldRef(mMutex);
}

Mutex::AutoLock DeviceBase::GetScopedLock() {
    return Mutex::AutoLock(mMutex.Get());
}

bool DeviceBase::IsLockedByCurrentThreadIfNeeded() const {
    return mMutex == nullptr || mMutex->IsLockedByCurrentThread();
}

IgnoreLazyClearCountScope::IgnoreLazyClearCountScope(DeviceBase* device)
    : mDevice(device), mLazyClearCountForTesting(device->mLazyClearCountForTesting) {}

IgnoreLazyClearCountScope::~IgnoreLazyClearCountScope() {
    mDevice->mLazyClearCountForTesting = mLazyClearCountForTesting;
}

}  // namespace dawn::native
