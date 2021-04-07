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

#include "dawn_native/Device.h"

#include "common/Log.h"
#include "dawn_native/Adapter.h"
#include "dawn_native/AttachmentState.h"
#include "dawn_native/BindGroup.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandBuffer.h"
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/ComputePipeline.h"
#include "dawn_native/CreatePipelineAsyncTracker.h"
#include "dawn_native/DynamicUploader.h"
#include "dawn_native/ErrorData.h"
#include "dawn_native/ErrorScope.h"
#include "dawn_native/ExternalTexture.h"
#include "dawn_native/Fence.h"
#include "dawn_native/Instance.h"
#include "dawn_native/InternalPipelineStore.h"
#include "dawn_native/PersistentCache.h"
#include "dawn_native/PipelineLayout.h"
#include "dawn_native/QuerySet.h"
#include "dawn_native/Queue.h"
#include "dawn_native/RenderBundleEncoder.h"
#include "dawn_native/RenderPipeline.h"
#include "dawn_native/Sampler.h"
#include "dawn_native/ShaderModule.h"
#include "dawn_native/Surface.h"
#include "dawn_native/SwapChain.h"
#include "dawn_native/Texture.h"
#include "dawn_native/ValidationUtils_autogen.h"

#include <unordered_set>

namespace dawn_native {

    // DeviceBase sub-structures

    // The caches are unordered_sets of pointers with special hash and compare functions
    // to compare the value of the objects, instead of the pointers.
    template <typename Object>
    using ContentLessObjectCache =
        std::unordered_set<Object*, typename Object::HashFunc, typename Object::EqualityFunc>;

    struct DeviceBase::Caches {
        ~Caches() {
            ASSERT(attachmentStates.empty());
            ASSERT(bindGroupLayouts.empty());
            ASSERT(computePipelines.empty());
            ASSERT(pipelineLayouts.empty());
            ASSERT(renderPipelines.empty());
            ASSERT(samplers.empty());
            ASSERT(shaderModules.empty());
        }

        ContentLessObjectCache<AttachmentStateBlueprint> attachmentStates;
        ContentLessObjectCache<BindGroupLayoutBase> bindGroupLayouts;
        ContentLessObjectCache<ComputePipelineBase> computePipelines;
        ContentLessObjectCache<PipelineLayoutBase> pipelineLayouts;
        ContentLessObjectCache<RenderPipelineBase> renderPipelines;
        ContentLessObjectCache<SamplerBase> samplers;
        ContentLessObjectCache<ShaderModuleBase> shaderModules;
    };

    struct DeviceBase::DeprecationWarnings {
        std::unordered_set<std::string> emitted;
        size_t count = 0;
    };

    // DeviceBase

    DeviceBase::DeviceBase(AdapterBase* adapter, const DeviceDescriptor* descriptor)
        : mInstance(adapter->GetInstance()), mAdapter(adapter) {
        if (descriptor != nullptr) {
            ApplyToggleOverrides(descriptor);
            ApplyExtensions(descriptor);
        }

        mFormatTable = BuildFormatTable(this);
        SetDefaultToggles();
    }

    DeviceBase::~DeviceBase() = default;

    MaybeError DeviceBase::Initialize(QueueBase* defaultQueue) {
        mQueue = AcquireRef(defaultQueue);

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

        mDeviceLostCallback = [](char const*, void*) {
            static bool calledOnce = false;
            if (!calledOnce) {
                calledOnce = true;
                dawn::WarningLog() << "No Dawn device lost callback was set. This is probably not "
                                      "intended. If you really want to ignore device lost "
                                      "and suppress this message, set the callback to null.";
            }
        };
#endif  // DAWN_ENABLE_ASSERTS

        mCaches = std::make_unique<DeviceBase::Caches>();
        mErrorScopeStack = std::make_unique<ErrorScopeStack>();
        mDynamicUploader = std::make_unique<DynamicUploader>(this);
        mCreatePipelineAsyncTracker = std::make_unique<CreatePipelineAsyncTracker>(this);
        mDeprecationWarnings = std::make_unique<DeprecationWarnings>();
        mInternalPipelineStore = std::make_unique<InternalPipelineStore>();
        mPersistentCache = std::make_unique<PersistentCache>(this);

        // Starting from now the backend can start doing reentrant calls so the device is marked as
        // alive.
        mState = State::Alive;

        DAWN_TRY_ASSIGN(mEmptyBindGroupLayout, CreateEmptyBindGroupLayout());

        return {};
    }

    void DeviceBase::ShutDownBase() {
        // Skip handling device facilities if they haven't even been created (or failed doing so)
        if (mState != State::BeingCreated) {
            // Reject all async pipeline creations.
            mCreatePipelineAsyncTracker->ClearForShutDown();
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
        }
        ASSERT(mCompletedSerial == mLastSubmittedSerial);
        ASSERT(mFutureSerial <= mCompletedSerial);

        if (mState != State::BeingCreated) {
            // The GPU timeline is finished.
            // Tick the queue-related tasks since they should be complete. This must be done before
            // ShutDownImpl() it may relinquish resources that will be freed by backends in the
            // ShutDownImpl() call.
            mQueue->Tick(GetCompletedCommandSerial());
            // Call TickImpl once last time to clean up resources
            // Ignore errors so that we can continue with destruction
            IgnoreErrors(TickImpl());
        }

        // At this point GPU operations are always finished, so we are in the disconnected state.
        mState = State::Disconnected;

        mDynamicUploader = nullptr;
        mCreatePipelineAsyncTracker = nullptr;
        mPersistentCache = nullptr;

        mEmptyBindGroupLayout = nullptr;

        mInternalPipelineStore = nullptr;

        AssumeCommandsComplete();
        // Tell the backend that it can free all the objects now that the GPU timeline is empty.
        ShutDownImpl();

        mCaches = nullptr;
    }

    void DeviceBase::HandleError(InternalErrorType type, const char* message) {
        if (type == InternalErrorType::DeviceLost) {
            // A real device lost happened. Set the state to disconnected as the device cannot be
            // used.
            mState = State::Disconnected;
        } else if (type == InternalErrorType::Internal) {
            // If we receive an internal error, assume the backend can't recover and proceed with
            // device destruction. We first wait for all previous commands to be completed so that
            // backend objects can be freed immediately, before handling the loss.

            // Move away from the Alive state so that the application cannot use this device
            // anymore.
            // TODO(cwallez@chromium.org): Do we need atomics for this to become visible to other
            // threads in a multithreaded scenario?
            mState = State::BeingDisconnected;

            // Ignore errors so that we can continue with destruction
            // Assume all commands are complete after WaitForIdleForDestruction (because they were)
            IgnoreErrors(WaitForIdleForDestruction());
            IgnoreErrors(TickImpl());
            AssumeCommandsComplete();
            ASSERT(mFutureSerial <= mCompletedSerial);
            mState = State::Disconnected;

            // Now everything is as if the device was lost.
            type = InternalErrorType::DeviceLost;
        }

        if (type == InternalErrorType::DeviceLost) {
            // The device was lost, call the application callback.
            if (mDeviceLostCallback != nullptr) {
                mDeviceLostCallback(message, mDeviceLostUserdata);
                mDeviceLostCallback = nullptr;
            }

            mQueue->HandleDeviceLoss();

            // Still forward device loss errors to the error scopes so they all reject.
            mErrorScopeStack->HandleError(ToWGPUErrorType(type), message);
        } else {
            // Pass the error to the error scope stack and call the uncaptured error callback
            // if it isn't handled. DeviceLost is not handled here because it should be
            // handled by the lost callback.
            bool captured = mErrorScopeStack->HandleError(ToWGPUErrorType(type), message);
            if (!captured && mUncapturedErrorCallback != nullptr) {
                mUncapturedErrorCallback(static_cast<WGPUErrorType>(ToWGPUErrorType(type)), message,
                                         mUncapturedErrorUserdata);
            }
        }
    }

    void DeviceBase::APIInjectError(wgpu::ErrorType type, const char* message) {
        if (ConsumedError(ValidateErrorType(type))) {
            return;
        }

        // This method should only be used to make error scope reject. For DeviceLost there is the
        // LoseForTesting function that can be used instead.
        if (type != wgpu::ErrorType::Validation && type != wgpu::ErrorType::OutOfMemory) {
            HandleError(InternalErrorType::Validation,
                        "Invalid injected error, must be Validation or OutOfMemory");
            return;
        }

        HandleError(FromWGPUErrorType(type), message);
    }

    void DeviceBase::ConsumeError(std::unique_ptr<ErrorData> error) {
        ASSERT(error != nullptr);
        std::ostringstream ss;
        ss << error->GetMessage();
        for (const auto& callsite : error->GetBacktrace()) {
            ss << "\n    at " << callsite.function << " (" << callsite.file << ":" << callsite.line
               << ")";
        }
        HandleError(error->GetType(), ss.str().c_str());
    }

    void DeviceBase::APISetUncapturedErrorCallback(wgpu::ErrorCallback callback, void* userdata) {
        mUncapturedErrorCallback = callback;
        mUncapturedErrorUserdata = userdata;
    }

    void DeviceBase::APISetDeviceLostCallback(wgpu::DeviceLostCallback callback, void* userdata) {
        mDeviceLostCallback = callback;
        mDeviceLostUserdata = userdata;
    }

    void DeviceBase::APIPushErrorScope(wgpu::ErrorFilter filter) {
        if (ConsumedError(ValidateErrorFilter(filter))) {
            return;
        }
        mErrorScopeStack->Push(filter);
    }

    bool DeviceBase::APIPopErrorScope(wgpu::ErrorCallback callback, void* userdata) {
        if (mErrorScopeStack->Empty()) {
            return false;
        }
        ErrorScope scope = mErrorScopeStack->Pop();
        if (callback != nullptr) {
            callback(static_cast<WGPUErrorType>(scope.GetErrorType()), scope.GetErrorMessage(),
                     userdata);
        }

        return true;
    }

    PersistentCache* DeviceBase::GetPersistentCache() {
        ASSERT(mPersistentCache.get() != nullptr);
        return mPersistentCache.get();
    }

    MaybeError DeviceBase::ValidateObject(const ObjectBase* object) const {
        ASSERT(object != nullptr);
        if (DAWN_UNLIKELY(object->GetDevice() != this)) {
            return DAWN_VALIDATION_ERROR("Object from a different device.");
        }
        if (DAWN_UNLIKELY(object->IsError())) {
            return DAWN_VALIDATION_ERROR("Object is an error.");
        }
        return {};
    }

    MaybeError DeviceBase::ValidateIsAlive() const {
        if (DAWN_LIKELY(mState == State::Alive)) {
            return {};
        }
        return DAWN_VALIDATION_ERROR("Device is lost");
    }

    void DeviceBase::APILoseForTesting() {
        if (mState != State::Alive) {
            return;
        }

        HandleError(InternalErrorType::Internal, "Device lost for testing");
    }

    DeviceBase::State DeviceBase::GetState() const {
        return mState;
    }

    bool DeviceBase::IsLost() const {
        ASSERT(mState != State::BeingCreated);
        return mState != State::Alive;
    }

    AdapterBase* DeviceBase::GetAdapter() const {
        return mAdapter;
    }

    dawn_platform::Platform* DeviceBase::GetPlatform() const {
        return GetAdapter()->GetInstance()->GetPlatform();
    }

    ExecutionSerial DeviceBase::GetCompletedCommandSerial() const {
        return mCompletedSerial;
    }

    ExecutionSerial DeviceBase::GetLastSubmittedCommandSerial() const {
        return mLastSubmittedSerial;
    }

    ExecutionSerial DeviceBase::GetFutureSerial() const {
        return mFutureSerial;
    }

    InternalPipelineStore* DeviceBase::GetInternalPipelineStore() {
        return mInternalPipelineStore.get();
    }

    void DeviceBase::IncrementLastSubmittedCommandSerial() {
        mLastSubmittedSerial++;
    }

    void DeviceBase::AssumeCommandsComplete() {
        ExecutionSerial maxSerial =
            ExecutionSerial(std::max(mLastSubmittedSerial + ExecutionSerial(1), mFutureSerial));
        mLastSubmittedSerial = maxSerial;
        mCompletedSerial = maxSerial;
    }

    bool DeviceBase::IsDeviceIdle() {
        ExecutionSerial maxSerial = std::max(mLastSubmittedSerial, mFutureSerial);
        if (mCompletedSerial == maxSerial) {
            return true;
        }
        return false;
    }

    ExecutionSerial DeviceBase::GetPendingCommandSerial() const {
        return mLastSubmittedSerial + ExecutionSerial(1);
    }

    void DeviceBase::AddFutureSerial(ExecutionSerial serial) {
        if (serial > mFutureSerial) {
            mFutureSerial = serial;
        }
    }

    MaybeError DeviceBase::CheckPassedSerials() {
        ExecutionSerial completedSerial;
        DAWN_TRY_ASSIGN(completedSerial, CheckAndUpdateCompletedSerials());

        ASSERT(completedSerial <= mLastSubmittedSerial);
        // completedSerial should not be less than mCompletedSerial unless it is 0.
        // It can be 0 when there's no fences to check.
        ASSERT(completedSerial >= mCompletedSerial || completedSerial == ExecutionSerial(0));

        if (completedSerial > mCompletedSerial) {
            mCompletedSerial = completedSerial;
        }

        return {};
    }

    ResultOrError<const Format*> DeviceBase::GetInternalFormat(wgpu::TextureFormat format) const {
        size_t index = ComputeFormatIndex(format);
        if (index >= mFormatTable.size()) {
            return DAWN_VALIDATION_ERROR("Unknown texture format");
        }

        const Format* internalFormat = &mFormatTable[index];
        if (!internalFormat->isSupported) {
            return DAWN_VALIDATION_ERROR("Unsupported texture format");
        }

        return internalFormat;
    }

    const Format& DeviceBase::GetValidInternalFormat(wgpu::TextureFormat format) const {
        size_t index = ComputeFormatIndex(format);
        ASSERT(index < mFormatTable.size());
        ASSERT(mFormatTable[index].isSupported);
        return mFormatTable[index];
    }

    ResultOrError<Ref<BindGroupLayoutBase>> DeviceBase::GetOrCreateBindGroupLayout(
        const BindGroupLayoutDescriptor* descriptor) {
        BindGroupLayoutBase blueprint(this, descriptor);

        const size_t blueprintHash = blueprint.ComputeContentHash();
        blueprint.SetContentHash(blueprintHash);

        Ref<BindGroupLayoutBase> result;
        auto iter = mCaches->bindGroupLayouts.find(&blueprint);
        if (iter != mCaches->bindGroupLayouts.end()) {
            result = *iter;
        } else {
            DAWN_TRY_ASSIGN(result, CreateBindGroupLayoutImpl(descriptor));
            result->SetIsCachedReference();
            result->SetContentHash(blueprintHash);
            mCaches->bindGroupLayouts.insert(result.Get());
        }

        return std::move(result);
    }

    void DeviceBase::UncacheBindGroupLayout(BindGroupLayoutBase* obj) {
        ASSERT(obj->IsCachedReference());
        size_t removedCount = mCaches->bindGroupLayouts.erase(obj);
        ASSERT(removedCount == 1);
    }

    // Private function used at initialization
    ResultOrError<Ref<BindGroupLayoutBase>> DeviceBase::CreateEmptyBindGroupLayout() {
        BindGroupLayoutDescriptor desc = {};
        desc.entryCount = 0;
        desc.entries = nullptr;

        return GetOrCreateBindGroupLayout(&desc);
    }

    BindGroupLayoutBase* DeviceBase::GetEmptyBindGroupLayout() {
        ASSERT(mEmptyBindGroupLayout != nullptr);
        return mEmptyBindGroupLayout.Get();
    }

    ResultOrError<Ref<ComputePipelineBase>> DeviceBase::GetOrCreateComputePipeline(
        const ComputePipelineDescriptor* descriptor) {
        ComputePipelineBase blueprint(this, descriptor);

        const size_t blueprintHash = blueprint.ComputeContentHash();
        blueprint.SetContentHash(blueprintHash);

        Ref<ComputePipelineBase> result;
        auto iter = mCaches->computePipelines.find(&blueprint);
        if (iter != mCaches->computePipelines.end()) {
            result = *iter;
        } else {
            DAWN_TRY_ASSIGN(result, CreateComputePipelineImpl(descriptor));
            result->SetIsCachedReference();
            result->SetContentHash(blueprintHash);
            mCaches->computePipelines.insert(result.Get());
        }

        return std::move(result);
    }

    void DeviceBase::UncacheComputePipeline(ComputePipelineBase* obj) {
        ASSERT(obj->IsCachedReference());
        size_t removedCount = mCaches->computePipelines.erase(obj);
        ASSERT(removedCount == 1);
    }

    ResultOrError<Ref<PipelineLayoutBase>> DeviceBase::GetOrCreatePipelineLayout(
        const PipelineLayoutDescriptor* descriptor) {
        PipelineLayoutBase blueprint(this, descriptor);

        const size_t blueprintHash = blueprint.ComputeContentHash();
        blueprint.SetContentHash(blueprintHash);

        Ref<PipelineLayoutBase> result;
        auto iter = mCaches->pipelineLayouts.find(&blueprint);
        if (iter != mCaches->pipelineLayouts.end()) {
            result = *iter;
        } else {
            DAWN_TRY_ASSIGN(result, CreatePipelineLayoutImpl(descriptor));
            result->SetIsCachedReference();
            result->SetContentHash(blueprintHash);
            mCaches->pipelineLayouts.insert(result.Get());
        }

        return std::move(result);
    }

    void DeviceBase::UncachePipelineLayout(PipelineLayoutBase* obj) {
        ASSERT(obj->IsCachedReference());
        size_t removedCount = mCaches->pipelineLayouts.erase(obj);
        ASSERT(removedCount == 1);
    }

    ResultOrError<Ref<RenderPipelineBase>> DeviceBase::GetOrCreateRenderPipeline(
        const RenderPipelineDescriptor2* descriptor) {
        RenderPipelineBase blueprint(this, descriptor);

        const size_t blueprintHash = blueprint.ComputeContentHash();
        blueprint.SetContentHash(blueprintHash);

        Ref<RenderPipelineBase> result;
        auto iter = mCaches->renderPipelines.find(&blueprint);
        if (iter != mCaches->renderPipelines.end()) {
            result = *iter;
        } else {
            DAWN_TRY_ASSIGN(result, CreateRenderPipelineImpl(descriptor));
            result->SetIsCachedReference();
            result->SetContentHash(blueprintHash);
            mCaches->renderPipelines.insert(result.Get());
        }

        return std::move(result);
    }

    void DeviceBase::UncacheRenderPipeline(RenderPipelineBase* obj) {
        ASSERT(obj->IsCachedReference());
        size_t removedCount = mCaches->renderPipelines.erase(obj);
        ASSERT(removedCount == 1);
    }

    ResultOrError<Ref<SamplerBase>> DeviceBase::GetOrCreateSampler(
        const SamplerDescriptor* descriptor) {
        SamplerBase blueprint(this, descriptor);

        const size_t blueprintHash = blueprint.ComputeContentHash();
        blueprint.SetContentHash(blueprintHash);

        Ref<SamplerBase> result;
        auto iter = mCaches->samplers.find(&blueprint);
        if (iter != mCaches->samplers.end()) {
            result = *iter;
        } else {
            DAWN_TRY_ASSIGN(result, CreateSamplerImpl(descriptor));
            result->SetIsCachedReference();
            result->SetContentHash(blueprintHash);
            mCaches->samplers.insert(result.Get());
        }

        return std::move(result);
    }

    void DeviceBase::UncacheSampler(SamplerBase* obj) {
        ASSERT(obj->IsCachedReference());
        size_t removedCount = mCaches->samplers.erase(obj);
        ASSERT(removedCount == 1);
    }

    ResultOrError<Ref<ShaderModuleBase>> DeviceBase::GetOrCreateShaderModule(
        const ShaderModuleDescriptor* descriptor,
        ShaderModuleParseResult* parseResult) {
        ShaderModuleBase blueprint(this, descriptor);

        const size_t blueprintHash = blueprint.ComputeContentHash();
        blueprint.SetContentHash(blueprintHash);

        Ref<ShaderModuleBase> result;
        auto iter = mCaches->shaderModules.find(&blueprint);
        if (iter != mCaches->shaderModules.end()) {
            result = *iter;
        } else {
            if (parseResult == nullptr) {
                // We skip the parse on creation if validation isn't enabled which let's us quickly
                // lookup in the cache without validating and parsing. We need the parsed module
                // now, so call validate. Most of |ValidateShaderModuleDescriptor| is parsing, but
                // we can consider splitting it if additional validation is added.
                ASSERT(!IsValidationEnabled());
                ShaderModuleParseResult localParseResult =
                    ValidateShaderModuleDescriptor(this, descriptor).AcquireSuccess();
                DAWN_TRY_ASSIGN(result, CreateShaderModuleImpl(descriptor, &localParseResult));
            } else {
                DAWN_TRY_ASSIGN(result, CreateShaderModuleImpl(descriptor, parseResult));
            }
            result->SetIsCachedReference();
            result->SetContentHash(blueprintHash);
            mCaches->shaderModules.insert(result.Get());
        }

        return std::move(result);
    }

    void DeviceBase::UncacheShaderModule(ShaderModuleBase* obj) {
        ASSERT(obj->IsCachedReference());
        size_t removedCount = mCaches->shaderModules.erase(obj);
        ASSERT(removedCount == 1);
    }

    Ref<AttachmentState> DeviceBase::GetOrCreateAttachmentState(
        AttachmentStateBlueprint* blueprint) {
        auto iter = mCaches->attachmentStates.find(blueprint);
        if (iter != mCaches->attachmentStates.end()) {
            return static_cast<AttachmentState*>(*iter);
        }

        Ref<AttachmentState> attachmentState = AcquireRef(new AttachmentState(this, *blueprint));
        attachmentState->SetIsCachedReference();
        attachmentState->SetContentHash(attachmentState->ComputeContentHash());
        mCaches->attachmentStates.insert(attachmentState.Get());
        return attachmentState;
    }

    Ref<AttachmentState> DeviceBase::GetOrCreateAttachmentState(
        const RenderBundleEncoderDescriptor* descriptor) {
        AttachmentStateBlueprint blueprint(descriptor);
        return GetOrCreateAttachmentState(&blueprint);
    }

    Ref<AttachmentState> DeviceBase::GetOrCreateAttachmentState(
        const RenderPipelineDescriptor* descriptor) {
        AttachmentStateBlueprint blueprint(descriptor);
        return GetOrCreateAttachmentState(&blueprint);
    }

    Ref<AttachmentState> DeviceBase::GetOrCreateAttachmentState(
        const RenderPipelineDescriptor2* descriptor) {
        AttachmentStateBlueprint blueprint(descriptor);
        return GetOrCreateAttachmentState(&blueprint);
    }

    Ref<AttachmentState> DeviceBase::GetOrCreateAttachmentState(
        const RenderPassDescriptor* descriptor) {
        AttachmentStateBlueprint blueprint(descriptor);
        return GetOrCreateAttachmentState(&blueprint);
    }

    void DeviceBase::UncacheAttachmentState(AttachmentState* obj) {
        ASSERT(obj->IsCachedReference());
        size_t removedCount = mCaches->attachmentStates.erase(obj);
        ASSERT(removedCount == 1);
    }

    // Object creation API methods

    BindGroupBase* DeviceBase::APICreateBindGroup(const BindGroupDescriptor* descriptor) {
        Ref<BindGroupBase> result;
        if (ConsumedError(CreateBindGroupInternal(descriptor), &result)) {
            return BindGroupBase::MakeError(this);
        }
        return result.Detach();
    }
    BindGroupLayoutBase* DeviceBase::APICreateBindGroupLayout(
        const BindGroupLayoutDescriptor* descriptor) {
        Ref<BindGroupLayoutBase> result;
        if (ConsumedError(CreateBindGroupLayoutInternal(descriptor), &result)) {
            return BindGroupLayoutBase::MakeError(this);
        }
        return result.Detach();
    }
    BufferBase* DeviceBase::APICreateBuffer(const BufferDescriptor* descriptor) {
        Ref<BufferBase> result = nullptr;
        if (ConsumedError(CreateBufferInternal(descriptor), &result)) {
            ASSERT(result == nullptr);
            return BufferBase::MakeError(this, descriptor);
        }
        return result.Detach();
    }
    CommandEncoder* DeviceBase::APICreateCommandEncoder(
        const CommandEncoderDescriptor* descriptor) {
        return new CommandEncoder(this, descriptor);
    }
    ComputePipelineBase* DeviceBase::APICreateComputePipeline(
        const ComputePipelineDescriptor* descriptor) {
        Ref<ComputePipelineBase> result;
        if (ConsumedError(CreateComputePipelineInternal(descriptor), &result)) {
            return ComputePipelineBase::MakeError(this);
        }
        return result.Detach();
    }
    void DeviceBase::APICreateComputePipelineAsync(const ComputePipelineDescriptor* descriptor,
                                                   WGPUCreateComputePipelineAsyncCallback callback,
                                                   void* userdata) {
        ResultOrError<Ref<ComputePipelineBase>> maybeResult =
            CreateComputePipelineInternal(descriptor);
        if (maybeResult.IsError()) {
            std::unique_ptr<ErrorData> error = maybeResult.AcquireError();
            callback(WGPUCreatePipelineAsyncStatus_Error, nullptr, error->GetMessage().c_str(),
                     userdata);
            return;
        }

        std::unique_ptr<CreateComputePipelineAsyncTask> request =
            std::make_unique<CreateComputePipelineAsyncTask>(maybeResult.AcquireSuccess().Detach(),
                                                             callback, userdata);
        mCreatePipelineAsyncTracker->TrackTask(std::move(request), GetPendingCommandSerial());
    }
    PipelineLayoutBase* DeviceBase::APICreatePipelineLayout(
        const PipelineLayoutDescriptor* descriptor) {
        Ref<PipelineLayoutBase> result;
        if (ConsumedError(CreatePipelineLayoutInternal(descriptor), &result)) {
            return PipelineLayoutBase::MakeError(this);
        }
        return result.Detach();
    }
    QuerySetBase* DeviceBase::APICreateQuerySet(const QuerySetDescriptor* descriptor) {
        Ref<QuerySetBase> result;
        if (ConsumedError(CreateQuerySetInternal(descriptor), &result)) {
            return QuerySetBase::MakeError(this);
        }
        return result.Detach();
    }
    SamplerBase* DeviceBase::APICreateSampler(const SamplerDescriptor* descriptor) {
        Ref<SamplerBase> result;
        if (ConsumedError(CreateSamplerInternal(descriptor), &result)) {
            return SamplerBase::MakeError(this);
        }
        return result.Detach();
    }
    void DeviceBase::APICreateRenderPipelineAsync(const RenderPipelineDescriptor2* descriptor,
                                                  WGPUCreateRenderPipelineAsyncCallback callback,
                                                  void* userdata) {
        ResultOrError<Ref<RenderPipelineBase>> maybeResult =
            CreateRenderPipelineInternal(descriptor);
        if (maybeResult.IsError()) {
            std::unique_ptr<ErrorData> error = maybeResult.AcquireError();
            callback(WGPUCreatePipelineAsyncStatus_Error, nullptr, error->GetMessage().c_str(),
                     userdata);
            return;
        }

        std::unique_ptr<CreateRenderPipelineAsyncTask> request =
            std::make_unique<CreateRenderPipelineAsyncTask>(maybeResult.AcquireSuccess().Detach(),
                                                            callback, userdata);
        mCreatePipelineAsyncTracker->TrackTask(std::move(request), GetPendingCommandSerial());
    }
    RenderBundleEncoder* DeviceBase::APICreateRenderBundleEncoder(
        const RenderBundleEncoderDescriptor* descriptor) {
        Ref<RenderBundleEncoder> result;
        if (ConsumedError(CreateRenderBundleEncoderInternal(descriptor), &result)) {
            return RenderBundleEncoder::MakeError(this);
        }
        return result.Detach();
    }
    RenderPipelineBase* DeviceBase::APICreateRenderPipeline(
        const RenderPipelineDescriptor* descriptor) {
        // TODO: Enable this warning once the tests have been converted to either use the new
        // format or expect the deprecation warning.
        EmitDeprecationWarning(
            "The format of RenderPipelineDescriptor has changed, and will soon require the "
            "new structure. Please begin using CreateRenderPipeline2() instead.");

        // Convert descriptor to the new format it before proceeding.
        RenderPipelineDescriptor2 normalizedDescriptor;

        normalizedDescriptor.label = descriptor->label;
        normalizedDescriptor.layout = descriptor->layout;

        normalizedDescriptor.vertex.module = descriptor->vertexStage.module;
        normalizedDescriptor.vertex.entryPoint = descriptor->vertexStage.entryPoint;

        normalizedDescriptor.primitive.topology = descriptor->primitiveTopology;

        normalizedDescriptor.multisample.count = descriptor->sampleCount;
        normalizedDescriptor.multisample.mask = descriptor->sampleMask;
        normalizedDescriptor.multisample.alphaToCoverageEnabled =
            descriptor->alphaToCoverageEnabled;

        if (descriptor->vertexState) {
            const VertexStateDescriptor* vertexState = descriptor->vertexState;
            normalizedDescriptor.primitive.stripIndexFormat = vertexState->indexFormat;
            normalizedDescriptor.vertex.bufferCount = vertexState->vertexBufferCount;
            normalizedDescriptor.vertex.buffers = vertexState->vertexBuffers;
        } else {
            normalizedDescriptor.vertex.bufferCount = 0;
            normalizedDescriptor.vertex.buffers = nullptr;
        }

        DepthStencilState depthStencil;
        if (descriptor->depthStencilState) {
            const DepthStencilStateDescriptor* depthStencilState = descriptor->depthStencilState;
            normalizedDescriptor.depthStencil = &depthStencil;

            depthStencil.format = depthStencilState->format;
            depthStencil.depthWriteEnabled = depthStencilState->depthWriteEnabled;
            depthStencil.depthCompare = depthStencilState->depthCompare;
            depthStencil.stencilFront = depthStencilState->stencilFront;
            depthStencil.stencilBack = depthStencilState->stencilBack;
            depthStencil.stencilReadMask = depthStencilState->stencilReadMask;
            depthStencil.stencilWriteMask = depthStencilState->stencilWriteMask;
        }

        if (descriptor->rasterizationState) {
            const RasterizationStateDescriptor* rasterizationState = descriptor->rasterizationState;
            normalizedDescriptor.primitive.frontFace = rasterizationState->frontFace;
            normalizedDescriptor.primitive.cullMode = rasterizationState->cullMode;
            depthStencil.depthBias = rasterizationState->depthBias;
            depthStencil.depthBiasSlopeScale = rasterizationState->depthBiasSlopeScale;
            depthStencil.depthBiasClamp = rasterizationState->depthBiasClamp;
        }

        FragmentState fragment;
        std::vector<ColorTargetState> targets;
        std::vector<BlendState> blendStates;
        if (descriptor->fragmentStage) {
            const ProgrammableStageDescriptor* fragmentStage = descriptor->fragmentStage;
            normalizedDescriptor.fragment = &fragment;

            fragment.module = fragmentStage->module;
            fragment.entryPoint = fragmentStage->entryPoint;

            targets.resize(descriptor->colorStateCount);
            blendStates.resize(descriptor->colorStateCount);

            for (uint32_t i = 0; i < descriptor->colorStateCount; ++i) {
                const ColorStateDescriptor& colorState = descriptor->colorStates[i];
                ColorTargetState& target = targets[i];
                target.format = colorState.format;
                target.writeMask = colorState.writeMask;

                if (BlendEnabled(&colorState)) {
                    BlendState* blend = &blendStates[i];
                    target.blend = blend;

                    blend->color.srcFactor = colorState.colorBlend.srcFactor;
                    blend->color.dstFactor = colorState.colorBlend.dstFactor;
                    blend->color.operation = colorState.colorBlend.operation;

                    blend->alpha.srcFactor = colorState.alphaBlend.srcFactor;
                    blend->alpha.dstFactor = colorState.alphaBlend.dstFactor;
                    blend->alpha.operation = colorState.alphaBlend.operation;
                }
            }

            fragment.targetCount = descriptor->colorStateCount;
            fragment.targets = targets.data();
        }

        Ref<RenderPipelineBase> result;
        if (ConsumedError(CreateRenderPipelineInternal(&normalizedDescriptor), &result)) {
            return RenderPipelineBase::MakeError(this);
        }
        return result.Detach();
    }
    RenderPipelineBase* DeviceBase::APICreateRenderPipeline2(
        const RenderPipelineDescriptor2* descriptor) {
        Ref<RenderPipelineBase> result;
        if (ConsumedError(CreateRenderPipelineInternal(descriptor), &result)) {
            return RenderPipelineBase::MakeError(this);
        }
        return result.Detach();
    }
    ShaderModuleBase* DeviceBase::APICreateShaderModule(const ShaderModuleDescriptor* descriptor) {
        Ref<ShaderModuleBase> result;
        if (ConsumedError(CreateShaderModuleInternal(descriptor), &result)) {
            return ShaderModuleBase::MakeError(this);
        }
        return result.Detach();
    }
    SwapChainBase* DeviceBase::APICreateSwapChain(Surface* surface,
                                                  const SwapChainDescriptor* descriptor) {
        Ref<SwapChainBase> result;
        if (ConsumedError(CreateSwapChainInternal(surface, descriptor), &result)) {
            return SwapChainBase::MakeError(this);
        }
        return result.Detach();
    }
    TextureBase* DeviceBase::APICreateTexture(const TextureDescriptor* descriptor) {
        Ref<TextureBase> result;
        if (ConsumedError(CreateTextureInternal(descriptor), &result)) {
            return TextureBase::MakeError(this);
        }
        return result.Detach();
    }
    TextureViewBase* DeviceBase::CreateTextureView(TextureBase* texture,
                                                   const TextureViewDescriptor* descriptor) {
        Ref<TextureViewBase> result;
        if (ConsumedError(CreateTextureViewInternal(texture, descriptor), &result)) {
            return TextureViewBase::MakeError(this);
        }
        return result.Detach();
    }

    // For Dawn Wire

    BufferBase* DeviceBase::APICreateErrorBuffer() {
        BufferDescriptor desc = {};
        return BufferBase::MakeError(this, &desc);
    }

    // Other Device API methods

    // Returns true if future ticking is needed.
    bool DeviceBase::APITick() {
        if (ConsumedError(Tick())) {
            return false;
        }
        return !IsDeviceIdle();
    }

    MaybeError DeviceBase::Tick() {
        DAWN_TRY(ValidateIsAlive());

        // to avoid overly ticking, we only want to tick when:
        // 1. the last submitted serial has moved beyond the completed serial
        // 2. or the completed serial has not reached the future serial set by the trackers
        if (mLastSubmittedSerial > mCompletedSerial || mCompletedSerial < mFutureSerial) {
            DAWN_TRY(CheckPassedSerials());
            DAWN_TRY(TickImpl());

            // There is no GPU work in flight, we need to move the serials forward so that
            // so that CPU operations waiting on GPU completion can know they don't have to wait.
            // AssumeCommandsComplete will assign the max serial we must tick to in order to
            // fire the awaiting callbacks.
            if (mCompletedSerial == mLastSubmittedSerial) {
                AssumeCommandsComplete();
            }

            // TODO(cwallez@chromium.org): decouple TickImpl from updating the serial so that we can
            // tick the dynamic uploader before the backend resource allocators. This would allow
            // reclaiming resources one tick earlier.
            mDynamicUploader->Deallocate(mCompletedSerial);
            mQueue->Tick(mCompletedSerial);

            mCreatePipelineAsyncTracker->Tick(mCompletedSerial);
        }

        return {};
    }

    QueueBase* DeviceBase::APIGetQueue() {
        // Backends gave the primary queue during initialization.
        ASSERT(mQueue != nullptr);

        // Returns a new reference to the queue.
        mQueue->Reference();
        return mQueue.Get();
    }

    QueueBase* DeviceBase::APIGetDefaultQueue() {
        EmitDeprecationWarning(
            "Device::GetDefaultQueue is deprecated, use Device::GetQueue() instead");
        return APIGetQueue();
    }

    ExternalTextureBase* DeviceBase::APICreateExternalTexture(
        const ExternalTextureDescriptor* descriptor) {
        Ref<ExternalTextureBase> result = nullptr;
        if (ConsumedError(CreateExternalTextureInternal(descriptor), &result)) {
            return ExternalTextureBase::MakeError(this);
        }

        return result.Detach();
    }

    void DeviceBase::ApplyExtensions(const DeviceDescriptor* deviceDescriptor) {
        ASSERT(deviceDescriptor);
        ASSERT(GetAdapter()->SupportsAllRequestedExtensions(deviceDescriptor->requiredExtensions));

        mEnabledExtensions = GetAdapter()->GetInstance()->ExtensionNamesToExtensionsSet(
            deviceDescriptor->requiredExtensions);
    }

    std::vector<const char*> DeviceBase::GetEnabledExtensions() const {
        return mEnabledExtensions.GetEnabledExtensionNames();
    }

    bool DeviceBase::IsExtensionEnabled(Extension extension) const {
        return mEnabledExtensions.IsEnabled(extension);
    }

    bool DeviceBase::IsValidationEnabled() const {
        return !IsToggleEnabled(Toggle::SkipValidation);
    }

    bool DeviceBase::IsRobustnessEnabled() const {
        return !IsToggleEnabled(Toggle::DisableRobustness);
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

    void DeviceBase::EmitDeprecationWarning(const char* warning) {
        mDeprecationWarnings->count++;
        if (mDeprecationWarnings->emitted.insert(warning).second) {
            dawn::WarningLog() << warning;
        }
    }

    QueueBase* DeviceBase::GetQueue() const {
        return mQueue.Get();
    }

    // Implementation details of object creation

    ResultOrError<Ref<BindGroupBase>> DeviceBase::CreateBindGroupInternal(
        const BindGroupDescriptor* descriptor) {
        DAWN_TRY(ValidateIsAlive());
        if (IsValidationEnabled()) {
            DAWN_TRY(ValidateBindGroupDescriptor(this, descriptor));
        }
        return CreateBindGroupImpl(descriptor);
    }

    ResultOrError<Ref<BindGroupLayoutBase>> DeviceBase::CreateBindGroupLayoutInternal(
        const BindGroupLayoutDescriptor* descriptor) {
        DAWN_TRY(ValidateIsAlive());
        if (IsValidationEnabled()) {
            DAWN_TRY(ValidateBindGroupLayoutDescriptor(this, descriptor));
        }
        return GetOrCreateBindGroupLayout(descriptor);
    }

    ResultOrError<Ref<BufferBase>> DeviceBase::CreateBufferInternal(
        const BufferDescriptor* descriptor) {
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

    ResultOrError<Ref<ComputePipelineBase>> DeviceBase::CreateComputePipelineInternal(
        const ComputePipelineDescriptor* descriptor) {
        DAWN_TRY(ValidateIsAlive());
        if (IsValidationEnabled()) {
            DAWN_TRY(ValidateComputePipelineDescriptor(this, descriptor));
        }

        if (descriptor->layout == nullptr) {
            ComputePipelineDescriptor descriptorWithDefaultLayout = *descriptor;

            // Ref will keep the pipeline layout alive until the end of the function where
            // the pipeline will take another reference.
            Ref<PipelineLayoutBase> layoutRef;
            DAWN_TRY_ASSIGN(layoutRef,
                            PipelineLayoutBase::CreateDefault(
                                this, {{SingleShaderStage::Compute, descriptor->computeStage.module,
                                        descriptor->computeStage.entryPoint}}));

            descriptorWithDefaultLayout.layout = layoutRef.Get();

            return GetOrCreateComputePipeline(&descriptorWithDefaultLayout);
        } else {
            return GetOrCreateComputePipeline(descriptor);
        }
    }

    ResultOrError<Ref<PipelineLayoutBase>> DeviceBase::CreatePipelineLayoutInternal(
        const PipelineLayoutDescriptor* descriptor) {
        DAWN_TRY(ValidateIsAlive());
        if (IsValidationEnabled()) {
            DAWN_TRY(ValidatePipelineLayoutDescriptor(this, descriptor));
        }
        return GetOrCreatePipelineLayout(descriptor);
    }

    ResultOrError<Ref<ExternalTextureBase>> DeviceBase::CreateExternalTextureInternal(
        const ExternalTextureDescriptor* descriptor) {
        if (IsValidationEnabled()) {
            DAWN_TRY(ValidateExternalTextureDescriptor(this, descriptor));
        }

        return ExternalTextureBase::Create(this, descriptor);
    }

    ResultOrError<Ref<QuerySetBase>> DeviceBase::CreateQuerySetInternal(
        const QuerySetDescriptor* descriptor) {
        DAWN_TRY(ValidateIsAlive());
        if (IsValidationEnabled()) {
            DAWN_TRY(ValidateQuerySetDescriptor(this, descriptor));
        }
        return CreateQuerySetImpl(descriptor);
    }

    ResultOrError<Ref<RenderBundleEncoder>> DeviceBase::CreateRenderBundleEncoderInternal(
        const RenderBundleEncoderDescriptor* descriptor) {
        DAWN_TRY(ValidateIsAlive());
        if (IsValidationEnabled()) {
            DAWN_TRY(ValidateRenderBundleEncoderDescriptor(this, descriptor));
        }
        return RenderBundleEncoder::Create(this, descriptor);
    }

    ResultOrError<Ref<RenderPipelineBase>> DeviceBase::CreateRenderPipelineInternal(
        const RenderPipelineDescriptor2* descriptor) {
        DAWN_TRY(ValidateIsAlive());
        if (IsValidationEnabled()) {
            DAWN_TRY(ValidateRenderPipelineDescriptor(this, descriptor));
        }

        if (descriptor->layout == nullptr) {
            RenderPipelineDescriptor2 descriptorWithDefaultLayout = *descriptor;

            // Ref will keep the pipeline layout alive until the end of the function where
            // the pipeline will take another reference.
            Ref<PipelineLayoutBase> layoutRef;
            DAWN_TRY_ASSIGN(layoutRef,
                            PipelineLayoutBase::CreateDefault(this, GetStages(descriptor)));
            descriptorWithDefaultLayout.layout = layoutRef.Get();

            return GetOrCreateRenderPipeline(&descriptorWithDefaultLayout);
        } else {
            return GetOrCreateRenderPipeline(descriptor);
        }
    }

    ResultOrError<Ref<SamplerBase>> DeviceBase::CreateSamplerInternal(
        const SamplerDescriptor* descriptor) {
        const SamplerDescriptor defaultDescriptor = {};
        DAWN_TRY(ValidateIsAlive());
        descriptor = descriptor != nullptr ? descriptor : &defaultDescriptor;
        if (IsValidationEnabled()) {
            DAWN_TRY(ValidateSamplerDescriptor(this, descriptor));
        }
        return GetOrCreateSampler(descriptor);
    }

    ResultOrError<Ref<ShaderModuleBase>> DeviceBase::CreateShaderModuleInternal(
        const ShaderModuleDescriptor* descriptor) {
        DAWN_TRY(ValidateIsAlive());

        ShaderModuleParseResult parseResult = {};
        ShaderModuleParseResult* parseResultPtr = nullptr;
        if (IsValidationEnabled()) {
            DAWN_TRY_ASSIGN(parseResult, ValidateShaderModuleDescriptor(this, descriptor));
            parseResultPtr = &parseResult;
        }

        return GetOrCreateShaderModule(descriptor, parseResultPtr);
    }

    ResultOrError<Ref<SwapChainBase>> DeviceBase::CreateSwapChainInternal(
        Surface* surface,
        const SwapChainDescriptor* descriptor) {
        DAWN_TRY(ValidateIsAlive());
        if (IsValidationEnabled()) {
            DAWN_TRY(ValidateSwapChainDescriptor(this, surface, descriptor));
        }

        // TODO(dawn:269): Remove this code path once implementation-based swapchains are removed.
        if (surface == nullptr) {
            return CreateSwapChainImpl(descriptor);
        } else {
            ASSERT(descriptor->implementation == 0);

            NewSwapChainBase* previousSwapChain = surface->GetAttachedSwapChain();
            ResultOrError<Ref<NewSwapChainBase>> maybeNewSwapChain =
                CreateSwapChainImpl(surface, previousSwapChain, descriptor);

            if (previousSwapChain != nullptr) {
                previousSwapChain->DetachFromSurface();
            }

            Ref<NewSwapChainBase> newSwapChain;
            DAWN_TRY_ASSIGN(newSwapChain, std::move(maybeNewSwapChain));

            newSwapChain->SetIsAttached();
            surface->SetAttachedSwapChain(newSwapChain.Get());
            return newSwapChain;
        }
    }

    ResultOrError<Ref<TextureBase>> DeviceBase::CreateTextureInternal(
        const TextureDescriptor* descriptor) {
        DAWN_TRY(ValidateIsAlive());
        TextureDescriptor fixedDescriptor = *descriptor;
        DAWN_TRY(FixUpDeprecatedGPUExtent3DDepth(this, &(fixedDescriptor.size)));
        if (IsValidationEnabled()) {
            DAWN_TRY(ValidateTextureDescriptor(this, &fixedDescriptor));
        }
        return CreateTextureImpl(&fixedDescriptor);
    }

    ResultOrError<Ref<TextureViewBase>> DeviceBase::CreateTextureViewInternal(
        TextureBase* texture,
        const TextureViewDescriptor* descriptor) {
        DAWN_TRY(ValidateIsAlive());
        DAWN_TRY(ValidateObject(texture));
        TextureViewDescriptor desc = GetTextureViewDescriptorWithDefaults(texture, descriptor);
        if (IsValidationEnabled()) {
            DAWN_TRY(ValidateTextureViewDescriptor(texture, &desc));
        }
        return CreateTextureViewImpl(texture, &desc);
    }

    // Other implementation details

    DynamicUploader* DeviceBase::GetDynamicUploader() const {
        return mDynamicUploader.get();
    }

    // The Toggle device facility

    std::vector<const char*> DeviceBase::GetTogglesUsed() const {
        return mEnabledToggles.GetContainedToggleNames();
    }

    bool DeviceBase::IsToggleEnabled(Toggle toggle) const {
        return mEnabledToggles.Has(toggle);
    }

    void DeviceBase::SetToggle(Toggle toggle, bool isEnabled) {
        if (!mOverridenToggles.Has(toggle)) {
            mEnabledToggles.Set(toggle, isEnabled);
        }
    }

    void DeviceBase::ForceSetToggle(Toggle toggle, bool isEnabled) {
        if (!mOverridenToggles.Has(toggle) && mEnabledToggles.Has(toggle) != isEnabled) {
            dawn::WarningLog() << "Forcing toggle \"" << ToggleEnumToName(toggle) << "\" to "
                               << isEnabled << " when it was overriden to be " << !isEnabled;
        }
        mEnabledToggles.Set(toggle, isEnabled);
    }

    void DeviceBase::SetDefaultToggles() {
        SetToggle(Toggle::LazyClearResourceOnFirstUse, true);
        SetToggle(Toggle::DisallowUnsafeAPIs, true);
    }

    void DeviceBase::ApplyToggleOverrides(const DeviceDescriptor* deviceDescriptor) {
        ASSERT(deviceDescriptor);

        for (const char* toggleName : deviceDescriptor->forceEnabledToggles) {
            Toggle toggle = GetAdapter()->GetInstance()->ToggleNameToEnum(toggleName);
            if (toggle != Toggle::InvalidEnum) {
                mEnabledToggles.Set(toggle, true);
                mOverridenToggles.Set(toggle, true);
            }
        }
        for (const char* toggleName : deviceDescriptor->forceDisabledToggles) {
            Toggle toggle = GetAdapter()->GetInstance()->ToggleNameToEnum(toggleName);
            if (toggle != Toggle::InvalidEnum) {
                mEnabledToggles.Set(toggle, false);
                mOverridenToggles.Set(toggle, true);
            }
        }
    }

}  // namespace dawn_native
