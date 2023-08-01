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

#ifndef SRC_DAWN_NATIVE_DEVICE_H_
#define SRC_DAWN_NATIVE_DEVICE_H_

#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "dawn/common/ContentLessObjectCache.h"
#include "dawn/common/Mutex.h"
#include "dawn/native/CacheKey.h"
#include "dawn/native/Commands.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/Error.h"
#include "dawn/native/ExecutionQueue.h"
#include "dawn/native/Features.h"
#include "dawn/native/Format.h"
#include "dawn/native/Forward.h"
#include "dawn/native/Limits.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/RefCountedWithExternalCount.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/UsageValidationMode.h"

#include "dawn/native/DawnNative.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::platform {
class WorkerTaskPool;
}  // namespace dawn::platform

namespace dawn::native {
class AsyncTaskManager;
class AttachmentState;
class AttachmentStateBlueprint;
class Blob;
class BlobCache;
class CallbackTaskManager;
class DynamicUploader;
class ErrorScopeStack;
class OwnedCompilationMessages;
struct CallbackTask;
struct InternalPipelineStore;
struct ShaderModuleParseResult;

using WGSLExtensionSet = std::unordered_set<std::string>;

class DeviceBase : public RefCountedWithExternalCount, public ExecutionQueueBase {
  public:
    DeviceBase(AdapterBase* adapter,
               const DeviceDescriptor* descriptor,
               const TogglesState& deviceToggles);
    ~DeviceBase() override;

    // Handles the error, causing a device loss if applicable. Almost always when a device loss
    // occurs because of an error, we want to call the device loss callback with an undefined
    // reason, but the ForceLoss API allows for an injection of the reason, hence the default
    // argument. The `additionalAllowedErrors` mask allows specifying additional errors are allowed
    // (on top of validation and device loss errors). Note that "allowed" is defined as surfacing to
    // users as the respective error rather than causing a device loss instead.
    void HandleError(std::unique_ptr<ErrorData> error,
                     InternalErrorType additionalAllowedErrors = InternalErrorType::None,
                     WGPUDeviceLostReason lost_reason = WGPUDeviceLostReason_Undefined);

    // Variants of ConsumedError must use the returned boolean to handle failure cases since an
    // error may cause a device loss and further execution may be undefined. This is especially
    // true for the ResultOrError variants.
    [[nodiscard]] bool ConsumedError(
        MaybeError maybeError,
        InternalErrorType additionalAllowedErrors = InternalErrorType::None) {
        if (DAWN_UNLIKELY(maybeError.IsError())) {
            ConsumeError(maybeError.AcquireError(), additionalAllowedErrors);
            return true;
        }
        return false;
    }

    template <typename T>
    [[nodiscard]] bool ConsumedError(
        ResultOrError<T> resultOrError,
        T* result,
        InternalErrorType additionalAllowedErrors = InternalErrorType::None) {
        if (DAWN_UNLIKELY(resultOrError.IsError())) {
            ConsumeError(resultOrError.AcquireError(), additionalAllowedErrors);
            return true;
        }
        *result = resultOrError.AcquireSuccess();
        return false;
    }

    template <typename... Args>
    [[nodiscard]] bool ConsumedError(MaybeError maybeError,
                                     InternalErrorType additionalAllowedErrors,
                                     const char* formatStr,
                                     const Args&... args) {
        if (DAWN_UNLIKELY(maybeError.IsError())) {
            std::unique_ptr<ErrorData> error = maybeError.AcquireError();
            if (error->GetType() == InternalErrorType::Validation) {
                error->AppendContext(formatStr, args...);
            }
            ConsumeError(std::move(error), additionalAllowedErrors);
            return true;
        }
        return false;
    }

    template <typename... Args>
    [[nodiscard]] bool ConsumedError(MaybeError maybeError,
                                     const char* formatStr,
                                     const Args&... args) {
        return ConsumedError(std::move(maybeError), InternalErrorType::None, formatStr, args...);
    }

    template <typename T, typename... Args>
    [[nodiscard]] bool ConsumedError(ResultOrError<T> resultOrError,
                                     T* result,
                                     InternalErrorType additionalAllowedErrors,
                                     const char* formatStr,
                                     const Args&... args) {
        if (DAWN_UNLIKELY(resultOrError.IsError())) {
            std::unique_ptr<ErrorData> error = resultOrError.AcquireError();
            if (error->GetType() == InternalErrorType::Validation) {
                error->AppendContext(formatStr, args...);
            }
            ConsumeError(std::move(error), additionalAllowedErrors);
            return true;
        }
        *result = resultOrError.AcquireSuccess();
        return false;
    }

    template <typename T, typename... Args>
    [[nodiscard]] bool ConsumedError(ResultOrError<T> resultOrError,
                                     T* result,
                                     const char* formatStr,
                                     const Args&... args) {
        return ConsumedError(std::move(resultOrError), result, InternalErrorType::None, formatStr,
                             args...);
    }

    MaybeError ValidateObject(const ApiObjectBase* object) const;

    AdapterBase* GetAdapter() const;
    PhysicalDeviceBase* GetPhysicalDevice() const;
    virtual dawn::platform::Platform* GetPlatform() const;

    // Returns the Format corresponding to the wgpu::TextureFormat or an error if the format
    // isn't a valid wgpu::TextureFormat or isn't supported by this device.
    // The pointer returned has the same lifetime as the device.
    ResultOrError<const Format*> GetInternalFormat(wgpu::TextureFormat format) const;

    // Returns the Format corresponding to the wgpu::TextureFormat and assumes the format is
    // valid and supported.
    // The reference returned has the same lifetime as the device.
    const Format& GetValidInternalFormat(wgpu::TextureFormat format) const;
    const Format& GetValidInternalFormat(FormatIndex formatIndex) const;

    virtual ResultOrError<Ref<CommandBufferBase>> CreateCommandBuffer(
        CommandEncoder* encoder,
        const CommandBufferDescriptor* descriptor) = 0;

    // Many Dawn objects are completely immutable once created which means that if two
    // creations are given the same arguments, they can return the same object. Reusing
    // objects will help make comparisons between objects by a single pointer comparison.
    //
    // Technically no object is immutable as they have a reference count, and an
    // application with reference-counting issues could "see" that objects are reused.
    // This is solved by automatic-reference counting, and also the fact that when using
    // the client-server wire every creation will get a different proxy object, with a
    // different reference count.
    //
    // When trying to create an object, we give both the descriptor and an example of what
    // the created object will be, the "blueprint". The blueprint is just a FooBase object
    // instead of a backend Foo object. If the blueprint doesn't match an object in the
    // cache, then the descriptor is used to make a new object.
    ResultOrError<Ref<BindGroupLayoutBase>> GetOrCreateBindGroupLayout(
        const BindGroupLayoutDescriptor* descriptor,
        PipelineCompatibilityToken pipelineCompatibilityToken = PipelineCompatibilityToken(0));

    BindGroupLayoutBase* GetEmptyBindGroupLayout();
    PipelineLayoutBase* GetEmptyPipelineLayout();

    ResultOrError<Ref<TextureViewBase>> CreateImplicitMSAARenderTextureViewFor(
        const TextureBase* singleSampledTexture,
        uint32_t sampleCount);

    ResultOrError<Ref<TextureViewBase>> GetOrCreatePlaceholderTextureViewForExternalTexture();

    ResultOrError<Ref<PipelineLayoutBase>> GetOrCreatePipelineLayout(
        const PipelineLayoutDescriptor* descriptor);

    ResultOrError<Ref<SamplerBase>> GetOrCreateSampler(const SamplerDescriptor* descriptor);

    ResultOrError<Ref<ShaderModuleBase>> GetOrCreateShaderModule(
        const ShaderModuleDescriptor* descriptor,
        ShaderModuleParseResult* parseResult,
        OwnedCompilationMessages* compilationMessages);

    Ref<AttachmentState> GetOrCreateAttachmentState(AttachmentState* blueprint);
    Ref<AttachmentState> GetOrCreateAttachmentState(
        const RenderBundleEncoderDescriptor* descriptor);
    Ref<AttachmentState> GetOrCreateAttachmentState(const RenderPipelineDescriptor* descriptor);
    Ref<AttachmentState> GetOrCreateAttachmentState(const RenderPassDescriptor* descriptor);

    Ref<PipelineCacheBase> GetOrCreatePipelineCache(const CacheKey& key);

    // Object creation methods that be used in a reentrant manner.
    ResultOrError<Ref<BindGroupBase>> CreateBindGroup(
        const BindGroupDescriptor* descriptor,
        UsageValidationMode mode = UsageValidationMode::Default);
    ResultOrError<Ref<BindGroupLayoutBase>> CreateBindGroupLayout(
        const BindGroupLayoutDescriptor* descriptor,
        bool allowInternalBinding = false);
    ResultOrError<Ref<BufferBase>> CreateBuffer(const BufferDescriptor* descriptor);
    ResultOrError<Ref<CommandEncoder>> CreateCommandEncoder(
        const CommandEncoderDescriptor* descriptor = nullptr);
    ResultOrError<Ref<ComputePipelineBase>> CreateComputePipeline(
        const ComputePipelineDescriptor* descriptor);
    ResultOrError<Ref<ComputePipelineBase>> CreateUninitializedComputePipeline(
        const ComputePipelineDescriptor* descriptor);
    ResultOrError<Ref<PipelineLayoutBase>> CreatePipelineLayout(
        const PipelineLayoutDescriptor* descriptor);
    ResultOrError<Ref<QuerySetBase>> CreateQuerySet(const QuerySetDescriptor* descriptor);
    ResultOrError<Ref<RenderBundleEncoder>> CreateRenderBundleEncoder(
        const RenderBundleEncoderDescriptor* descriptor);
    ResultOrError<Ref<RenderPipelineBase>> CreateRenderPipeline(
        const RenderPipelineDescriptor* descriptor);
    ResultOrError<Ref<RenderPipelineBase>> CreateUninitializedRenderPipeline(
        const RenderPipelineDescriptor* descriptor);
    ResultOrError<Ref<SamplerBase>> CreateSampler(const SamplerDescriptor* descriptor = nullptr);
    ResultOrError<Ref<ShaderModuleBase>> CreateShaderModule(
        const ShaderModuleDescriptor* descriptor,
        OwnedCompilationMessages* compilationMessages = nullptr);
    ResultOrError<Ref<SwapChainBase>> CreateSwapChain(Surface* surface,
                                                      const SwapChainDescriptor* descriptor);
    ResultOrError<Ref<TextureBase>> CreateTexture(const TextureDescriptor* descriptor);
    ResultOrError<Ref<TextureViewBase>> CreateTextureView(TextureBase* texture,
                                                          const TextureViewDescriptor* descriptor);

    ResultOrError<wgpu::TextureUsage> GetSupportedSurfaceUsage(const Surface* surface) const;

    // Implementation of API object creation methods. DO NOT use them in a reentrant manner.
    BindGroupBase* APICreateBindGroup(const BindGroupDescriptor* descriptor);
    BindGroupLayoutBase* APICreateBindGroupLayout(const BindGroupLayoutDescriptor* descriptor);
    BufferBase* APICreateBuffer(const BufferDescriptor* descriptor);
    CommandEncoder* APICreateCommandEncoder(const CommandEncoderDescriptor* descriptor);
    ComputePipelineBase* APICreateComputePipeline(const ComputePipelineDescriptor* descriptor);
    PipelineLayoutBase* APICreatePipelineLayout(const PipelineLayoutDescriptor* descriptor);
    QuerySetBase* APICreateQuerySet(const QuerySetDescriptor* descriptor);
    void APICreateComputePipelineAsync(const ComputePipelineDescriptor* descriptor,
                                       WGPUCreateComputePipelineAsyncCallback callback,
                                       void* userdata);
    void APICreateRenderPipelineAsync(const RenderPipelineDescriptor* descriptor,
                                      WGPUCreateRenderPipelineAsyncCallback callback,
                                      void* userdata);
    RenderBundleEncoder* APICreateRenderBundleEncoder(
        const RenderBundleEncoderDescriptor* descriptor);
    RenderPipelineBase* APICreateRenderPipeline(const RenderPipelineDescriptor* descriptor);
    ExternalTextureBase* APICreateExternalTexture(const ExternalTextureDescriptor* descriptor);
    SamplerBase* APICreateSampler(const SamplerDescriptor* descriptor);
    ShaderModuleBase* APICreateShaderModule(const ShaderModuleDescriptor* descriptor);
    ShaderModuleBase* APICreateErrorShaderModule(const ShaderModuleDescriptor* descriptor,
                                                 const char* errorMessage);
    SwapChainBase* APICreateSwapChain(Surface* surface, const SwapChainDescriptor* descriptor);
    TextureBase* APICreateTexture(const TextureDescriptor* descriptor);

    wgpu::TextureUsage APIGetSupportedSurfaceUsage(Surface* surface);

    InternalPipelineStore* GetInternalPipelineStore();

    // For Dawn Wire
    BufferBase* APICreateErrorBuffer(const BufferDescriptor* desc);
    ExternalTextureBase* APICreateErrorExternalTexture();
    TextureBase* APICreateErrorTexture(const TextureDescriptor* desc);

    AdapterBase* APIGetAdapter();
    QueueBase* APIGetQueue();

    bool APIGetLimits(SupportedLimits* limits) const;
    bool APIHasFeature(wgpu::FeatureName feature) const;
    size_t APIEnumerateFeatures(wgpu::FeatureName* features) const;
    void APIInjectError(wgpu::ErrorType type, const char* message);
    bool APITick();
    void APIValidateTextureDescriptor(const TextureDescriptor* desc);

    void APISetDeviceLostCallback(wgpu::DeviceLostCallback callback, void* userdata);
    void APISetUncapturedErrorCallback(wgpu::ErrorCallback callback, void* userdata);
    void APISetLoggingCallback(wgpu::LoggingCallback callback, void* userdata);
    void APIPushErrorScope(wgpu::ErrorFilter filter);
    void APIPopErrorScope(wgpu::ErrorCallback callback, void* userdata);

    MaybeError ValidateIsAlive() const;

    BlobCache* GetBlobCache();
    Blob LoadCachedBlob(const CacheKey& key);
    void StoreCachedBlob(const CacheKey& key, const Blob& blob);

    MaybeError CopyFromStagingToBuffer(BufferBase* source,
                                       uint64_t sourceOffset,
                                       BufferBase* destination,
                                       uint64_t destinationOffset,
                                       uint64_t size);
    MaybeError CopyFromStagingToTexture(BufferBase* source,
                                        const TextureDataLayout& src,
                                        const TextureCopy& dst,
                                        const Extent3D& copySizePixels);

    DynamicUploader* GetDynamicUploader() const;

    // The device state which is a combination of creation state and loss state.
    //
    //   - BeingCreated: the device didn't finish creation yet and the frontend cannot be used
    //     (both for the application calling WebGPU, or re-entrant calls). No work exists on
    //     the GPU timeline.
    //   - Alive: the device is usable and might have work happening on the GPU timeline.
    //   - BeingDisconnected: the device is no longer usable because we are waiting for all
    //     work on the GPU timeline to finish. (this is to make validation prevent the
    //     application from adding more work during the transition from Available to
    //     Disconnected)
    //   - Disconnected: there is no longer work happening on the GPU timeline and the CPU data
    //     structures can be safely destroyed without additional synchronization.
    //   - Destroyed: the device is disconnected and resources have been reclaimed.
    enum class State {
        BeingCreated,
        Alive,
        BeingDisconnected,
        Disconnected,
        Destroyed,
    };
    State GetState() const;
    bool IsLost() const;
    ApiObjectList* GetObjectTrackingList(ObjectType type);

    std::vector<const char*> GetTogglesUsed() const;
    WGSLExtensionSet GetWGSLExtensionAllowList() const;
    bool IsToggleEnabled(Toggle toggle) const;
    bool IsValidationEnabled() const;
    bool IsRobustnessEnabled() const;
    bool IsCompatibilityMode() const;

    size_t GetLazyClearCountForTesting();
    void IncrementLazyClearCountForTesting();
    size_t GetDeprecationWarningCountForTesting();
    void EmitDeprecationWarning(const std::string& warning);
    void EmitWarningOnce(const std::string& message);
    void EmitLog(const char* message);
    void EmitLog(WGPULoggingType loggingType, const char* message);
    void APIForceLoss(wgpu::DeviceLostReason reason, const char* message);
    QueueBase* GetQueue() const;

    friend class IgnoreLazyClearCountScope;

    MaybeError Tick();

    // TODO(crbug.com/dawn/839): Organize the below backend-specific parameters into the struct
    // BackendMetadata that we can query from the device.
    virtual uint32_t GetOptimalBytesPerRowAlignment() const = 0;
    virtual uint64_t GetOptimalBufferToTextureCopyOffsetAlignment() const = 0;
    virtual uint64_t GetBufferCopyOffsetAlignmentForDepthStencil() const;

    virtual float GetTimestampPeriodInNS() const = 0;

    virtual bool ShouldDuplicateNumWorkgroupsForDispatchIndirect(
        ComputePipelineBase* computePipeline) const;

    virtual bool MayRequireDuplicationOfIndirectParameters() const;

    virtual bool ShouldDuplicateParametersForDrawIndirect(
        const RenderPipelineBase* renderPipelineBase) const;

    // Whether the backend supports blitting the resolve texture with draw calls in the same render
    // pass that it will be resolved into.
    virtual bool IsResolveTextureBlitWithDrawSupported() const;

    bool HasFeature(Feature feature) const;

    const CombinedLimits& GetLimits() const;

    AsyncTaskManager* GetAsyncTaskManager() const;
    CallbackTaskManager* GetCallbackTaskManager() const;
    dawn::platform::WorkerTaskPool* GetWorkerTaskPool() const;

    // Enqueue a successfully-create async pipeline creation callback.
    void AddComputePipelineAsyncCallbackTask(Ref<ComputePipelineBase> pipeline,
                                             WGPUCreateComputePipelineAsyncCallback callback,
                                             void* userdata);
    void AddRenderPipelineAsyncCallbackTask(Ref<RenderPipelineBase> pipeline,
                                            WGPUCreateRenderPipelineAsyncCallback callback,
                                            void* userdata);
    // Enqueue a failed async pipeline creation callback.
    // If the device is lost, then further errors should not be reported to
    // the application. Instead of an error, a successful callback is enqueued, using
    // an error pipeline created with `label`.
    void AddComputePipelineAsyncCallbackTask(std::unique_ptr<ErrorData> error,
                                             const char* label,
                                             WGPUCreateComputePipelineAsyncCallback callback,
                                             void* userdata);
    void AddRenderPipelineAsyncCallbackTask(std::unique_ptr<ErrorData> error,
                                            const char* label,
                                            WGPUCreateRenderPipelineAsyncCallback callback,
                                            void* userdata);

    PipelineCompatibilityToken GetNextPipelineCompatibilityToken();

    const CacheKey& GetCacheKey() const;
    const std::string& GetLabel() const;
    void APISetLabel(const char* label);
    void APIDestroy();

    virtual void AppendDebugLayerMessages(ErrorData* error) {}

    // It is guaranteed that the wrapped mutex will outlive the Device (if the Device is deleted
    // before the AutoLockAndHoldRef).
    [[nodiscard]] Mutex::AutoLockAndHoldRef GetScopedLockSafeForDelete();
    // This lock won't guarantee the wrapped mutex will be alive if the Device is deleted before the
    // AutoLock. It would crash if such thing happens.
    [[nodiscard]] Mutex::AutoLock GetScopedLock();

    // This method returns true if Feature::ImplicitDeviceSynchronization is turned on and the
    // device is locked by current thread. This method is only enabled when DAWN_ENABLE_ASSERTS is
    // turned on. Thus it should only be wrapped inside ASSERT() macro. i.e.
    // ASSERT(device.IsLockedByCurrentThread())
    bool IsLockedByCurrentThreadIfNeeded() const;

    // In the 'Normal' mode, currently recorded commands in the backend normally will be actually
    // submitted in the next Tick. However in the 'Passive' mode, the submission will be postponed
    // as late as possible, for example, until the client has explictly issued a submission.
    enum class SubmitMode { Normal, Passive };

  protected:
    // Constructor used only for mocking and testing.
    DeviceBase();

    void ForceSetToggleForTesting(Toggle toggle, bool isEnabled);

    MaybeError Initialize(Ref<QueueBase> defaultQueue);
    void DestroyObjects();
    void Destroy();

  private:
    void WillDropLastExternalRef() override;

    virtual ResultOrError<Ref<BindGroupBase>> CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) = 0;
    virtual ResultOrError<Ref<BindGroupLayoutInternalBase>> CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) = 0;
    virtual ResultOrError<Ref<BufferBase>> CreateBufferImpl(const BufferDescriptor* descriptor) = 0;
    virtual ResultOrError<Ref<ExternalTextureBase>> CreateExternalTextureImpl(
        const ExternalTextureDescriptor* descriptor);
    virtual ResultOrError<Ref<PipelineLayoutBase>> CreatePipelineLayoutImpl(
        const PipelineLayoutDescriptor* descriptor) = 0;
    virtual ResultOrError<Ref<QuerySetBase>> CreateQuerySetImpl(
        const QuerySetDescriptor* descriptor) = 0;
    virtual ResultOrError<Ref<SamplerBase>> CreateSamplerImpl(
        const SamplerDescriptor* descriptor) = 0;
    virtual ResultOrError<Ref<ShaderModuleBase>> CreateShaderModuleImpl(
        const ShaderModuleDescriptor* descriptor,
        ShaderModuleParseResult* parseResult,
        OwnedCompilationMessages* compilationMessages) = 0;
    // Note that previousSwapChain may be nullptr, or come from a different backend.
    virtual ResultOrError<Ref<SwapChainBase>> CreateSwapChainImpl(
        Surface* surface,
        SwapChainBase* previousSwapChain,
        const SwapChainDescriptor* descriptor) = 0;
    virtual ResultOrError<Ref<TextureBase>> CreateTextureImpl(
        const TextureDescriptor* descriptor) = 0;
    virtual ResultOrError<Ref<TextureViewBase>> CreateTextureViewImpl(
        TextureBase* texture,
        const TextureViewDescriptor* descriptor) = 0;
    virtual Ref<ComputePipelineBase> CreateUninitializedComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) = 0;
    virtual Ref<RenderPipelineBase> CreateUninitializedRenderPipelineImpl(
        const RenderPipelineDescriptor* descriptor) = 0;
    virtual void SetLabelImpl();

    virtual ResultOrError<wgpu::TextureUsage> GetSupportedSurfaceUsageImpl(
        const Surface* surface) const = 0;

    virtual MaybeError TickImpl() = 0;
    void FlushCallbackTaskQueue();

    ResultOrError<Ref<BindGroupLayoutBase>> CreateEmptyBindGroupLayout();
    ResultOrError<Ref<PipelineLayoutBase>> CreateEmptyPipelineLayout();

    Ref<ComputePipelineBase> GetCachedComputePipeline(
        ComputePipelineBase* uninitializedComputePipeline);
    Ref<RenderPipelineBase> GetCachedRenderPipeline(
        RenderPipelineBase* uninitializedRenderPipeline);
    Ref<ComputePipelineBase> AddOrGetCachedComputePipeline(
        Ref<ComputePipelineBase> computePipeline);
    Ref<RenderPipelineBase> AddOrGetCachedRenderPipeline(Ref<RenderPipelineBase> renderPipeline);
    virtual Ref<PipelineCacheBase> GetOrCreatePipelineCacheImpl(const CacheKey& key);
    virtual void InitializeComputePipelineAsyncImpl(Ref<ComputePipelineBase> computePipeline,
                                                    WGPUCreateComputePipelineAsyncCallback callback,
                                                    void* userdata);
    virtual void InitializeRenderPipelineAsyncImpl(Ref<RenderPipelineBase> renderPipeline,
                                                   WGPUCreateRenderPipelineAsyncCallback callback,
                                                   void* userdata);

    void ApplyFeatures(const DeviceDescriptor* deviceDescriptor);

    void SetWGSLExtensionAllowList();

    void ConsumeError(std::unique_ptr<ErrorData> error,
                      InternalErrorType additionalAllowedErrors = InternalErrorType::None);

    bool HasPendingTasks();
    bool IsDeviceIdle();

    // DestroyImpl is used to clean up and release resources used by device, does not wait for
    // GPU or check errors.
    virtual void DestroyImpl() = 0;

    virtual MaybeError CopyFromStagingToBufferImpl(BufferBase* source,
                                                   uint64_t sourceOffset,
                                                   BufferBase* destination,
                                                   uint64_t destinationOffset,
                                                   uint64_t size) = 0;
    virtual MaybeError CopyFromStagingToTextureImpl(const BufferBase* source,
                                                    const TextureDataLayout& src,
                                                    const TextureCopy& dst,
                                                    const Extent3D& copySizePixels) = 0;

    wgpu::ErrorCallback mUncapturedErrorCallback = nullptr;
    void* mUncapturedErrorUserdata = nullptr;

    wgpu::LoggingCallback mLoggingCallback = nullptr;
    void* mLoggingUserdata = nullptr;

    wgpu::DeviceLostCallback mDeviceLostCallback = nullptr;
    void* mDeviceLostUserdata = nullptr;

    std::unique_ptr<ErrorScopeStack> mErrorScopeStack;

    Ref<AdapterBase> mAdapter;

    // The object caches aren't exposed in the header as they would require a lot of
    // additional includes.
    struct Caches;
    std::unique_ptr<Caches> mCaches;

    Ref<BindGroupLayoutBase> mEmptyBindGroupLayout;
    Ref<PipelineLayoutBase> mEmptyPipelineLayout;

    Ref<TextureViewBase> mExternalTexturePlaceholderView;

    std::unique_ptr<DynamicUploader> mDynamicUploader;
    std::unique_ptr<AsyncTaskManager> mAsyncTaskManager;
    Ref<QueueBase> mQueue;

    struct DeprecationWarnings;
    std::unique_ptr<DeprecationWarnings> mDeprecationWarnings;

    std::unordered_set<std::string> mWarnings;

    State mState = State::BeingCreated;

    PerObjectType<ApiObjectList> mObjectLists;

    FormatTable mFormatTable;

    TogglesState mToggles;

    size_t mLazyClearCountForTesting = 0;
    std::atomic_uint64_t mNextPipelineCompatibilityToken;

    CombinedLimits mLimits;
    FeaturesSet mEnabledFeatures;
    WGSLExtensionSet mWGSLExtensionAllowList;

    std::unique_ptr<InternalPipelineStore> mInternalPipelineStore;

    Ref<CallbackTaskManager> mCallbackTaskManager;
    std::unique_ptr<dawn::platform::WorkerTaskPool> mWorkerTaskPool;
    std::string mLabel;
    CacheKey mDeviceCacheKey;

    // This pointer is non-null if Feature::ImplicitDeviceSynchronization is turned on.
    Ref<Mutex> mMutex = nullptr;
};

ResultOrError<Ref<PipelineLayoutBase>> ValidateLayoutAndGetComputePipelineDescriptorWithDefaults(
    DeviceBase* device,
    const ComputePipelineDescriptor& descriptor,
    ComputePipelineDescriptor* outDescriptor);

ResultOrError<Ref<PipelineLayoutBase>> ValidateLayoutAndGetRenderPipelineDescriptorWithDefaults(
    DeviceBase* device,
    const RenderPipelineDescriptor& descriptor,
    RenderPipelineDescriptor* outDescriptor);

class IgnoreLazyClearCountScope : public NonMovable {
  public:
    explicit IgnoreLazyClearCountScope(DeviceBase* device);
    ~IgnoreLazyClearCountScope();

  private:
    // Disable heap allocation
    void* operator new(size_t) = delete;

    DeviceBase* mDevice;
    size_t mLazyClearCountForTesting;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_DEVICE_H_
