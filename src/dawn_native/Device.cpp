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

#include "dawn_native/Adapter.h"
#include "dawn_native/AttachmentState.h"
#include "dawn_native/BindGroup.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandBuffer.h"
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/ComputePipeline.h"
#include "dawn_native/DynamicUploader.h"
#include "dawn_native/ErrorData.h"
#include "dawn_native/ErrorScope.h"
#include "dawn_native/ErrorScopeTracker.h"
#include "dawn_native/Fence.h"
#include "dawn_native/FenceSignalTracker.h"
#include "dawn_native/Instance.h"
#include "dawn_native/PipelineLayout.h"
#include "dawn_native/Queue.h"
#include "dawn_native/RenderBundleEncoder.h"
#include "dawn_native/RenderPipeline.h"
#include "dawn_native/Sampler.h"
#include "dawn_native/ShaderModule.h"
#include "dawn_native/SwapChain.h"
#include "dawn_native/Texture.h"
#include "dawn_native/ValidationUtils_autogen.h"

#include <unordered_set>

namespace dawn_native {

    // DeviceBase::Caches

    // The caches are unordered_sets of pointers with special hash and compare functions
    // to compare the value of the objects, instead of the pointers.
    template <typename Object>
    using ContentLessObjectCache =
        std::unordered_set<Object*, typename Object::HashFunc, typename Object::EqualityFunc>;

    struct DeviceBase::Caches {
        ContentLessObjectCache<AttachmentStateBlueprint> attachmentStates;
        ContentLessObjectCache<BindGroupLayoutBase> bindGroupLayouts;
        ContentLessObjectCache<ComputePipelineBase> computePipelines;
        ContentLessObjectCache<PipelineLayoutBase> pipelineLayouts;
        ContentLessObjectCache<RenderPipelineBase> renderPipelines;
        ContentLessObjectCache<SamplerBase> samplers;
        ContentLessObjectCache<ShaderModuleBase> shaderModules;
    };

    // DeviceBase

    DeviceBase::DeviceBase(AdapterBase* adapter, const DeviceDescriptor* descriptor)
        : mAdapter(adapter),
          mRootErrorScope(AcquireRef(new ErrorScope())),
          mCurrentErrorScope(mRootErrorScope.Get()) {
        mCaches = std::make_unique<DeviceBase::Caches>();
        mErrorScopeTracker = std::make_unique<ErrorScopeTracker>(this);
        mFenceSignalTracker = std::make_unique<FenceSignalTracker>(this);
        mDynamicUploader = std::make_unique<DynamicUploader>(this);
        SetDefaultToggles();

        if (descriptor != nullptr) {
            ApplyExtensions(descriptor);
        }

        mFormatTable = BuildFormatTable(this);
    }

    DeviceBase::~DeviceBase() {
        // Devices must explicitly free the uploader
        ASSERT(mDynamicUploader == nullptr);
        ASSERT(mDeferredCreateBufferMappedAsyncResults.empty());

        ASSERT(mCaches->attachmentStates.empty());
        ASSERT(mCaches->bindGroupLayouts.empty());
        ASSERT(mCaches->computePipelines.empty());
        ASSERT(mCaches->pipelineLayouts.empty());
        ASSERT(mCaches->renderPipelines.empty());
        ASSERT(mCaches->samplers.empty());
        ASSERT(mCaches->shaderModules.empty());
    }

    void DeviceBase::HandleError(dawn::ErrorType type, const char* message) {
        mCurrentErrorScope->HandleError(type, message);
    }

    void DeviceBase::HandleError(ErrorData* data) {
        mCurrentErrorScope->HandleError(data);
    }

    void DeviceBase::SetUncapturedErrorCallback(dawn::ErrorCallback callback, void* userdata) {
        mRootErrorScope->SetCallback(callback, userdata);
    }

    void DeviceBase::PushErrorScope(dawn::ErrorFilter filter) {
        if (ConsumedError(ValidateErrorFilter(filter))) {
            return;
        }
        mCurrentErrorScope = AcquireRef(new ErrorScope(filter, mCurrentErrorScope.Get()));
    }

    bool DeviceBase::PopErrorScope(dawn::ErrorCallback callback, void* userdata) {
        if (DAWN_UNLIKELY(mCurrentErrorScope.Get() == mRootErrorScope.Get())) {
            return false;
        }
        mCurrentErrorScope->SetCallback(callback, userdata);
        mCurrentErrorScope = Ref<ErrorScope>(mCurrentErrorScope->GetParent());

        return true;
    }

    ErrorScope* DeviceBase::GetCurrentErrorScope() {
        ASSERT(mCurrentErrorScope.Get() != nullptr);
        return mCurrentErrorScope.Get();
    }

    MaybeError DeviceBase::ValidateObject(const ObjectBase* object) const {
        if (DAWN_UNLIKELY(object->GetDevice() != this)) {
            return DAWN_VALIDATION_ERROR("Object from a different device.");
        }
        if (DAWN_UNLIKELY(object->IsError())) {
            return DAWN_VALIDATION_ERROR("Object is an error.");
        }
        return {};
    }

    AdapterBase* DeviceBase::GetAdapter() const {
        return mAdapter;
    }

    dawn_platform::Platform* DeviceBase::GetPlatform() const {
        return GetAdapter()->GetInstance()->GetPlatform();
    }

    ErrorScopeTracker* DeviceBase::GetErrorScopeTracker() const {
        return mErrorScopeTracker.get();
    }

    FenceSignalTracker* DeviceBase::GetFenceSignalTracker() const {
        return mFenceSignalTracker.get();
    }

    ResultOrError<const Format*> DeviceBase::GetInternalFormat(dawn::TextureFormat format) const {
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

    const Format& DeviceBase::GetValidInternalFormat(dawn::TextureFormat format) const {
        size_t index = ComputeFormatIndex(format);
        ASSERT(index < mFormatTable.size());
        ASSERT(mFormatTable[index].isSupported);
        return mFormatTable[index];
    }

    ResultOrError<BindGroupLayoutBase*> DeviceBase::GetOrCreateBindGroupLayout(
        const BindGroupLayoutDescriptor* descriptor) {
        BindGroupLayoutBase blueprint(this, descriptor, true);

        auto iter = mCaches->bindGroupLayouts.find(&blueprint);
        if (iter != mCaches->bindGroupLayouts.end()) {
            (*iter)->Reference();
            return *iter;
        }

        BindGroupLayoutBase* backendObj;
        DAWN_TRY_ASSIGN(backendObj, CreateBindGroupLayoutImpl(descriptor));
        mCaches->bindGroupLayouts.insert(backendObj);
        return backendObj;
    }

    void DeviceBase::UncacheBindGroupLayout(BindGroupLayoutBase* obj) {
        size_t removedCount = mCaches->bindGroupLayouts.erase(obj);
        ASSERT(removedCount == 1);
    }

    ResultOrError<ComputePipelineBase*> DeviceBase::GetOrCreateComputePipeline(
        const ComputePipelineDescriptor* descriptor) {
        ComputePipelineBase blueprint(this, descriptor, true);

        auto iter = mCaches->computePipelines.find(&blueprint);
        if (iter != mCaches->computePipelines.end()) {
            (*iter)->Reference();
            return *iter;
        }

        ComputePipelineBase* backendObj;
        DAWN_TRY_ASSIGN(backendObj, CreateComputePipelineImpl(descriptor));
        mCaches->computePipelines.insert(backendObj);
        return backendObj;
    }

    void DeviceBase::UncacheComputePipeline(ComputePipelineBase* obj) {
        size_t removedCount = mCaches->computePipelines.erase(obj);
        ASSERT(removedCount == 1);
    }

    ResultOrError<PipelineLayoutBase*> DeviceBase::GetOrCreatePipelineLayout(
        const PipelineLayoutDescriptor* descriptor) {
        PipelineLayoutBase blueprint(this, descriptor, true);

        auto iter = mCaches->pipelineLayouts.find(&blueprint);
        if (iter != mCaches->pipelineLayouts.end()) {
            (*iter)->Reference();
            return *iter;
        }

        PipelineLayoutBase* backendObj;
        DAWN_TRY_ASSIGN(backendObj, CreatePipelineLayoutImpl(descriptor));
        mCaches->pipelineLayouts.insert(backendObj);
        return backendObj;
    }

    void DeviceBase::UncachePipelineLayout(PipelineLayoutBase* obj) {
        size_t removedCount = mCaches->pipelineLayouts.erase(obj);
        ASSERT(removedCount == 1);
    }

    ResultOrError<RenderPipelineBase*> DeviceBase::GetOrCreateRenderPipeline(
        const RenderPipelineDescriptor* descriptor) {
        RenderPipelineBase blueprint(this, descriptor, true);

        auto iter = mCaches->renderPipelines.find(&blueprint);
        if (iter != mCaches->renderPipelines.end()) {
            (*iter)->Reference();
            return *iter;
        }

        RenderPipelineBase* backendObj;
        DAWN_TRY_ASSIGN(backendObj, CreateRenderPipelineImpl(descriptor));
        mCaches->renderPipelines.insert(backendObj);
        return backendObj;
    }

    void DeviceBase::UncacheRenderPipeline(RenderPipelineBase* obj) {
        size_t removedCount = mCaches->renderPipelines.erase(obj);
        ASSERT(removedCount == 1);
    }

    ResultOrError<SamplerBase*> DeviceBase::GetOrCreateSampler(
        const SamplerDescriptor* descriptor) {
        SamplerBase blueprint(this, descriptor, true);

        auto iter = mCaches->samplers.find(&blueprint);
        if (iter != mCaches->samplers.end()) {
            (*iter)->Reference();
            return *iter;
        }

        SamplerBase* backendObj;
        DAWN_TRY_ASSIGN(backendObj, CreateSamplerImpl(descriptor));
        mCaches->samplers.insert(backendObj);
        return backendObj;
    }

    void DeviceBase::UncacheSampler(SamplerBase* obj) {
        size_t removedCount = mCaches->samplers.erase(obj);
        ASSERT(removedCount == 1);
    }

    ResultOrError<ShaderModuleBase*> DeviceBase::GetOrCreateShaderModule(
        const ShaderModuleDescriptor* descriptor) {
        ShaderModuleBase blueprint(this, descriptor, true);

        auto iter = mCaches->shaderModules.find(&blueprint);
        if (iter != mCaches->shaderModules.end()) {
            (*iter)->Reference();
            return *iter;
        }

        ShaderModuleBase* backendObj;
        DAWN_TRY_ASSIGN(backendObj, CreateShaderModuleImpl(descriptor));
        mCaches->shaderModules.insert(backendObj);
        return backendObj;
    }

    void DeviceBase::UncacheShaderModule(ShaderModuleBase* obj) {
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
        const RenderPassDescriptor* descriptor) {
        AttachmentStateBlueprint blueprint(descriptor);
        return GetOrCreateAttachmentState(&blueprint);
    }

    void DeviceBase::UncacheAttachmentState(AttachmentState* obj) {
        size_t removedCount = mCaches->attachmentStates.erase(obj);
        ASSERT(removedCount == 1);
    }

    // Object creation API methods

    BindGroupBase* DeviceBase::CreateBindGroup(const BindGroupDescriptor* descriptor) {
        BindGroupBase* result = nullptr;

        if (ConsumedError(CreateBindGroupInternal(&result, descriptor))) {
            return BindGroupBase::MakeError(this);
        }

        return result;
    }
    BindGroupLayoutBase* DeviceBase::CreateBindGroupLayout(
        const BindGroupLayoutDescriptor* descriptor) {
        BindGroupLayoutBase* result = nullptr;

        if (ConsumedError(CreateBindGroupLayoutInternal(&result, descriptor))) {
            return BindGroupLayoutBase::MakeError(this);
        }

        return result;
    }
    BufferBase* DeviceBase::CreateBuffer(const BufferDescriptor* descriptor) {
        BufferBase* result = nullptr;

        if (ConsumedError(CreateBufferInternal(&result, descriptor))) {
            return BufferBase::MakeError(this);
        }

        return result;
    }
    DawnCreateBufferMappedResult DeviceBase::CreateBufferMapped(
        const BufferDescriptor* descriptor) {
        BufferBase* buffer = nullptr;
        uint8_t* data = nullptr;

        uint64_t size = descriptor->size;
        if (ConsumedError(CreateBufferInternal(&buffer, descriptor)) ||
            ConsumedError(buffer->MapAtCreation(&data))) {
            // Map failed. Replace the buffer with an error buffer.
            if (buffer != nullptr) {
                delete buffer;
            }
            buffer = BufferBase::MakeErrorMapped(this, size, &data);
        }

        ASSERT(buffer != nullptr);
        if (data == nullptr) {
            // |data| may be nullptr if there was an OOM in MakeErrorMapped.
            // Non-zero dataLength and nullptr data is used to indicate there should be
            // mapped data but the allocation failed.
            ASSERT(buffer->IsError());
        } else {
            memset(data, 0, size);
        }

        DawnCreateBufferMappedResult result = {};
        result.buffer = reinterpret_cast<DawnBuffer>(buffer);
        result.data = data;
        result.dataLength = size;

        return result;
    }
    void DeviceBase::CreateBufferMappedAsync(const BufferDescriptor* descriptor,
                                             dawn::BufferCreateMappedCallback callback,
                                             void* userdata) {
        DawnCreateBufferMappedResult result = CreateBufferMapped(descriptor);

        DawnBufferMapAsyncStatus status = DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS;
        if (result.data == nullptr || result.dataLength != descriptor->size) {
            status = DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR;
        }

        DeferredCreateBufferMappedAsync deferred_info;
        deferred_info.callback = callback;
        deferred_info.status = status;
        deferred_info.result = result;
        deferred_info.userdata = userdata;

        // The callback is deferred so it matches the async behavior of WebGPU.
        mDeferredCreateBufferMappedAsyncResults.push_back(deferred_info);
    }
    CommandEncoderBase* DeviceBase::CreateCommandEncoder(
        const CommandEncoderDescriptor* descriptor) {
        return new CommandEncoderBase(this, descriptor);
    }
    ComputePipelineBase* DeviceBase::CreateComputePipeline(
        const ComputePipelineDescriptor* descriptor) {
        ComputePipelineBase* result = nullptr;

        if (ConsumedError(CreateComputePipelineInternal(&result, descriptor))) {
            return ComputePipelineBase::MakeError(this);
        }

        return result;
    }
    PipelineLayoutBase* DeviceBase::CreatePipelineLayout(
        const PipelineLayoutDescriptor* descriptor) {
        PipelineLayoutBase* result = nullptr;

        if (ConsumedError(CreatePipelineLayoutInternal(&result, descriptor))) {
            return PipelineLayoutBase::MakeError(this);
        }

        return result;
    }
    QueueBase* DeviceBase::CreateQueue() {
        QueueBase* result = nullptr;

        if (ConsumedError(CreateQueueInternal(&result))) {
            // If queue creation failure ever becomes possible, we should implement MakeError and
            // friends for them.
            UNREACHABLE();
            return nullptr;
        }

        return result;
    }
    SamplerBase* DeviceBase::CreateSampler(const SamplerDescriptor* descriptor) {
        SamplerBase* result = nullptr;

        if (ConsumedError(CreateSamplerInternal(&result, descriptor))) {
            return SamplerBase::MakeError(this);
        }

        return result;
    }
    RenderBundleEncoderBase* DeviceBase::CreateRenderBundleEncoder(
        const RenderBundleEncoderDescriptor* descriptor) {
        RenderBundleEncoderBase* result = nullptr;

        if (ConsumedError(CreateRenderBundleEncoderInternal(&result, descriptor))) {
            return RenderBundleEncoderBase::MakeError(this);
        }

        return result;
    }
    RenderPipelineBase* DeviceBase::CreateRenderPipeline(
        const RenderPipelineDescriptor* descriptor) {
        RenderPipelineBase* result = nullptr;

        if (ConsumedError(CreateRenderPipelineInternal(&result, descriptor))) {
            return RenderPipelineBase::MakeError(this);
        }

        return result;
    }
    ShaderModuleBase* DeviceBase::CreateShaderModule(const ShaderModuleDescriptor* descriptor) {
        ShaderModuleBase* result = nullptr;

        if (ConsumedError(CreateShaderModuleInternal(&result, descriptor))) {
            return ShaderModuleBase::MakeError(this);
        }

        return result;
    }
    SwapChainBase* DeviceBase::CreateSwapChain(const SwapChainDescriptor* descriptor) {
        SwapChainBase* result = nullptr;

        if (ConsumedError(CreateSwapChainInternal(&result, descriptor))) {
            return SwapChainBase::MakeError(this);
        }

        return result;
    }
    TextureBase* DeviceBase::CreateTexture(const TextureDescriptor* descriptor) {
        TextureBase* result = nullptr;

        if (ConsumedError(CreateTextureInternal(&result, descriptor))) {
            return TextureBase::MakeError(this);
        }

        return result;
    }
    TextureViewBase* DeviceBase::CreateTextureView(TextureBase* texture,
                                                   const TextureViewDescriptor* descriptor) {
        TextureViewBase* result = nullptr;

        if (ConsumedError(CreateTextureViewInternal(&result, texture, descriptor))) {
            return TextureViewBase::MakeError(this);
        }

        return result;
    }

    // Other Device API methods

    void DeviceBase::Tick() {
        TickImpl();
        {
            auto deferredResults = std::move(mDeferredCreateBufferMappedAsyncResults);
            for (const auto& deferred : deferredResults) {
                deferred.callback(deferred.status, deferred.result, deferred.userdata);
            }
        }
        mErrorScopeTracker->Tick(GetCompletedCommandSerial());
        mFenceSignalTracker->Tick(GetCompletedCommandSerial());
    }

    void DeviceBase::Reference() {
        ASSERT(mRefCount != 0);
        mRefCount++;
    }

    void DeviceBase::Release() {
        ASSERT(mRefCount != 0);
        mRefCount--;
        if (mRefCount == 0) {
            delete this;
        }
    }

    void DeviceBase::ApplyToggleOverrides(const DeviceDescriptor* deviceDescriptor) {
        ASSERT(deviceDescriptor);

        for (const char* toggleName : deviceDescriptor->forceEnabledToggles) {
            Toggle toggle = GetAdapter()->GetInstance()->ToggleNameToEnum(toggleName);
            if (toggle != Toggle::InvalidEnum) {
                mTogglesSet.SetToggle(toggle, true);
            }
        }
        for (const char* toggleName : deviceDescriptor->forceDisabledToggles) {
            Toggle toggle = GetAdapter()->GetInstance()->ToggleNameToEnum(toggleName);
            if (toggle != Toggle::InvalidEnum) {
                mTogglesSet.SetToggle(toggle, false);
            }
        }
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

    std::vector<const char*> DeviceBase::GetTogglesUsed() const {
        return mTogglesSet.GetEnabledToggleNames();
    }

    bool DeviceBase::IsExtensionEnabled(Extension extension) const {
        return mEnabledExtensions.IsEnabled(extension);
    }

    bool DeviceBase::IsToggleEnabled(Toggle toggle) const {
        return mTogglesSet.IsEnabled(toggle);
    }

    size_t DeviceBase::GetLazyClearCountForTesting() {
        return mLazyClearCountForTesting;
    }

    void DeviceBase::IncrementLazyClearCountForTesting() {
        ++mLazyClearCountForTesting;
    }

    void DeviceBase::SetDefaultToggles() {
        // Sets the default-enabled toggles
        mTogglesSet.SetToggle(Toggle::LazyClearResourceOnFirstUse, true);
    }

    // Implementation details of object creation

    MaybeError DeviceBase::CreateBindGroupInternal(BindGroupBase** result,
                                                   const BindGroupDescriptor* descriptor) {
        DAWN_TRY(ValidateBindGroupDescriptor(this, descriptor));
        DAWN_TRY_ASSIGN(*result, CreateBindGroupImpl(descriptor));
        return {};
    }

    MaybeError DeviceBase::CreateBindGroupLayoutInternal(
        BindGroupLayoutBase** result,
        const BindGroupLayoutDescriptor* descriptor) {
        DAWN_TRY(ValidateBindGroupLayoutDescriptor(this, descriptor));
        DAWN_TRY_ASSIGN(*result, GetOrCreateBindGroupLayout(descriptor));
        return {};
    }

    MaybeError DeviceBase::CreateBufferInternal(BufferBase** result,
                                                const BufferDescriptor* descriptor) {
        DAWN_TRY(ValidateBufferDescriptor(this, descriptor));
        DAWN_TRY_ASSIGN(*result, CreateBufferImpl(descriptor));
        return {};
    }

    MaybeError DeviceBase::CreateComputePipelineInternal(
        ComputePipelineBase** result,
        const ComputePipelineDescriptor* descriptor) {
        DAWN_TRY(ValidateComputePipelineDescriptor(this, descriptor));
        DAWN_TRY_ASSIGN(*result, GetOrCreateComputePipeline(descriptor));
        return {};
    }

    MaybeError DeviceBase::CreatePipelineLayoutInternal(
        PipelineLayoutBase** result,
        const PipelineLayoutDescriptor* descriptor) {
        DAWN_TRY(ValidatePipelineLayoutDescriptor(this, descriptor));
        DAWN_TRY_ASSIGN(*result, GetOrCreatePipelineLayout(descriptor));
        return {};
    }

    MaybeError DeviceBase::CreateQueueInternal(QueueBase** result) {
        DAWN_TRY_ASSIGN(*result, CreateQueueImpl());
        return {};
    }

    MaybeError DeviceBase::CreateRenderBundleEncoderInternal(
        RenderBundleEncoderBase** result,
        const RenderBundleEncoderDescriptor* descriptor) {
        DAWN_TRY(ValidateRenderBundleEncoderDescriptor(this, descriptor));
        *result = new RenderBundleEncoderBase(this, descriptor);
        return {};
    }

    MaybeError DeviceBase::CreateRenderPipelineInternal(
        RenderPipelineBase** result,
        const RenderPipelineDescriptor* descriptor) {
        DAWN_TRY(ValidateRenderPipelineDescriptor(this, descriptor));
        DAWN_TRY_ASSIGN(*result, GetOrCreateRenderPipeline(descriptor));
        return {};
    }

    MaybeError DeviceBase::CreateSamplerInternal(SamplerBase** result,
                                                 const SamplerDescriptor* descriptor) {
        DAWN_TRY(ValidateSamplerDescriptor(this, descriptor));
        DAWN_TRY_ASSIGN(*result, GetOrCreateSampler(descriptor));
        return {};
    }

    MaybeError DeviceBase::CreateShaderModuleInternal(ShaderModuleBase** result,
                                                      const ShaderModuleDescriptor* descriptor) {
        DAWN_TRY(ValidateShaderModuleDescriptor(this, descriptor));
        DAWN_TRY_ASSIGN(*result, GetOrCreateShaderModule(descriptor));
        return {};
    }

    MaybeError DeviceBase::CreateSwapChainInternal(SwapChainBase** result,
                                                   const SwapChainDescriptor* descriptor) {
        DAWN_TRY(ValidateSwapChainDescriptor(this, descriptor));
        DAWN_TRY_ASSIGN(*result, CreateSwapChainImpl(descriptor));
        return {};
    }

    MaybeError DeviceBase::CreateTextureInternal(TextureBase** result,
                                                 const TextureDescriptor* descriptor) {
        DAWN_TRY(ValidateTextureDescriptor(this, descriptor));
        DAWN_TRY_ASSIGN(*result, CreateTextureImpl(descriptor));
        return {};
    }

    MaybeError DeviceBase::CreateTextureViewInternal(TextureViewBase** result,
                                                     TextureBase* texture,
                                                     const TextureViewDescriptor* descriptor) {
        DAWN_TRY(ValidateObject(texture));
        TextureViewDescriptor desc = GetTextureViewDescriptorWithDefaults(texture, descriptor);
        DAWN_TRY(ValidateTextureViewDescriptor(texture, &desc));
        DAWN_TRY_ASSIGN(*result, CreateTextureViewImpl(texture, &desc));
        return {};
    }

    // Other implementation details

    ResultOrError<DynamicUploader*> DeviceBase::GetDynamicUploader() const {
        if (mDynamicUploader->IsEmpty()) {
            DAWN_TRY(mDynamicUploader->CreateAndAppendBuffer());
        }
        return mDynamicUploader.get();
    }

    void DeviceBase::SetToggle(Toggle toggle, bool isEnabled) {
        mTogglesSet.SetToggle(toggle, isEnabled);
    }

}  // namespace dawn_native
