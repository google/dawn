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

#include <memory>

#include "dawn/native/dawn_platform.h"

#include "dawn/common/Platform.h"
#include "dawn/native/Device.h"
#include "dawn/native/QuerySet.h"
#include "dawn/native/opengl/Forward.h"
#include "dawn/native/opengl/GLFormat.h"
#include "dawn/native/opengl/OpenGLFunctions.h"

// Remove windows.h macros after glad's include of windows.h
#if DAWN_PLATFORM_IS(WINDOWS)
#include "dawn/common/windows_with_undefs.h"
#endif

using EGLImage = void*;

namespace dawn::native::opengl {

class Device final : public DeviceBase {
  public:
    class Context;
    static ResultOrError<Ref<Device>> Create(AdapterBase* adapter,
                                             const DeviceDescriptor* descriptor,
                                             const OpenGLFunctions& functions,
                                             std::unique_ptr<Context> context,
                                             const TogglesState& deviceToggles);
    ~Device() override;

    MaybeError Initialize(const DeviceDescriptor* descriptor);

    // Returns all the OpenGL entry points and ensures that the associated
    // Context is current.
    const OpenGLFunctions& GetGL() const;

    const GLFormat& GetGLFormat(const Format& format);

    MaybeError ValidateTextureCanBeWrapped(const TextureDescriptor* descriptor);
    TextureBase* CreateTextureWrappingEGLImage(const ExternalImageDescriptor* descriptor,
                                               ::EGLImage image);
    TextureBase* CreateTextureWrappingGLTexture(const ExternalImageDescriptor* descriptor,
                                                GLuint texture);

    ResultOrError<Ref<CommandBufferBase>> CreateCommandBuffer(
        CommandEncoder* encoder,
        const CommandBufferDescriptor* descriptor) override;

    MaybeError TickImpl() override;

    MaybeError CopyFromStagingToBufferImpl(BufferBase* source,
                                           uint64_t sourceOffset,
                                           BufferBase* destination,
                                           uint64_t destinationOffset,
                                           uint64_t size) override;

    MaybeError CopyFromStagingToTextureImpl(const BufferBase* source,
                                            const TextureDataLayout& src,
                                            const TextureCopy& dst,
                                            const Extent3D& copySizePixels) override;

    uint32_t GetOptimalBytesPerRowAlignment() const override;
    uint64_t GetOptimalBufferToTextureCopyOffsetAlignment() const override;

    float GetTimestampPeriodInNS() const override;

    class Context {
      public:
        virtual ~Context() {}
        virtual void MakeCurrent() = 0;
    };

  private:
    Device(AdapterBase* adapter,
           const DeviceDescriptor* descriptor,
           const OpenGLFunctions& functions,
           std::unique_ptr<Context> context,
           const TogglesState& deviceToggless);

    ResultOrError<Ref<BindGroupBase>> CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) override;
    ResultOrError<Ref<BindGroupLayoutInternalBase>> CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) override;
    ResultOrError<Ref<BufferBase>> CreateBufferImpl(const BufferDescriptor* descriptor) override;
    ResultOrError<Ref<PipelineLayoutBase>> CreatePipelineLayoutImpl(
        const PipelineLayoutDescriptor* descriptor) override;
    ResultOrError<Ref<QuerySetBase>> CreateQuerySetImpl(
        const QuerySetDescriptor* descriptor) override;
    ResultOrError<Ref<SamplerBase>> CreateSamplerImpl(const SamplerDescriptor* descriptor) override;
    ResultOrError<Ref<ShaderModuleBase>> CreateShaderModuleImpl(
        const ShaderModuleDescriptor* descriptor,
        ShaderModuleParseResult* parseResult,
        OwnedCompilationMessages* compilationMessages) override;
    ResultOrError<Ref<SwapChainBase>> CreateSwapChainImpl(
        Surface* surface,
        SwapChainBase* previousSwapChain,
        const SwapChainDescriptor* descriptor) override;
    ResultOrError<Ref<TextureBase>> CreateTextureImpl(const TextureDescriptor* descriptor) override;
    ResultOrError<Ref<TextureViewBase>> CreateTextureViewImpl(
        TextureBase* texture,
        const TextureViewDescriptor* descriptor) override;
    Ref<ComputePipelineBase> CreateUninitializedComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) override;
    Ref<RenderPipelineBase> CreateUninitializedRenderPipelineImpl(
        const RenderPipelineDescriptor* descriptor) override;

    ResultOrError<wgpu::TextureUsage> GetSupportedSurfaceUsageImpl(
        const Surface* surface) const override;

    GLenum GetBGRAInternalFormat(const OpenGLFunctions& gl) const;
    void DestroyImpl() override;

    const OpenGLFunctions mGL;

    GLFormatTable mFormatTable;
    std::unique_ptr<Context> mContext = nullptr;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_DEVICEGL_H_
