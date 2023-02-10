// Copyright 2018 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_OPENGL_DEVICEGL_H_
#define SRC_DAWN_NATIVE_OPENGL_DEVICEGL_H_

#include <memory>
#include <queue>
#include <utility>

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

typedef void* EGLImage;

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

    void SubmitFenceSync();

    MaybeError ValidateEGLImageCanBeWrapped(const TextureDescriptor* descriptor, ::EGLImage image);
    TextureBase* CreateTextureWrappingEGLImage(const ExternalImageDescriptor* descriptor,
                                               ::EGLImage image);

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
    void ForceEventualFlushOfCommands() override;

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
    ResultOrError<Ref<BindGroupLayoutBase>> CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor,
        PipelineCompatibilityToken pipelineCompatibilityToken) override;
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
        const SwapChainDescriptor* descriptor) override;
    ResultOrError<Ref<NewSwapChainBase>> CreateSwapChainImpl(
        Surface* surface,
        NewSwapChainBase* previousSwapChain,
        const SwapChainDescriptor* descriptor) override;
    ResultOrError<Ref<TextureBase>> CreateTextureImpl(const TextureDescriptor* descriptor) override;
    ResultOrError<Ref<TextureViewBase>> CreateTextureViewImpl(
        TextureBase* texture,
        const TextureViewDescriptor* descriptor) override;
    Ref<ComputePipelineBase> CreateUninitializedComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) override;
    Ref<RenderPipelineBase> CreateUninitializedRenderPipelineImpl(
        const RenderPipelineDescriptor* descriptor) override;

    GLenum GetBGRAInternalFormat() const;
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override;
    void DestroyImpl() override;
    MaybeError WaitForIdleForDestruction() override;
    bool HasPendingCommands() const override;

    const OpenGLFunctions mGL;

    std::queue<std::pair<GLsync, ExecutionSerial>> mFencesInFlight;

    GLFormatTable mFormatTable;
    std::unique_ptr<Context> mContext = nullptr;
    // Has pending GL commands which are not associated with a fence.
    mutable bool mHasPendingCommands = false;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_DEVICEGL_H_
