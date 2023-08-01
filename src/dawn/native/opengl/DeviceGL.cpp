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

#include "dawn/common/Log.h"

#include "dawn/native/BackendConnection.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/Instance.h"
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
#include "dawn/native/opengl/TextureGL.h"

namespace {

void KHRONOS_APIENTRY OnGLDebugMessage(GLenum source,
                                       GLenum type,
                                       GLuint id,
                                       GLenum severity,
                                       GLsizei length,
                                       const GLchar* message,
                                       const void* userParam) {
    const char* sourceText;
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            sourceText = "OpenGL";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            sourceText = "Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            sourceText = "Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            sourceText = "Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            sourceText = "Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            sourceText = "Other";
            break;
        default:
            sourceText = "UNKNOWN";
            break;
    }

    const char* severityText;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            severityText = "High";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            severityText = "Medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            severityText = "Low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            severityText = "Notification";
            break;
        default:
            severityText = "UNKNOWN";
            break;
    }

    if (type == GL_DEBUG_TYPE_ERROR) {
        dawn::WarningLog() << "OpenGL error:"
                           << "\n    Source: " << sourceText      //
                           << "\n    ID: " << id                  //
                           << "\n    Severity: " << severityText  //
                           << "\n    Message: " << message;

        // Abort on an error when in Debug mode.
        UNREACHABLE();
    }
}

}  // anonymous namespace

namespace dawn::native::opengl {

// static
ResultOrError<Ref<Device>> Device::Create(AdapterBase* adapter,
                                          const DeviceDescriptor* descriptor,
                                          const OpenGLFunctions& functions,
                                          std::unique_ptr<Context> context,
                                          const TogglesState& deviceToggles) {
    Ref<Device> device =
        AcquireRef(new Device(adapter, descriptor, functions, std::move(context), deviceToggles));
    DAWN_TRY(device->Initialize(descriptor));
    return device;
}

Device::Device(AdapterBase* adapter,
               const DeviceDescriptor* descriptor,
               const OpenGLFunctions& functions,
               std::unique_ptr<Context> context,
               const TogglesState& deviceToggles)
    : DeviceBase(adapter, descriptor, deviceToggles),
      mGL(functions),
      mContext(std::move(context)) {}

Device::~Device() {
    Destroy();
}

MaybeError Device::Initialize(const DeviceDescriptor* descriptor) {
    const OpenGLFunctions& gl = GetGL();

    mFormatTable = BuildGLFormatTable(GetBGRAInternalFormat());

    // Use the debug output functionality to get notified about GL errors
    // TODO(crbug.com/dawn/1475): add support for the KHR_debug and ARB_debug_output
    // extensions
    bool hasDebugOutput = gl.IsAtLeastGL(4, 3) || gl.IsAtLeastGLES(3, 2);

    if (GetPhysicalDevice()->GetInstance()->IsBackendValidationEnabled() && hasDebugOutput) {
        gl.Enable(GL_DEBUG_OUTPUT);
        gl.Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        // Any GL error; dangerous undefined behavior; any shader compiler and linker errors
        gl.DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr,
                               GL_TRUE);

        // Severe performance warnings; GLSL or other shader compiler and linker warnings;
        // use of currently deprecated behavior
        gl.DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr,
                               GL_TRUE);

        // Performance warnings from redundant state changes; trivial undefined behavior
        // This is disabled because we do an incredible amount of redundant state changes.
        gl.DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr,
                               GL_FALSE);

        // Any message which is not an error or performance concern
        gl.DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0,
                               nullptr, GL_FALSE);
        gl.DebugMessageCallback(&OnGLDebugMessage, nullptr);
    }

    // Set initial state.
    gl.Enable(GL_DEPTH_TEST);
    gl.Enable(GL_SCISSOR_TEST);
    gl.Enable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    if (gl.GetVersion().IsDesktop()) {
        // These are not necessary on GLES. The functionality is enabled by default, and
        // works by specifying sample counts and SRGB textures, respectively.
        gl.Enable(GL_MULTISAMPLE);
        gl.Enable(GL_FRAMEBUFFER_SRGB);
    }
    gl.Enable(GL_SAMPLE_MASK);

    return DeviceBase::Initialize(AcquireRef(new Queue(this, &descriptor->defaultQueue)));
}

const GLFormat& Device::GetGLFormat(const Format& format) {
    ASSERT(format.IsSupported());
    ASSERT(format.GetIndex() < mFormatTable.size());

    const GLFormat& result = mFormatTable[format.GetIndex()];
    ASSERT(result.isSupportedOnBackend);
    return result;
}

GLenum Device::GetBGRAInternalFormat() const {
    const OpenGLFunctions& gl = GetGL();
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
ResultOrError<Ref<BindGroupLayoutInternalBase>> Device::CreateBindGroupLayoutImpl(
    const BindGroupLayoutDescriptor* descriptor) {
    return AcquireRef(new BindGroupLayout(this, descriptor));
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
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* compilationMessages) {
    return ShaderModule::Create(this, descriptor, parseResult, compilationMessages);
}
ResultOrError<Ref<SwapChainBase>> Device::CreateSwapChainImpl(
    Surface* surface,
    SwapChainBase* previousSwapChain,
    const SwapChainDescriptor* descriptor) {
    return DAWN_VALIDATION_ERROR("New swapchains not implemented.");
}
ResultOrError<Ref<TextureBase>> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
    return Texture::Create(this, descriptor);
}
ResultOrError<Ref<TextureViewBase>> Device::CreateTextureViewImpl(
    TextureBase* texture,
    const TextureViewDescriptor* descriptor) {
    return AcquireRef(new TextureView(texture, descriptor));
}

ResultOrError<wgpu::TextureUsage> Device::GetSupportedSurfaceUsageImpl(
    const Surface* surface) const {
    wgpu::TextureUsage usages = wgpu::TextureUsage::RenderAttachment |
                                wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc |
                                wgpu::TextureUsage::CopyDst;
    return usages;
}

void Device::SubmitFenceSync() {
    if (!mHasPendingCommands) {
        return;
    }

    const OpenGLFunctions& gl = GetGL();
    GLsync sync = gl.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    IncrementLastSubmittedCommandSerial();
    mFencesInFlight.emplace(sync, GetLastSubmittedCommandSerial());

    // Reset mHasPendingCommands after GetGL() which will set mHasPendingCommands to true.
    mHasPendingCommands = false;
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
    const OpenGLFunctions& gl = GetGL();
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
        gl.DeleteTextures(1, &tex);
        HandleError(DAWN_VALIDATION_ERROR(
            "EGLImage size (width: %u, height: %u, depth: 1) doesn't match descriptor size %s.",
            width, height, &textureDescriptor->size));
        return nullptr;
    }

    // TODO(dawn:803): Validate the OpenGL texture format from the EGLImage against the format
    // in the passed-in TextureDescriptor.
    return new Texture(this, textureDescriptor, tex, TextureBase::TextureState::OwnedInternal);
}

MaybeError Device::TickImpl() {
    SubmitFenceSync();
    return {};
}

ResultOrError<ExecutionSerial> Device::CheckAndUpdateCompletedSerials() {
    ExecutionSerial fenceSerial{0};
    const OpenGLFunctions& gl = GetGL();
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

MaybeError Device::CopyFromStagingToBufferImpl(BufferBase* source,
                                               uint64_t sourceOffset,
                                               BufferBase* destination,
                                               uint64_t destinationOffset,
                                               uint64_t size) {
    return DAWN_UNIMPLEMENTED_ERROR("Device unable to copy from staging buffer.");
}

MaybeError Device::CopyFromStagingToTextureImpl(const BufferBase* source,
                                                const TextureDataLayout& src,
                                                const TextureCopy& dst,
                                                const Extent3D& copySizePixels) {
    return DAWN_UNIMPLEMENTED_ERROR("Device unable to copy from staging buffer to texture.");
}

void Device::DestroyImpl() {
    ASSERT(GetState() == State::Disconnected);
}

MaybeError Device::WaitForIdleForDestruction() {
    const OpenGLFunctions& gl = GetGL();
    gl.Finish();
    DAWN_TRY(CheckPassedSerials());
    ASSERT(mFencesInFlight.empty());

    return {};
}

bool Device::HasPendingCommands() const {
    return mHasPendingCommands;
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

void Device::ForceEventualFlushOfCommands() {}

const OpenGLFunctions& Device::GetGL() const {
    if (mContext) {
        mContext->MakeCurrent();
    }
    mHasPendingCommands = true;
    return mGL;
}

}  // namespace dawn::native::opengl
