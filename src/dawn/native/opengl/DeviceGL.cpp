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

#include "dawn/native/opengl/DeviceGL.h"

#include "dawn/native/BackendConnection.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/StagingBuffer.h"
#include "dawn/native/opengl/BindGroupGL.h"
#include "dawn/native/opengl/BindGroupLayoutGL.h"
#include "dawn/native/opengl/BufferGL.h"
#include "dawn/native/opengl/CommandBufferGL.h"
#include "dawn/native/opengl/ComputePipelineGL.h"
#include "dawn/native/opengl/PipelineLayoutGL.h"
#include "dawn/native/opengl/QuerySetGL.h"
#include "dawn/native/opengl/QueueGL.h"
#include "dawn/native/opengl/RenderPipelineGL.h"
#include "dawn/native/opengl/SamplerGL.h"
#include "dawn/native/opengl/ShaderModuleGL.h"
#include "dawn/native/opengl/SwapChainGL.h"
#include "dawn/native/opengl/TextureGL.h"

namespace dawn::native::opengl {

// static
ResultOrError<Ref<Device>> Device::Create(AdapterBase* adapter,
                                          const DeviceDescriptor* descriptor,
                                          const OpenGLFunctions& functions) {
    Ref<Device> device = AcquireRef(new Device(adapter, descriptor, functions));
    DAWN_TRY(device->Initialize(descriptor));
    return device;
}

Device::Device(AdapterBase* adapter,
               const DeviceDescriptor* descriptor,
               const OpenGLFunctions& functions)
    : DeviceBase(adapter, descriptor), gl(functions) {}

Device::~Device() {
    Destroy();
}

MaybeError Device::Initialize(const DeviceDescriptor* descriptor) {
    InitTogglesFromDriver();
    mFormatTable = BuildGLFormatTable(GetBGRAInternalFormat());

    return DeviceBase::Initialize(AcquireRef(new Queue(this, &descriptor->defaultQueue)));
}

void Device::InitTogglesFromDriver() {
    bool supportsBaseVertex = gl.IsAtLeastGLES(3, 2) || gl.IsAtLeastGL(3, 2);

    bool supportsBaseInstance = gl.IsAtLeastGLES(3, 2) || gl.IsAtLeastGL(4, 2);

    // TODO(crbug.com/dawn/582): Use OES_draw_buffers_indexed where available.
    bool supportsIndexedDrawBuffers = gl.IsAtLeastGLES(3, 2) || gl.IsAtLeastGL(3, 0);

    bool supportsSnormRead =
        gl.IsAtLeastGL(4, 4) || gl.IsGLExtensionSupported("GL_EXT_render_snorm");

    bool supportsDepthRead = gl.IsAtLeastGL(3, 0) || gl.IsGLExtensionSupported("GL_NV_read_depth");

    bool supportsStencilRead =
        gl.IsAtLeastGL(3, 0) || gl.IsGLExtensionSupported("GL_NV_read_stencil");

    bool supportsDepthStencilRead =
        gl.IsAtLeastGL(3, 0) || gl.IsGLExtensionSupported("GL_NV_read_depth_stencil");

    // Desktop GL supports BGRA textures via swizzling in the driver; ES requires an extension.
    bool supportsBGRARead =
        gl.GetVersion().IsDesktop() || gl.IsGLExtensionSupported("GL_EXT_read_format_bgra");

    bool supportsSampleVariables = gl.IsAtLeastGL(4, 0) || gl.IsAtLeastGLES(3, 2) ||
                                   gl.IsGLExtensionSupported("GL_OES_sample_variables");

    // TODO(crbug.com/dawn/343): We can support the extension variants, but need to load the EXT
    // procs without the extension suffix.
    // We'll also need emulation of shader builtins gl_BaseVertex and gl_BaseInstance.

    // supportsBaseVertex |=
    //     (gl.IsAtLeastGLES(2, 0) &&
    //      (gl.IsGLExtensionSupported("OES_draw_elements_base_vertex") ||
    //       gl.IsGLExtensionSupported("EXT_draw_elements_base_vertex"))) ||
    //     (gl.IsAtLeastGL(3, 1) && gl.IsGLExtensionSupported("ARB_draw_elements_base_vertex"));

    // supportsBaseInstance |=
    //     (gl.IsAtLeastGLES(3, 1) && gl.IsGLExtensionSupported("EXT_base_instance")) ||
    //     (gl.IsAtLeastGL(3, 1) && gl.IsGLExtensionSupported("ARB_base_instance"));

    // TODO(crbug.com/dawn/343): Investigate emulation.
    SetToggle(Toggle::DisableBaseVertex, !supportsBaseVertex);
    SetToggle(Toggle::DisableBaseInstance, !supportsBaseInstance);
    SetToggle(Toggle::DisableIndexedDrawBuffers, !supportsIndexedDrawBuffers);
    SetToggle(Toggle::DisableSnormRead, !supportsSnormRead);
    SetToggle(Toggle::DisableDepthRead, !supportsDepthRead);
    SetToggle(Toggle::DisableStencilRead, !supportsStencilRead);
    SetToggle(Toggle::DisableDepthStencilRead, !supportsDepthStencilRead);
    SetToggle(Toggle::DisableBGRARead, !supportsBGRARead);
    SetToggle(Toggle::DisableSampleVariables, !supportsSampleVariables);
    SetToggle(Toggle::FlushBeforeClientWaitSync, gl.GetVersion().IsES());
    // For OpenGL ES, we must use a placeholder fragment shader for vertex-only render pipeline.
    SetToggle(Toggle::UsePlaceholderFragmentInVertexOnlyPipeline, gl.GetVersion().IsES());
}

const GLFormat& Device::GetGLFormat(const Format& format) {
    ASSERT(format.isSupported);
    ASSERT(format.GetIndex() < mFormatTable.size());

    const GLFormat& result = mFormatTable[format.GetIndex()];
    ASSERT(result.isSupportedOnBackend);
    return result;
}

GLenum Device::GetBGRAInternalFormat() const {
    if (gl.IsGLExtensionSupported("GL_EXT_texture_format_BGRA8888") ||
        gl.IsGLExtensionSupported("GL_APPLE_texture_format_BGRA8888")) {
        return GL_BGRA8_EXT;
    } else {
        // Desktop GL will swizzle to/from RGBA8 for BGRA formats.
        return GL_RGBA8;
    }
}

ResultOrError<Ref<BindGroupBase>> Device::CreateBindGroupImpl(
    const BindGroupDescriptor* descriptor) {
    DAWN_TRY(ValidateGLBindGroupDescriptor(descriptor));
    return BindGroup::Create(this, descriptor);
}
ResultOrError<Ref<BindGroupLayoutBase>> Device::CreateBindGroupLayoutImpl(
    const BindGroupLayoutDescriptor* descriptor,
    PipelineCompatibilityToken pipelineCompatibilityToken) {
    return AcquireRef(new BindGroupLayout(this, descriptor, pipelineCompatibilityToken));
}
ResultOrError<Ref<BufferBase>> Device::CreateBufferImpl(const BufferDescriptor* descriptor) {
    return AcquireRef(new Buffer(this, descriptor));
}
ResultOrError<Ref<CommandBufferBase>> Device::CreateCommandBuffer(
    CommandEncoder* encoder,
    const CommandBufferDescriptor* descriptor) {
    return AcquireRef(new CommandBuffer(encoder, descriptor));
}
Ref<ComputePipelineBase> Device::CreateUninitializedComputePipelineImpl(
    const ComputePipelineDescriptor* descriptor) {
    return ComputePipeline::CreateUninitialized(this, descriptor);
}
ResultOrError<Ref<PipelineLayoutBase>> Device::CreatePipelineLayoutImpl(
    const PipelineLayoutDescriptor* descriptor) {
    return AcquireRef(new PipelineLayout(this, descriptor));
}
ResultOrError<Ref<QuerySetBase>> Device::CreateQuerySetImpl(const QuerySetDescriptor* descriptor) {
    return AcquireRef(new QuerySet(this, descriptor));
}
Ref<RenderPipelineBase> Device::CreateUninitializedRenderPipelineImpl(
    const RenderPipelineDescriptor* descriptor) {
    return RenderPipeline::CreateUninitialized(this, descriptor);
}
ResultOrError<Ref<SamplerBase>> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
    return AcquireRef(new Sampler(this, descriptor));
}
ResultOrError<Ref<ShaderModuleBase>> Device::CreateShaderModuleImpl(
    const ShaderModuleDescriptor* descriptor,
    ShaderModuleParseResult* parseResult) {
    return ShaderModule::Create(this, descriptor, parseResult);
}
ResultOrError<Ref<SwapChainBase>> Device::CreateSwapChainImpl(
    const SwapChainDescriptor* descriptor) {
    return AcquireRef(new SwapChain(this, descriptor));
}
ResultOrError<Ref<NewSwapChainBase>> Device::CreateSwapChainImpl(
    Surface* surface,
    NewSwapChainBase* previousSwapChain,
    const SwapChainDescriptor* descriptor) {
    return DAWN_FORMAT_VALIDATION_ERROR("New swapchains not implemented.");
}
ResultOrError<Ref<TextureBase>> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
    return AcquireRef(new Texture(this, descriptor));
}
ResultOrError<Ref<TextureViewBase>> Device::CreateTextureViewImpl(
    TextureBase* texture,
    const TextureViewDescriptor* descriptor) {
    return AcquireRef(new TextureView(texture, descriptor));
}

void Device::SubmitFenceSync() {
    GLsync sync = gl.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    IncrementLastSubmittedCommandSerial();
    mFencesInFlight.emplace(sync, GetLastSubmittedCommandSerial());
}

MaybeError Device::ValidateEGLImageCanBeWrapped(const TextureDescriptor* descriptor,
                                                ::EGLImage image) {
    DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D,
                    "Texture dimension (%s) is not %s.", descriptor->dimension,
                    wgpu::TextureDimension::e2D);

    DAWN_INVALID_IF(descriptor->mipLevelCount != 1, "Mip level count (%u) is not 1.",
                    descriptor->mipLevelCount);

    DAWN_INVALID_IF(descriptor->size.depthOrArrayLayers != 1, "Array layer count (%u) is not 1.",
                    descriptor->size.depthOrArrayLayers);

    DAWN_INVALID_IF(descriptor->sampleCount != 1, "Sample count (%u) is not 1.",
                    descriptor->sampleCount);

    DAWN_INVALID_IF(descriptor->usage &
                        (wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding),
                    "Texture usage (%s) cannot have %s or %s.", descriptor->usage,
                    wgpu::TextureUsage::TextureBinding, wgpu::TextureUsage::StorageBinding);

    return {};
}
TextureBase* Device::CreateTextureWrappingEGLImage(const ExternalImageDescriptor* descriptor,
                                                   ::EGLImage image) {
    const TextureDescriptor* textureDescriptor = FromAPI(descriptor->cTextureDescriptor);

    if (ConsumedError(ValidateTextureDescriptor(this, textureDescriptor))) {
        return nullptr;
    }
    if (ConsumedError(ValidateEGLImageCanBeWrapped(textureDescriptor, image))) {
        return nullptr;
    }

    GLuint tex;
    gl.GenTextures(1, &tex);
    gl.BindTexture(GL_TEXTURE_2D, tex);
    gl.EGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

    GLint width, height, internalFormat;
    gl.GetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    gl.GetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    gl.GetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

    if (textureDescriptor->size.width != static_cast<uint32_t>(width) ||
        textureDescriptor->size.height != static_cast<uint32_t>(height) ||
        textureDescriptor->size.depthOrArrayLayers != 1) {
        ConsumedError(DAWN_FORMAT_VALIDATION_ERROR(
            "EGLImage size (width: %u, height: %u, depth: 1) doesn't match descriptor size %s.",
            width, height, &textureDescriptor->size));
        gl.DeleteTextures(1, &tex);
        return nullptr;
    }

    // TODO(dawn:803): Validate the OpenGL texture format from the EGLImage against the format
    // in the passed-in TextureDescriptor.
    return new Texture(this, textureDescriptor, tex, TextureBase::TextureState::OwnedInternal);
}

MaybeError Device::TickImpl() {
    return {};
}

ResultOrError<ExecutionSerial> Device::CheckAndUpdateCompletedSerials() {
    ExecutionSerial fenceSerial{0};
    while (!mFencesInFlight.empty()) {
        auto [sync, tentativeSerial] = mFencesInFlight.front();

        // Fence are added in order, so we can stop searching as soon
        // as we see one that's not ready.

        // TODO(crbug.com/dawn/633): Remove this workaround after the deadlock issue is fixed.
        if (IsToggleEnabled(Toggle::FlushBeforeClientWaitSync)) {
            gl.Flush();
        }
        GLenum result = gl.ClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
        if (result == GL_TIMEOUT_EXPIRED) {
            return fenceSerial;
        }
        // Update fenceSerial since fence is ready.
        fenceSerial = tentativeSerial;

        gl.DeleteSync(sync);

        mFencesInFlight.pop();

        ASSERT(fenceSerial > GetCompletedCommandSerial());
    }
    return fenceSerial;
}

ResultOrError<std::unique_ptr<StagingBufferBase>> Device::CreateStagingBuffer(size_t size) {
    return DAWN_UNIMPLEMENTED_ERROR("Device unable to create staging buffer.");
}

MaybeError Device::CopyFromStagingToBuffer(StagingBufferBase* source,
                                           uint64_t sourceOffset,
                                           BufferBase* destination,
                                           uint64_t destinationOffset,
                                           uint64_t size) {
    return DAWN_UNIMPLEMENTED_ERROR("Device unable to copy from staging buffer.");
}

MaybeError Device::CopyFromStagingToTexture(const StagingBufferBase* source,
                                            const TextureDataLayout& src,
                                            TextureCopy* dst,
                                            const Extent3D& copySizePixels) {
    return DAWN_UNIMPLEMENTED_ERROR("Device unable to copy from staging buffer to texture.");
}

void Device::DestroyImpl() {
    ASSERT(GetState() == State::Disconnected);
}

MaybeError Device::WaitForIdleForDestruction() {
    gl.Finish();
    DAWN_TRY(CheckPassedSerials());
    ASSERT(mFencesInFlight.empty());

    return {};
}

uint32_t Device::GetOptimalBytesPerRowAlignment() const {
    return 1;
}

uint64_t Device::GetOptimalBufferToTextureCopyOffsetAlignment() const {
    return 1;
}

float Device::GetTimestampPeriodInNS() const {
    return 1.0f;
}

}  // namespace dawn::native::opengl
