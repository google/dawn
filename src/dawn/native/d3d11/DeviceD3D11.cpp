// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/d3d11/DeviceD3D11.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <utility>

#include "dawn/common/GPUInfo.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/D3D11Backend.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/ExternalImageDXGIImpl.h"
#include "dawn/native/d3d/UtilsD3D.h"
#include "dawn/native/d3d11/BackendD3D11.h"
#include "dawn/native/d3d11/BindGroupD3D11.h"
#include "dawn/native/d3d11/BindGroupLayoutD3D11.h"
#include "dawn/native/d3d11/BufferD3D11.h"
#include "dawn/native/d3d11/CommandBufferD3D11.h"
#include "dawn/native/d3d11/ComputePipelineD3D11.h"
#include "dawn/native/d3d11/FenceD3D11.h"
#include "dawn/native/d3d11/PhysicalDeviceD3D11.h"
#include "dawn/native/d3d11/PipelineLayoutD3D11.h"
#include "dawn/native/d3d11/PlatformFunctionsD3D11.h"
#include "dawn/native/d3d11/QuerySetD3D11.h"
#include "dawn/native/d3d11/QueueD3D11.h"
#include "dawn/native/d3d11/RenderPipelineD3D11.h"
#include "dawn/native/d3d11/SamplerD3D11.h"
#include "dawn/native/d3d11/ShaderModuleD3D11.h"
#include "dawn/native/d3d11/SharedFenceD3D11.h"
#include "dawn/native/d3d11/SharedTextureMemoryD3D11.h"
#include "dawn/native/d3d11/SwapChainD3D11.h"
#include "dawn/native/d3d11/TextureD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d11 {
namespace {

static constexpr uint64_t kMaxDebugMessagesToPrint = 5;

void AppendDebugLayerMessagesToError(ID3D11InfoQueue* infoQueue,
                                     uint64_t totalErrors,
                                     ErrorData* error) {
    DAWN_ASSERT(totalErrors > 0);
    DAWN_ASSERT(error != nullptr);

    uint64_t errorsToPrint = std::min(kMaxDebugMessagesToPrint, totalErrors);
    for (uint64_t i = 0; i < errorsToPrint; ++i) {
        std::ostringstream messageStream;
        SIZE_T messageLength = 0;
        HRESULT hr = infoQueue->GetMessage(i, nullptr, &messageLength);
        if (FAILED(hr)) {
            messageStream << " ID3D11InfoQueue::GetMessage failed with " << hr;
            error->AppendBackendMessage(messageStream.str());
            continue;
        }

        std::unique_ptr<uint8_t[]> messageData(new uint8_t[messageLength]);
        D3D11_MESSAGE* message = reinterpret_cast<D3D11_MESSAGE*>(messageData.get());
        hr = infoQueue->GetMessage(i, message, &messageLength);
        if (FAILED(hr)) {
            messageStream << " ID3D11InfoQueue::GetMessage failed with " << hr;
            error->AppendBackendMessage(messageStream.str());
            continue;
        }

        messageStream << message->pDescription << " (" << message->ID << ")";
        error->AppendBackendMessage(messageStream.str());
    }
    if (errorsToPrint < totalErrors) {
        std::ostringstream messages;
        messages << (totalErrors - errorsToPrint) << " messages silenced";
        error->AppendBackendMessage(messages.str());
    }

    // We only print up to the first kMaxDebugMessagesToPrint errors
    infoQueue->ClearStoredMessages();
}

}  // namespace

// static
ResultOrError<Ref<Device>> Device::Create(AdapterBase* adapter,
                                          const UnpackedPtr<DeviceDescriptor>& descriptor,
                                          const TogglesState& deviceToggles) {
    Ref<Device> device = AcquireRef(new Device(adapter, descriptor, deviceToggles));
    DAWN_TRY(device->Initialize(descriptor));
    return device;
}

MaybeError Device::Initialize(const UnpackedPtr<DeviceDescriptor>& descriptor) {
    DAWN_TRY_ASSIGN(mD3d11Device, ToBackend(GetPhysicalDevice())->CreateD3D11Device());
    DAWN_ASSERT(mD3d11Device != nullptr);

    mIsDebugLayerEnabled = IsDebugLayerEnabled(mD3d11Device);

    // Get the ID3D11Device5 interface which is need for creating fences.
    // TODO(dawn:1741): Handle the case where ID3D11Device5 is not available.
    DAWN_TRY(CheckHRESULT(mD3d11Device.As(&mD3d11Device5), "D3D11: getting ID3D11Device5"));

    Ref<Queue> queue;
    DAWN_TRY_ASSIGN(queue, Queue::Create(this, &descriptor->defaultQueue));
    DAWN_TRY(CheckHRESULT(
        queue->GetFence()->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &mFenceHandle),
        "D3D11: creating fence shared handle"));

    DAWN_TRY(DeviceBase::Initialize(queue));
    DAWN_TRY(queue->InitializePendingContext());

    SetLabelImpl();

    return {};
}

Device::~Device() = default;

ID3D11Device* Device::GetD3D11Device() const {
    return mD3d11Device.Get();
}

ID3D11Device5* Device::GetD3D11Device5() const {
    return mD3d11Device5.Get();
}

ScopedCommandRecordingContext Device::GetScopedPendingCommandContext(SubmitMode submitMode) {
    return ToBackend(GetQueue())->GetScopedPendingCommandContext(submitMode);
}

ScopedSwapStateCommandRecordingContext Device::GetScopedSwapStatePendingCommandContext(
    SubmitMode submitMode) {
    return ToBackend(GetQueue())->GetScopedSwapStatePendingCommandContext(submitMode);
}

MaybeError Device::TickImpl() {
    // Check for debug layer messages before executing the command context in case we encounter an
    // error during execution and early out as a result.
    DAWN_TRY(CheckDebugLayerAndGenerateErrors());
    DAWN_TRY(ToBackend(GetQueue())->SubmitPendingCommands());
    return {};
}

void Device::ReferenceUntilUnused(ComPtr<IUnknown> object) {
    mUsedComObjectRefs.Enqueue(object, GetPendingCommandSerial());
}

ResultOrError<Ref<BindGroupBase>> Device::CreateBindGroupImpl(
    const BindGroupDescriptor* descriptor) {
    return BindGroup::Create(this, descriptor);
}

ResultOrError<Ref<BindGroupLayoutInternalBase>> Device::CreateBindGroupLayoutImpl(
    const BindGroupLayoutDescriptor* descriptor) {
    return BindGroupLayout::Create(this, descriptor);
}

ResultOrError<Ref<BufferBase>> Device::CreateBufferImpl(
    const UnpackedPtr<BufferDescriptor>& descriptor) {
    return Buffer::Create(this, descriptor, /*commandContext=*/nullptr);
}

ResultOrError<Ref<CommandBufferBase>> Device::CreateCommandBuffer(
    CommandEncoder* encoder,
    const CommandBufferDescriptor* descriptor) {
    return CommandBuffer::Create(encoder, descriptor);
}

Ref<ComputePipelineBase> Device::CreateUninitializedComputePipelineImpl(
    const UnpackedPtr<ComputePipelineDescriptor>& descriptor) {
    return ComputePipeline::CreateUninitialized(this, descriptor);
}

ResultOrError<Ref<PipelineLayoutBase>> Device::CreatePipelineLayoutImpl(
    const UnpackedPtr<PipelineLayoutDescriptor>& descriptor) {
    return PipelineLayout::Create(this, descriptor);
}

ResultOrError<Ref<QuerySetBase>> Device::CreateQuerySetImpl(const QuerySetDescriptor* descriptor) {
    return QuerySet::Create(this, descriptor);
}

Ref<RenderPipelineBase> Device::CreateUninitializedRenderPipelineImpl(
    const UnpackedPtr<RenderPipelineDescriptor>& descriptor) {
    return RenderPipeline::CreateUninitialized(this, descriptor);
}

ResultOrError<Ref<SamplerBase>> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
    return Sampler::Create(this, descriptor);
}

ResultOrError<Ref<ShaderModuleBase>> Device::CreateShaderModuleImpl(
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* compilationMessages) {
    return ShaderModule::Create(this, descriptor, parseResult, compilationMessages);
}

ResultOrError<Ref<SwapChainBase>> Device::CreateSwapChainImpl(
    Surface* surface,
    SwapChainBase* previousSwapChain,
    const SwapChainDescriptor* descriptor) {
    return SwapChain::Create(this, surface, previousSwapChain, descriptor);
}

ResultOrError<Ref<TextureBase>> Device::CreateTextureImpl(
    const UnpackedPtr<TextureDescriptor>& descriptor) {
    return Texture::Create(this, descriptor);
}

ResultOrError<Ref<TextureViewBase>> Device::CreateTextureViewImpl(
    TextureBase* texture,
    const TextureViewDescriptor* descriptor) {
    return TextureView::Create(texture, descriptor);
}

void Device::InitializeComputePipelineAsyncImpl(Ref<ComputePipelineBase> computePipeline,
                                                WGPUCreateComputePipelineAsyncCallback callback,
                                                void* userdata) {
    ComputePipeline::InitializeAsync(std::move(computePipeline), callback, userdata);
}

void Device::InitializeRenderPipelineAsyncImpl(Ref<RenderPipelineBase> renderPipeline,
                                               WGPUCreateRenderPipelineAsyncCallback callback,
                                               void* userdata) {
    RenderPipeline::InitializeAsync(std::move(renderPipeline), callback, userdata);
}

ResultOrError<Ref<SharedTextureMemoryBase>> Device::ImportSharedTextureMemoryImpl(
    const SharedTextureMemoryDescriptor* descriptor) {
    UnpackedPtr<SharedTextureMemoryDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    wgpu::SType type;
    DAWN_TRY_ASSIGN(
        type, (unpacked.ValidateBranches<Branch<SharedTextureMemoryDXGISharedHandleDescriptor>,
                                         Branch<SharedTextureMemoryD3D11Texture2DDescriptor>>()));

    switch (type) {
        case wgpu::SType::SharedTextureMemoryDXGISharedHandleDescriptor:
            DAWN_INVALID_IF(!HasFeature(Feature::SharedTextureMemoryDXGISharedHandle),
                            "%s is not enabled.",
                            wgpu::FeatureName::SharedTextureMemoryDXGISharedHandle);
            return SharedTextureMemory::Create(
                this, descriptor->label,
                unpacked.Get<SharedTextureMemoryDXGISharedHandleDescriptor>());
        case wgpu::SType::SharedTextureMemoryD3D11Texture2DDescriptor:
            DAWN_INVALID_IF(!HasFeature(Feature::SharedTextureMemoryD3D11Texture2D),
                            "%s is not enabled.",
                            wgpu::FeatureName::SharedTextureMemoryD3D11Texture2D);
            return SharedTextureMemory::Create(
                this, descriptor->label,
                unpacked.Get<SharedTextureMemoryD3D11Texture2DDescriptor>());
        default:
            DAWN_UNREACHABLE();
    }
}

ResultOrError<Ref<SharedFenceBase>> Device::ImportSharedFenceImpl(
    const SharedFenceDescriptor* descriptor) {
    UnpackedPtr<SharedFenceDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    wgpu::SType type;
    DAWN_TRY_ASSIGN(type,
                    (unpacked.ValidateBranches<Branch<SharedFenceDXGISharedHandleDescriptor>>()));

    switch (type) {
        case wgpu::SType::SharedFenceDXGISharedHandleDescriptor:
            DAWN_INVALID_IF(!HasFeature(Feature::SharedFenceDXGISharedHandle), "%s is not enabled.",
                            wgpu::FeatureName::SharedFenceDXGISharedHandle);
            return SharedFence::Create(this, descriptor->label,
                                       unpacked.Get<SharedFenceDXGISharedHandleDescriptor>());
        default:
            DAWN_UNREACHABLE();
    }
}

MaybeError Device::CopyFromStagingToBufferImpl(BufferBase* source,
                                               uint64_t sourceOffset,
                                               BufferBase* destination,
                                               uint64_t destinationOffset,
                                               uint64_t size) {
    // D3D11 requires that buffers are unmapped before being used in a copy.
    DAWN_TRY(source->Unmap());

    auto commandContext = GetScopedPendingCommandContext(Device::SubmitMode::Normal);
    return Buffer::Copy(&commandContext, ToBackend(source), sourceOffset, size,
                        ToBackend(destination), destinationOffset);
}

MaybeError Device::CopyFromStagingToTextureImpl(const BufferBase* source,
                                                const TextureDataLayout& src,
                                                const TextureCopy& dst,
                                                const Extent3D& copySizePixels) {
    return DAWN_UNIMPLEMENTED_ERROR("CopyFromStagingToTextureImpl");
}

const DeviceInfo& Device::GetDeviceInfo() const {
    return ToBackend(GetPhysicalDevice())->GetDeviceInfo();
}

MaybeError Device::CheckDebugLayerAndGenerateErrors() {
    if (!mIsDebugLayerEnabled) {
        return {};
    }

    ComPtr<ID3D11InfoQueue> infoQueue;
    DAWN_TRY(CheckHRESULT(mD3d11Device.As(&infoQueue),
                          "D3D11 QueryInterface ID3D11Device to ID3D11InfoQueue"));
    uint64_t totalErrors = infoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();

    // Check if any errors have occurred otherwise we would be creating an empty error. Note
    // that we use GetNumStoredMessagesAllowedByRetrievalFilter instead of GetNumStoredMessages
    // because we only convert WARNINGS or higher messages to dawn errors.
    if (totalErrors == 0) {
        return {};
    }

    auto error = DAWN_INTERNAL_ERROR("The D3D11 debug layer reported uncaught errors.");

    AppendDebugLayerMessagesToError(infoQueue.Get(), totalErrors, error.get());

    return error;
}

void Device::AppendDebugLayerMessages(ErrorData* error) {
    if (!GetPhysicalDevice()->GetInstance()->IsBackendValidationEnabled()) {
        return;
    }

    ComPtr<ID3D11InfoQueue> infoQueue;
    if (FAILED(mD3d11Device.As(&infoQueue))) {
        return;
    }
    uint64_t totalErrors = infoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();

    if (totalErrors == 0) {
        return;
    }

    AppendDebugLayerMessagesToError(infoQueue.Get(), totalErrors, error);
}

void Device::AppendDeviceLostMessage(ErrorData* error) {
    if (mD3d11Device) {
        HRESULT result = mD3d11Device->GetDeviceRemovedReason();
        error->AppendBackendMessage("Device removed reason: %s (0x%08X)",
                                    d3d::HRESULTAsString(result), result);
    }
}

void Device::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the device is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the device.
    // - It may be called when the last ref to the device is dropped and the device
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the device since there are no other live refs.
    DAWN_ASSERT(GetState() == State::Disconnected);

    Base::DestroyImpl();
}

uint32_t Device::GetOptimalBytesPerRowAlignment() const {
    return 256;
}

uint64_t Device::GetOptimalBufferToTextureCopyOffsetAlignment() const {
    return 1;
}

float Device::GetTimestampPeriodInNS() const {
    return 1.0f;
}

void Device::SetLabelImpl() {}

ResultOrError<Ref<d3d::Fence>> Device::CreateFence(
    const d3d::ExternalImageDXGIFenceDescriptor* descriptor) {
    return Fence::CreateFromHandle(mD3d11Device5.Get(), descriptor->fenceHandle,
                                   descriptor->fenceValue);
}

ResultOrError<std::unique_ptr<d3d::ExternalImageDXGIImpl>> Device::CreateExternalImageDXGIImplImpl(
    const ExternalImageDescriptor* descriptor) {
    // ExternalImageDXGIImpl holds a weak reference to the device. If the device is destroyed before
    // the image is created, the image will have a dangling reference to the device which can cause
    // a use-after-free.
    DAWN_TRY(ValidateIsAlive());

    ComPtr<ID3D11Resource> d3d11Resource;
    switch (descriptor->GetType()) {
        case ExternalImageType::DXGISharedHandle: {
            const auto* sharedHandleDescriptor =
                static_cast<const d3d::ExternalImageDescriptorDXGISharedHandle*>(descriptor);
            DAWN_TRY(CheckHRESULT(
                mD3d11Device5->OpenSharedResource1(sharedHandleDescriptor->sharedHandle,
                                                   IID_PPV_ARGS(&d3d11Resource)),
                "D3D11 OpenSharedResource1"));
            break;
        }
        case ExternalImageType::D3D11Texture: {
            const auto* d3d11TextureDescriptor =
                static_cast<const d3d::ExternalImageDescriptorD3D11Texture*>(descriptor);
            DAWN_TRY(CheckHRESULT(d3d11TextureDescriptor->texture.As(&d3d11Resource),
                                  "Cannot get ID3D11Resource from texture"));
            ComPtr<ID3D11Device> textureDevice;
            d3d11Resource->GetDevice(textureDevice.GetAddressOf());
            DAWN_INVALID_IF(
                textureDevice.Get() != mD3d11Device.Get(),
                "The D3D11 device of the texture and the D3D11 device of the WebGPU device "
                "must be same.");
            break;
        }
        default: {
            return DAWN_VALIDATION_ERROR("descriptor type (%d) is not supported",
                                         static_cast<int>(descriptor->GetType()));
        }
    }

    UnpackedPtr<TextureDescriptor> textureDescriptor;
    DAWN_TRY_ASSIGN(textureDescriptor, ValidateAndUnpack(FromAPI(descriptor->cTextureDescriptor)));
    DAWN_TRY(
        ValidateTextureDescriptor(this, textureDescriptor, AllowMultiPlanarTextureFormat::Yes));

    DAWN_TRY_CONTEXT(d3d::ValidateTextureDescriptorCanBeWrapped(textureDescriptor),
                     "validating that a D3D11 external image can be wrapped with %s",
                     textureDescriptor);

    DAWN_TRY(ValidateTextureCanBeWrapped(d3d11Resource.Get(), textureDescriptor));

    // Shared handle is assumed to support resource sharing capability. The resource
    // shared capability tier must agree to share resources between D3D devices.
    const Format* format = GetInternalFormat(textureDescriptor->format).AcquireSuccess();
    if (format->IsMultiPlanar() && descriptor->GetType() == ExternalImageType::DXGISharedHandle) {
        DAWN_TRY(ValidateVideoTextureCanBeShared(
            this, d3d::DXGITextureFormat(textureDescriptor->format)));
    }

    return std::make_unique<d3d::ExternalImageDXGIImpl>(this, std::move(d3d11Resource),
                                                        textureDescriptor);
}

bool Device::MayRequireDuplicationOfIndirectParameters() const {
    return true;
}

uint64_t Device::GetBufferCopyOffsetAlignmentForDepthStencil() const {
    return DeviceBase::GetBufferCopyOffsetAlignmentForDepthStencil();
}

bool Device::IsResolveTextureBlitWithDrawSupported() const {
    return true;
}

Ref<TextureBase> Device::CreateD3DExternalTexture(const UnpackedPtr<TextureDescriptor>& descriptor,
                                                  ComPtr<IUnknown> d3dTexture,
                                                  std::vector<Ref<d3d::Fence>> waitFences,
                                                  bool isSwapChainTexture,
                                                  bool isInitialized) {
    Ref<Texture> dawnTexture;
    if (ConsumedError(
            Texture::CreateExternalImage(this, descriptor, std::move(d3dTexture),
                                         std::move(waitFences), isSwapChainTexture, isInitialized),
            &dawnTexture)) {
        return nullptr;
    }
    return {dawnTexture};
}

uint32_t Device::GetUAVSlotCount() const {
    return ToBackend(GetPhysicalDevice())->GetUAVSlotCount();
}

ResultOrError<TextureViewBase*> Device::GetOrCreateCachedImplicitPixelLocalStorageAttachment(
    uint32_t width,
    uint32_t height,
    uint32_t implicitAttachmentIndex) {
    DAWN_ASSERT(implicitAttachmentIndex <= kMaxPLSSlots);

    TextureViewBase* currentAttachmentView =
        mImplicitPixelLocalStorageAttachmentTextureViews[implicitAttachmentIndex].Get();
    if (currentAttachmentView == nullptr ||
        currentAttachmentView->GetTexture()->GetWidth(Aspect::Color) < width ||
        currentAttachmentView->GetTexture()->GetHeight(Aspect::Color) < height) {
        // Create one 2D texture for each attachment. Note that currently on D3D11 backend we cannot
        // create a Texture2D UAV on a 2D array texture with baseArrayLayer > 0 because D3D11
        // requires the Unordered Access View dimension declared in the shader code must match the
        // view type bound to the Pixel Shader unit, while TEXTURE2D doesn't match TEXTURE2DARRAY.
        // TODO(dawn:1703): support 2D array storage textures as implicit pixel local storage
        // attachments in WGSL.
        TextureDescriptor desc;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.format = RenderPipelineBase::kImplicitPLSSlotFormat;
        desc.usage = wgpu::TextureUsage::StorageAttachment;
        desc.size = {width, height, 1};

        Ref<TextureBase> newAttachment;
        DAWN_TRY_ASSIGN(newAttachment, CreateTexture(&desc));
        DAWN_TRY_ASSIGN(mImplicitPixelLocalStorageAttachmentTextureViews[implicitAttachmentIndex],
                        newAttachment->CreateView());
    }
    return mImplicitPixelLocalStorageAttachmentTextureViews[implicitAttachmentIndex].Get();
}

}  // namespace dawn::native::d3d11
