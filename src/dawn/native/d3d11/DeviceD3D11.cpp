// Copyright 2023 The Dawn Authors
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

#include "dawn/native/d3d11/DeviceD3D11.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <utility>

#include "dawn/common/GPUInfo.h"
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
#include "dawn/native/d3d11/SwapChainD3D11.h"
#include "dawn/native/d3d11/TextureD3D11.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d11 {
namespace {

static constexpr uint64_t kMaxDebugMessagesToPrint = 5;

void AppendDebugLayerMessagesToError(ID3D11InfoQueue* infoQueue,
                                     uint64_t totalErrors,
                                     ErrorData* error) {
    ASSERT(totalErrors > 0);
    ASSERT(error != nullptr);

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
                                          const DeviceDescriptor* descriptor,
                                          const TogglesState& deviceToggles) {
    Ref<Device> device = AcquireRef(new Device(adapter, descriptor, deviceToggles));
    DAWN_TRY(device->Initialize(descriptor));
    return device;
}

MaybeError Device::Initialize(const DeviceDescriptor* descriptor) {
    DAWN_TRY_ASSIGN(mD3d11Device, ToBackend(GetPhysicalDevice())->CreateD3D11Device());
    ASSERT(mD3d11Device != nullptr);

    DAWN_TRY(DeviceBase::Initialize(Queue::Create(this, &descriptor->defaultQueue)));

    // Get the ID3D11Device5 interface which is need for creating fences.
    // TODO(dawn:1741): Handle the case where ID3D11Device5 is not available.
    DAWN_TRY(CheckHRESULT(mD3d11Device.As(&mD3d11Device5), "D3D11: getting ID3D11Device5"));

    // Create the fence.
    DAWN_TRY(
        CheckHRESULT(mD3d11Device5->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(&mFence)),
                     "D3D11: creating fence"));

    DAWN_TRY(CheckHRESULT(mFence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &mFenceHandle),
                          "D3D11: creating fence shared handle"));

    // Create the fence event.
    mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    DAWN_TRY(mPendingCommands.Intialize(this));

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

CommandRecordingContext* Device::GetPendingCommandContext(Device::SubmitMode submitMode) {
    // Callers of GetPendingCommandList do so to record commands. Only reserve a command
    // allocator when it is needed so we don't submit empty command lists
    DAWN_ASSERT(mPendingCommands.IsOpen());

    if (submitMode == SubmitMode::Normal) {
        mPendingCommands.SetNeedsSubmit();
    }
    return &mPendingCommands;
}

MaybeError Device::TickImpl() {
    // Perform cleanup operations to free unused objects
    [[maybe_unused]] ExecutionSerial completedSerial = GetCompletedCommandSerial();

    // Check for debug layer messages before executing the command context in case we encounter an
    // error during execution and early out as a result.
    DAWN_TRY(CheckDebugLayerAndGenerateErrors());
    if (mPendingCommands.IsOpen() && mPendingCommands.NeedsSubmit()) {
        DAWN_TRY(ExecutePendingCommandContext());
        DAWN_TRY(NextSerial());
    }
    DAWN_TRY(CheckDebugLayerAndGenerateErrors());

    return {};
}

MaybeError Device::NextSerial() {
    IncrementLastSubmittedCommandSerial();

    TRACE_EVENT1(GetPlatform(), General, "D3D11Device::SignalFence", "serial",
                 uint64_t(GetLastSubmittedCommandSerial()));

    CommandRecordingContext* commandContext =
        GetPendingCommandContext(DeviceBase::SubmitMode::Passive);
    DAWN_TRY(CheckHRESULT(commandContext->GetD3D11DeviceContext4()->Signal(
                              mFence.Get(), uint64_t(GetLastSubmittedCommandSerial())),
                          "D3D11 command queue signal fence"));

    return {};
}

MaybeError Device::WaitForSerial(ExecutionSerial serial) {
    DAWN_TRY(CheckPassedSerials());
    if (GetCompletedCommandSerial() < serial) {
        DAWN_TRY(CheckHRESULT(mFence->SetEventOnCompletion(uint64_t(serial), mFenceEvent),
                              "D3D11 set event on completion"));
        WaitForSingleObject(mFenceEvent, INFINITE);
        DAWN_TRY(CheckPassedSerials());
    }
    return {};
}

ResultOrError<ExecutionSerial> Device::CheckAndUpdateCompletedSerials() {
    ExecutionSerial completedSerial = ExecutionSerial(mFence->GetCompletedValue());
    if (DAWN_UNLIKELY(completedSerial == ExecutionSerial(UINT64_MAX))) {
        // GetCompletedValue returns UINT64_MAX if the device was removed.
        // Try to query the failure reason.
        DAWN_TRY(CheckHRESULT(mD3d11Device->GetDeviceRemovedReason(),
                              "ID3D11Device::GetDeviceRemovedReason"));
        // Otherwise, return a generic device lost error.
        return DAWN_DEVICE_LOST_ERROR("Device lost");
    }

    if (completedSerial <= GetCompletedCommandSerial()) {
        return ExecutionSerial(0);
    }

    return completedSerial;
}

void Device::ReferenceUntilUnused(ComPtr<IUnknown> object) {
    mUsedComObjectRefs.Enqueue(object, GetPendingCommandSerial());
}

bool Device::HasPendingCommands() const {
    return mPendingCommands.NeedsSubmit();
}

void Device::ForceEventualFlushOfCommands() {}

MaybeError Device::ExecutePendingCommandContext() {
    return mPendingCommands.ExecuteCommandList(this);
}

ResultOrError<Ref<BindGroupBase>> Device::CreateBindGroupImpl(
    const BindGroupDescriptor* descriptor) {
    return BindGroup::Create(this, descriptor);
}

ResultOrError<Ref<BindGroupLayoutInternalBase>> Device::CreateBindGroupLayoutImpl(
    const BindGroupLayoutDescriptor* descriptor) {
    return BindGroupLayout::Create(this, descriptor);
}

ResultOrError<Ref<BufferBase>> Device::CreateBufferImpl(const BufferDescriptor* descriptor) {
    return Buffer::Create(this, descriptor);
}

ResultOrError<Ref<CommandBufferBase>> Device::CreateCommandBuffer(
    CommandEncoder* encoder,
    const CommandBufferDescriptor* descriptor) {
    return CommandBuffer::Create(encoder, descriptor);
}

Ref<ComputePipelineBase> Device::CreateUninitializedComputePipelineImpl(
    const ComputePipelineDescriptor* descriptor) {
    return ComputePipeline::CreateUninitialized(this, descriptor);
}

ResultOrError<Ref<PipelineLayoutBase>> Device::CreatePipelineLayoutImpl(
    const PipelineLayoutDescriptor* descriptor) {
    return PipelineLayout::Create(this, descriptor);
}

ResultOrError<Ref<QuerySetBase>> Device::CreateQuerySetImpl(const QuerySetDescriptor* descriptor) {
    return QuerySet::Create(this, descriptor);
}

Ref<RenderPipelineBase> Device::CreateUninitializedRenderPipelineImpl(
    const RenderPipelineDescriptor* descriptor) {
    return RenderPipeline::CreateUninitialized(this, descriptor);
}

ResultOrError<Ref<SamplerBase>> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
    return Sampler::Create(this, descriptor);
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
    return SwapChain::Create(this, surface, previousSwapChain, descriptor);
}

ResultOrError<Ref<TextureBase>> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
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

MaybeError Device::CopyFromStagingToBufferImpl(BufferBase* source,
                                               uint64_t sourceOffset,
                                               BufferBase* destination,
                                               uint64_t destinationOffset,
                                               uint64_t size) {
    CommandRecordingContext* commandContext = GetPendingCommandContext();
    return Buffer::Copy(commandContext, ToBackend(source), sourceOffset, size,
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

MaybeError Device::WaitForIdleForDestruction() {
    DAWN_TRY(NextSerial());
    // Wait for all in-flight commands to finish executing
    DAWN_TRY(WaitForSerial(GetLastSubmittedCommandSerial()));

    return {};
}

MaybeError Device::CheckDebugLayerAndGenerateErrors() {
    if (!GetPhysicalDevice()->GetInstance()->IsBackendValidationEnabled()) {
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

void Device::DestroyImpl() {
    ASSERT(GetState() == State::Disconnected);

    Base::DestroyImpl();

    if (mFenceEvent != nullptr) {
        ::CloseHandle(mFenceEvent);
        mFenceEvent = nullptr;
    }

    mPendingCommands.Release();
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

    const TextureDescriptor* textureDescriptor = FromAPI(descriptor->cTextureDescriptor);
    DAWN_TRY(
        ValidateTextureDescriptor(this, textureDescriptor, AllowMultiPlanarTextureFormat::Yes));

    DAWN_TRY_CONTEXT(d3d::ValidateTextureDescriptorCanBeWrapped(textureDescriptor),
                     "validating that a D3D11 external image can be wrapped with %s",
                     textureDescriptor);

    DAWN_TRY(ValidateTextureCanBeWrapped(d3d11Resource.Get(), textureDescriptor));

    // Shared handle is assumed to support resource sharing capability. The resource
    // shared capability tier must agree to share resources between D3D devices.
    const Format* format = GetInternalFormat(textureDescriptor->format).AcquireSuccess();
    if (format->IsMultiPlanar()) {
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

Ref<TextureBase> Device::CreateD3DExternalTexture(const TextureDescriptor* descriptor,
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

}  // namespace dawn::native::d3d11
