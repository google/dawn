// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_OPENGL_DEVICEGL_H_
#define SRC_DAWN_NATIVE_OPENGL_DEVICEGL_H_

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "dawn/common/MutexProtected.h"
#include "dawn/native/dawn_platform.h"

#include "dawn/common/Platform.h"
#include "dawn/native/Device.h"
#include "dawn/native/ExecutionQueue.h"
#include "dawn/native/QuerySet.h"
#include "dawn/native/opengl/ContextEGL.h"
#include "dawn/native/opengl/EGLFunctions.h"
#include "dawn/native/opengl/Forward.h"
#include "dawn/native/opengl/GLFormat.h"
#include "dawn/native/opengl/OpenGLFunctions.h"

namespace dawn::native {
class AHBFunctions;
}  // namespace dawn::native

namespace dawn::native::opengl {

class Device final : public DeviceBase {
  public:
    using GLWorkFunc = std::function<MaybeError(const OpenGLFunctions&)>;

    class Context;
    static ResultOrError<Ref<Device>> Create(AdapterBase* adapter,
                                             const UnpackedPtr<DeviceDescriptor>& descriptor,
                                             const OpenGLFunctions& functions,
                                             std::unique_ptr<ContextEGL> context,
                                             const TogglesState& deviceToggles,
                                             Ref<DeviceBase::DeviceLostEvent>&& lostEvent);
    ~Device() override;

    MaybeError Initialize(const UnpackedPtr<DeviceDescriptor>& descriptor);

    // Queues up GL work to be executed later in FlushPendingGLCommands().
    // If the device is already executing GL works, the work will be executed immediately.
    // This can happen when a pending task indirectly creates temporary resources.
    template <typename Fn>
    MaybeError EnqueueGL(ExecutionQueueBase::SubmitMode submitMode, Fn&& work) {
        MarkGLUsed(submitMode);
        if (mContext->IsInScopedMakeCurrent()) {
            return work(mGL);
        }
        mPendingGLWorkList.Use([&](auto workList) { workList->push_back(std::forward<Fn>(work)); });
        return {};
    }

    template <typename Fn>
    MaybeError EnqueueGL(Fn&& work) {
        return EnqueueGL(ExecutionQueueBase::SubmitMode::Normal, std::forward<Fn>(work));
    }

    // Flush any pending GL commands enqueued via EnqueueGL().
    MaybeError FlushPendingGLCommands();

    // Shortcut for EnqueueGL + FlushPendingGLCommands.
    // Note: this will flush any pending GL work before running the provided function.
    template <typename Fn>
    MaybeError EnqueueAndFlushGL(ExecutionQueueBase::SubmitMode submitMode, Fn&& work) {
        DAWN_TRY(EnqueueGL(submitMode, std::forward<Fn>(work)));
        return FlushPendingGLCommands();
    }

    template <typename Fn>
    MaybeError EnqueueAndFlushGL(Fn&& work) {
        return EnqueueAndFlushGL(ExecutionQueueBase::SubmitMode::Normal, std::forward<Fn>(work));
    }

    // Execute the GL work immediately without flushing pending tasks.
    // This is used for tasks that don't depend on previous tasks such as resource creations.
    template <typename Fn>
    auto ExecuteGL(ExecutionQueueBase::SubmitMode submitMode, Fn&& work)
        -> std::invoke_result_t<Fn, const OpenGLFunctions&> {
        ContextEGL::ScopedMakeCurrent scopedCurrentContext;
        DAWN_TRY_ASSIGN(scopedCurrentContext, mContext->MakeCurrent());

        MarkGLUsed(submitMode);

        auto result = work(mGL);
        if (DAWN_UNLIKELY(result.IsError())) {
            return std::move(result);
        }

        DAWN_TRY(scopedCurrentContext.End());
        return std::move(result);
    }

    // TODO(451928481): remove this from public access once all places are updated to use
    // EnqueueGL().
    // Returns all the OpenGL entry points and ensures that the associated GL context is current.
    const OpenGLFunctions& GetGL(bool makeCurrent = true) const;

    // Helper functions to get access to relevant EGL objects.
    const EGLFunctions& GetEGL(bool makeCurrent) const;
    EGLDisplay GetEGLDisplay() const;
    ContextEGL* GetContext() const;

    const GLFormat& GetGLFormat(const Format& format);

    int GetMaxTextureMaxAnisotropy() const;

    MaybeError ValidateTextureCanBeWrapped(const UnpackedPtr<TextureDescriptor>& descriptor);
    Ref<TextureBase> CreateTextureWrappingEGLImage(const ExternalImageDescriptor* descriptor,
                                                   ::EGLImage image);
    Ref<TextureBase> CreateTextureWrappingGLTexture(const ExternalImageDescriptor* descriptor,
                                                    GLuint texture);

    ResultOrError<Ref<CommandBufferBase>> CreateCommandBuffer(
        CommandEncoder* encoder,
        const CommandBufferDescriptor* descriptor) override;

    MaybeError TickImpl() override;

    MaybeError CopyFromStagingToBuffer(BufferBase* source,
                                       uint64_t sourceOffset,
                                       BufferBase* destination,
                                       uint64_t destinationOffset,
                                       uint64_t size) override;

    MaybeError CopyFromStagingToTextureImpl(BufferBase* source,
                                            const TexelCopyBufferLayout& src,
                                            const TextureCopy& dst,
                                            const Extent3D& copySizePixels) override;

    uint32_t GetOptimalBytesPerRowAlignment() const override;
    uint64_t GetOptimalBufferToTextureCopyOffsetAlignment() const override;

    float GetTimestampPeriodInNS() const override;

    bool MayRequireDuplicationOfIndirectParameters() const override;
    bool ShouldApplyIndexBufferOffsetToFirstIndex() const override;

    const AHBFunctions* GetOrLoadAHBFunctions();

    const Buffer* GetInternalTextureBuiltinsUniformBuffer() const;
    const Buffer* GetInternalArrayLengthUniformBuffer() const;

  private:
    Device(AdapterBase* adapter,
           const UnpackedPtr<DeviceDescriptor>& descriptor,
           const OpenGLFunctions& functions,
           std::unique_ptr<ContextEGL> context,
           const TogglesState& deviceToggles,
           Ref<DeviceBase::DeviceLostEvent>&& lostEvent);

    ResultOrError<Ref<BindGroupBase>> CreateBindGroupImpl(
        const UnpackedPtr<BindGroupDescriptor>& descriptor) override;
    ResultOrError<Ref<BindGroupLayoutInternalBase>> CreateBindGroupLayoutImpl(
        const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor) override;
    ResultOrError<Ref<BufferBase>> CreateBufferImpl(
        const UnpackedPtr<BufferDescriptor>& descriptor) override;
    ResultOrError<Ref<PipelineLayoutBase>> CreatePipelineLayoutImpl(
        const UnpackedPtr<PipelineLayoutDescriptor>& descriptor) override;
    ResultOrError<Ref<QuerySetBase>> CreateQuerySetImpl(
        const QuerySetDescriptor* descriptor) override;
    ResultOrError<Ref<ResourceTableBase>> CreateResourceTableImpl(
        const ResourceTableDescriptor* descriptor) override;
    ResultOrError<Ref<SamplerBase>> CreateSamplerImpl(const SamplerDescriptor* descriptor) override;
    ResultOrError<Ref<ShaderModuleBase>> CreateShaderModuleImpl(
        const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
        const std::vector<tint::wgsl::Extension>& internalExtensions) override;
    ResultOrError<Ref<SwapChainBase>> CreateSwapChainImpl(
        Surface* surface,
        SwapChainBase* previousSwapChain,
        const SurfaceConfiguration* config) override;
    ResultOrError<Ref<TextureBase>> CreateTextureImpl(
        const UnpackedPtr<TextureDescriptor>& descriptor) override;
    ResultOrError<Ref<TextureViewBase>> CreateTextureViewImpl(
        TextureBase* texture,
        const UnpackedPtr<TextureViewDescriptor>& descriptor) override;
    Ref<ComputePipelineBase> CreateUninitializedComputePipelineImpl(
        const UnpackedPtr<ComputePipelineDescriptor>& descriptor) override;
    Ref<RenderPipelineBase> CreateUninitializedRenderPipelineImpl(
        const UnpackedPtr<RenderPipelineDescriptor>& descriptor) override;
    ResultOrError<Ref<SharedTextureMemoryBase>> ImportSharedTextureMemoryImpl(
        const SharedTextureMemoryDescriptor* descriptor) override;
    ResultOrError<Ref<SharedFenceBase>> ImportSharedFenceImpl(
        const SharedFenceDescriptor* descriptor) override;
    ResultOrError<Ref<TextureBase>> CreateTextureWrappingEGLImageImpl(
        const ExternalImageDescriptor* descriptor,
        ::EGLImage image);
    ResultOrError<Ref<TextureBase>> CreateTextureWrappingGLTextureImpl(
        const ExternalImageDescriptor* descriptor,
        GLuint texture);

    void DestroyImpl() override;

    void MarkGLUsed(ExecutionQueueBase::SubmitMode submitMode) const;

    const OpenGLFunctions mGL;

    MutexProtected<std::vector<GLWorkFunc>> mPendingGLWorkList;

    GLFormatTable mFormatTable;
    std::unique_ptr<ContextEGL> mContext;
    int mMaxTextureMaxAnisotropy = 0;

    // Maintain an internal uniform buffer to store extra information needed by shader emulation for
    // certain texture builtins.
    Ref<Buffer> mTextureBuiltinsBuffer;

    // Maintain an internal uniform buffer to store extra array length information if needed.
    Ref<Buffer> mArrayLengthBuffer;

#if DAWN_PLATFORM_IS(ANDROID)
    std::unique_ptr<AHBFunctions> mAHBFunctions;
#endif  // DAWN_PLATFORM_IS(ANDROID)
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_DEVICEGL_H_
