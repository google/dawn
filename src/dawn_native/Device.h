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

#ifndef DAWNNATIVE_DEVICE_H_
#define DAWNNATIVE_DEVICE_H_

#include "common/Serial.h"
#include "dawn_native/Error.h"
#include "dawn_native/Extensions.h"
#include "dawn_native/Format.h"
#include "dawn_native/Forward.h"
#include "dawn_native/ObjectBase.h"
#include "dawn_native/Toggles.h"

#include "dawn_native/DawnNative.h"
#include "dawn_native/dawn_platform.h"

#include <memory>

namespace dawn_native {
    class AdapterBase;
    class AttachmentState;
    class AttachmentStateBlueprint;
    class BindGroupLayoutBase;
    class DynamicUploader;
    class ErrorScope;
    class ErrorScopeTracker;
    class FenceSignalTracker;
    class StagingBufferBase;

    class DeviceBase {
      public:
        DeviceBase(AdapterBase* adapter, const DeviceDescriptor* descriptor);
        virtual ~DeviceBase();

        void HandleError(InternalErrorType type, const char* message);

        bool ConsumedError(MaybeError maybeError) {
            if (DAWN_UNLIKELY(maybeError.IsError())) {
                ConsumeError(maybeError.AcquireError());
                return true;
            }
            return false;
        }

        template <typename T>
        bool ConsumedError(ResultOrError<T> resultOrError, T* result) {
            if (DAWN_UNLIKELY(resultOrError.IsError())) {
                ConsumeError(resultOrError.AcquireError());
                return true;
            }
            *result = resultOrError.AcquireSuccess();
            return false;
        }

        MaybeError ValidateObject(const ObjectBase* object) const;

        AdapterBase* GetAdapter() const;
        dawn_platform::Platform* GetPlatform() const;

        ErrorScopeTracker* GetErrorScopeTracker() const;
        FenceSignalTracker* GetFenceSignalTracker() const;

        // Returns the Format corresponding to the wgpu::TextureFormat or an error if the format
        // isn't a valid wgpu::TextureFormat or isn't supported by this device.
        // The pointer returned has the same lifetime as the device.
        ResultOrError<const Format*> GetInternalFormat(wgpu::TextureFormat format) const;

        // Returns the Format corresponding to the wgpu::TextureFormat and assumes the format is
        // valid and supported.
        // The reference returned has the same lifetime as the device.
        const Format& GetValidInternalFormat(wgpu::TextureFormat format) const;

        virtual CommandBufferBase* CreateCommandBuffer(
            CommandEncoder* encoder,
            const CommandBufferDescriptor* descriptor) = 0;

        virtual Serial GetCompletedCommandSerial() const = 0;
        virtual Serial GetLastSubmittedCommandSerial() const = 0;
        virtual Serial GetPendingCommandSerial() const = 0;
        virtual MaybeError TickImpl() = 0;

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
        ResultOrError<BindGroupLayoutBase*> GetOrCreateBindGroupLayout(
            const BindGroupLayoutDescriptor* descriptor);
        void UncacheBindGroupLayout(BindGroupLayoutBase* obj);

        ResultOrError<ComputePipelineBase*> GetOrCreateComputePipeline(
            const ComputePipelineDescriptor* descriptor);
        void UncacheComputePipeline(ComputePipelineBase* obj);

        ResultOrError<PipelineLayoutBase*> GetOrCreatePipelineLayout(
            const PipelineLayoutDescriptor* descriptor);
        void UncachePipelineLayout(PipelineLayoutBase* obj);

        ResultOrError<RenderPipelineBase*> GetOrCreateRenderPipeline(
            const RenderPipelineDescriptor* descriptor);
        void UncacheRenderPipeline(RenderPipelineBase* obj);

        ResultOrError<SamplerBase*> GetOrCreateSampler(const SamplerDescriptor* descriptor);
        void UncacheSampler(SamplerBase* obj);

        ResultOrError<ShaderModuleBase*> GetOrCreateShaderModule(
            const ShaderModuleDescriptor* descriptor);
        void UncacheShaderModule(ShaderModuleBase* obj);

        Ref<AttachmentState> GetOrCreateAttachmentState(AttachmentStateBlueprint* blueprint);
        Ref<AttachmentState> GetOrCreateAttachmentState(
            const RenderBundleEncoderDescriptor* descriptor);
        Ref<AttachmentState> GetOrCreateAttachmentState(const RenderPipelineDescriptor* descriptor);
        Ref<AttachmentState> GetOrCreateAttachmentState(const RenderPassDescriptor* descriptor);
        void UncacheAttachmentState(AttachmentState* obj);

        // Dawn API
        BindGroupBase* CreateBindGroup(const BindGroupDescriptor* descriptor);
        BindGroupLayoutBase* CreateBindGroupLayout(const BindGroupLayoutDescriptor* descriptor);
        BufferBase* CreateBuffer(const BufferDescriptor* descriptor);
        WGPUCreateBufferMappedResult CreateBufferMapped(const BufferDescriptor* descriptor);
        void CreateBufferMappedAsync(const BufferDescriptor* descriptor,
                                     wgpu::BufferCreateMappedCallback callback,
                                     void* userdata);
        CommandEncoder* CreateCommandEncoder(const CommandEncoderDescriptor* descriptor);
        ComputePipelineBase* CreateComputePipeline(const ComputePipelineDescriptor* descriptor);
        PipelineLayoutBase* CreatePipelineLayout(const PipelineLayoutDescriptor* descriptor);
        QueueBase* CreateQueue();
        RenderBundleEncoder* CreateRenderBundleEncoder(
            const RenderBundleEncoderDescriptor* descriptor);
        RenderPipelineBase* CreateRenderPipeline(const RenderPipelineDescriptor* descriptor);
        SamplerBase* CreateSampler(const SamplerDescriptor* descriptor);
        ShaderModuleBase* CreateShaderModule(const ShaderModuleDescriptor* descriptor);
        SwapChainBase* CreateSwapChain(Surface* surface, const SwapChainDescriptor* descriptor);
        TextureBase* CreateTexture(const TextureDescriptor* descriptor);
        TextureViewBase* CreateTextureView(TextureBase* texture,
                                           const TextureViewDescriptor* descriptor);

        void InjectError(wgpu::ErrorType type, const char* message);

        void Tick();

        void SetDeviceLostCallback(wgpu::DeviceLostCallback callback, void* userdata);
        void SetUncapturedErrorCallback(wgpu::ErrorCallback callback, void* userdata);
        void PushErrorScope(wgpu::ErrorFilter filter);
        bool PopErrorScope(wgpu::ErrorCallback callback, void* userdata);

        MaybeError ValidateIsAlive() const;

        ErrorScope* GetCurrentErrorScope();

        void Reference();
        void Release();

        virtual ResultOrError<std::unique_ptr<StagingBufferBase>> CreateStagingBuffer(
            size_t size) = 0;
        virtual MaybeError CopyFromStagingToBuffer(StagingBufferBase* source,
                                                   uint64_t sourceOffset,
                                                   BufferBase* destination,
                                                   uint64_t destinationOffset,
                                                   uint64_t size) = 0;

        DynamicUploader* GetDynamicUploader() const;

        std::vector<const char*> GetEnabledExtensions() const;
        std::vector<const char*> GetTogglesUsed() const;
        bool IsExtensionEnabled(Extension extension) const;
        bool IsToggleEnabled(Toggle toggle) const;
        bool IsValidationEnabled() const;
        size_t GetLazyClearCountForTesting();
        void IncrementLazyClearCountForTesting();
        void LoseForTesting();
        bool IsLost() const;

      protected:
        void SetToggle(Toggle toggle, bool isEnabled);
        void ApplyToggleOverrides(const DeviceDescriptor* deviceDescriptor);
        void BaseDestructor();

        std::unique_ptr<DynamicUploader> mDynamicUploader;
        // LossStatus::Alive means the device is alive and can be used normally.
        // LossStatus::BeingLost means the device is in the process of being lost and should not
        //              accept any new commands.
        // LossStatus::AlreadyLost means the device has been lost and can no longer be used,
        //             all resources have been freed.
        enum class LossStatus { Alive, BeingLost, AlreadyLost };
        LossStatus mLossStatus = LossStatus::Alive;

      private:
        virtual ResultOrError<BindGroupBase*> CreateBindGroupImpl(
            const BindGroupDescriptor* descriptor) = 0;
        virtual ResultOrError<BindGroupLayoutBase*> CreateBindGroupLayoutImpl(
            const BindGroupLayoutDescriptor* descriptor) = 0;
        virtual ResultOrError<BufferBase*> CreateBufferImpl(const BufferDescriptor* descriptor) = 0;
        virtual ResultOrError<ComputePipelineBase*> CreateComputePipelineImpl(
            const ComputePipelineDescriptor* descriptor) = 0;
        virtual ResultOrError<PipelineLayoutBase*> CreatePipelineLayoutImpl(
            const PipelineLayoutDescriptor* descriptor) = 0;
        virtual ResultOrError<QueueBase*> CreateQueueImpl() = 0;
        virtual ResultOrError<RenderPipelineBase*> CreateRenderPipelineImpl(
            const RenderPipelineDescriptor* descriptor) = 0;
        virtual ResultOrError<SamplerBase*> CreateSamplerImpl(
            const SamplerDescriptor* descriptor) = 0;
        virtual ResultOrError<ShaderModuleBase*> CreateShaderModuleImpl(
            const ShaderModuleDescriptor* descriptor) = 0;
        virtual ResultOrError<SwapChainBase*> CreateSwapChainImpl(
            const SwapChainDescriptor* descriptor) = 0;
        // Note that previousSwapChain may be nullptr, or come from a different backend.
        virtual ResultOrError<NewSwapChainBase*> CreateSwapChainImpl(
            Surface* surface,
            NewSwapChainBase* previousSwapChain,
            const SwapChainDescriptor* descriptor) = 0;
        virtual ResultOrError<TextureBase*> CreateTextureImpl(
            const TextureDescriptor* descriptor) = 0;
        virtual ResultOrError<TextureViewBase*> CreateTextureViewImpl(
            TextureBase* texture,
            const TextureViewDescriptor* descriptor) = 0;

        MaybeError CreateBindGroupInternal(BindGroupBase** result,
                                           const BindGroupDescriptor* descriptor);
        MaybeError CreateBindGroupLayoutInternal(BindGroupLayoutBase** result,
                                                 const BindGroupLayoutDescriptor* descriptor);
        MaybeError CreateBufferInternal(BufferBase** result, const BufferDescriptor* descriptor);
        MaybeError CreateComputePipelineInternal(ComputePipelineBase** result,
                                                 const ComputePipelineDescriptor* descriptor);
        MaybeError CreatePipelineLayoutInternal(PipelineLayoutBase** result,
                                                const PipelineLayoutDescriptor* descriptor);
        MaybeError CreateQueueInternal(QueueBase** result);
        MaybeError CreateRenderBundleEncoderInternal(
            RenderBundleEncoder** result,
            const RenderBundleEncoderDescriptor* descriptor);
        MaybeError CreateRenderPipelineInternal(RenderPipelineBase** result,
                                                const RenderPipelineDescriptor* descriptor);
        MaybeError CreateSamplerInternal(SamplerBase** result, const SamplerDescriptor* descriptor);
        MaybeError CreateShaderModuleInternal(ShaderModuleBase** result,
                                              const ShaderModuleDescriptor* descriptor);
        MaybeError CreateSwapChainInternal(SwapChainBase** result,
                                           Surface* surface,
                                           const SwapChainDescriptor* descriptor);
        MaybeError CreateTextureInternal(TextureBase** result, const TextureDescriptor* descriptor);
        MaybeError CreateTextureViewInternal(TextureViewBase** result,
                                             TextureBase* texture,
                                             const TextureViewDescriptor* descriptor);

        void ApplyExtensions(const DeviceDescriptor* deviceDescriptor);

        void SetDefaultToggles();

        void ConsumeError(std::unique_ptr<ErrorData> error);

        // Destroy is used to clean up and release resources used by device, does not wait for GPU
        // or check errors.
        virtual void Destroy() = 0;

        // WaitForIdleForDestruction waits for GPU to finish, checks errors and gets ready for
        // destruction. This is only used when properly destructing the device. For a real
        // device loss, this function doesn't need to be called since the driver already closed all
        // resources.
        virtual MaybeError WaitForIdleForDestruction() = 0;

        void HandleLoss(const char* message);
        wgpu::DeviceLostCallback mDeviceLostCallback = nullptr;
        void* mDeviceLostUserdata;

        AdapterBase* mAdapter = nullptr;

        Ref<ErrorScope> mRootErrorScope;
        Ref<ErrorScope> mCurrentErrorScope;

        // The object caches aren't exposed in the header as they would require a lot of
        // additional includes.
        struct Caches;
        std::unique_ptr<Caches> mCaches;

        struct DeferredCreateBufferMappedAsync {
            wgpu::BufferCreateMappedCallback callback;
            WGPUBufferMapAsyncStatus status;
            WGPUCreateBufferMappedResult result;
            void* userdata;
        };

        std::unique_ptr<ErrorScopeTracker> mErrorScopeTracker;
        std::unique_ptr<FenceSignalTracker> mFenceSignalTracker;
        std::vector<DeferredCreateBufferMappedAsync> mDeferredCreateBufferMappedAsyncResults;

        uint32_t mRefCount = 1;

        FormatTable mFormatTable;

        TogglesSet mTogglesSet;
        size_t mLazyClearCountForTesting = 0;

        ExtensionsSet mEnabledExtensions;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_DEVICE_H_
