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

#include "dawn_native/metal/DeviceMTL.h"

#include "common/GPUInfo.h"
#include "common/Platform.h"
#include "dawn_native/BackendConnection.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Commands.h"
#include "dawn_native/ErrorData.h"
#include "dawn_native/metal/BindGroupLayoutMTL.h"
#include "dawn_native/metal/BindGroupMTL.h"
#include "dawn_native/metal/BufferMTL.h"
#include "dawn_native/metal/CommandBufferMTL.h"
#include "dawn_native/metal/ComputePipelineMTL.h"
#include "dawn_native/metal/PipelineLayoutMTL.h"
#include "dawn_native/metal/QuerySetMTL.h"
#include "dawn_native/metal/QueueMTL.h"
#include "dawn_native/metal/RenderPipelineMTL.h"
#include "dawn_native/metal/SamplerMTL.h"
#include "dawn_native/metal/ShaderModuleMTL.h"
#include "dawn_native/metal/StagingBufferMTL.h"
#include "dawn_native/metal/SwapChainMTL.h"
#include "dawn_native/metal/TextureMTL.h"
#include "dawn_native/metal/UtilsMetal.h"
#include "dawn_platform/DawnPlatform.h"
#include "dawn_platform/tracing/TraceEvent.h"

#include <type_traits>

namespace dawn_native { namespace metal {

    // static
    ResultOrError<Device*> Device::Create(AdapterBase* adapter,
                                          NSPRef<id<MTLDevice>> mtlDevice,
                                          const DeviceDescriptor* descriptor) {
        Ref<Device> device = AcquireRef(new Device(adapter, std::move(mtlDevice), descriptor));
        DAWN_TRY(device->Initialize());
        return device.Detach();
    }

    Device::Device(AdapterBase* adapter,
                   NSPRef<id<MTLDevice>> mtlDevice,
                   const DeviceDescriptor* descriptor)
        : DeviceBase(adapter, descriptor), mMtlDevice(std::move(mtlDevice)), mCompletedSerial(0) {
    }

    Device::~Device() {
        ShutDownBase();
    }

    MaybeError Device::Initialize() {
        InitTogglesFromDriver();

        if (!IsRobustnessEnabled()) {
            ForceSetToggle(Toggle::MetalEnableVertexPulling, false);
        }

        mCommandQueue.Acquire([*mMtlDevice newCommandQueue]);

        return DeviceBase::Initialize(new Queue(this));
    }

    void Device::InitTogglesFromDriver() {
        {
            bool haveStoreAndMSAAResolve = false;
#if defined(DAWN_PLATFORM_MACOS)
            if (@available(macOS 10.12, *)) {
                haveStoreAndMSAAResolve =
                    [*mMtlDevice supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v2];
            }
#elif defined(DAWN_PLATFORM_IOS)
            haveStoreAndMSAAResolve =
                [*mMtlDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v2];
#endif
            // On tvOS, we would need MTLFeatureSet_tvOS_GPUFamily2_v1.
            SetToggle(Toggle::EmulateStoreAndMSAAResolve, !haveStoreAndMSAAResolve);

            bool haveSamplerCompare = true;
#if defined(DAWN_PLATFORM_IOS)
            haveSamplerCompare = [*mMtlDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1];
#endif
            // TODO(crbug.com/dawn/342): Investigate emulation -- possibly expensive.
            SetToggle(Toggle::MetalDisableSamplerCompare, !haveSamplerCompare);

            bool haveBaseVertexBaseInstance = true;
#if defined(DAWN_PLATFORM_IOS)
            haveBaseVertexBaseInstance =
                [*mMtlDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1];
#endif
            // TODO(crbug.com/dawn/343): Investigate emulation.
            SetToggle(Toggle::DisableBaseVertex, !haveBaseVertexBaseInstance);
            SetToggle(Toggle::DisableBaseInstance, !haveBaseVertexBaseInstance);
        }

        // TODO(jiawei.shao@intel.com): tighten this workaround when the driver bug is fixed.
        SetToggle(Toggle::AlwaysResolveIntoZeroLevelAndLayer, true);

        // TODO(hao.x.li@intel.com): Use MTLStorageModeShared instead of MTLStorageModePrivate when
        // creating MTLCounterSampleBuffer in QuerySet on Intel platforms, otherwise it fails to
        // create the buffer. Change to use MTLStorageModePrivate when the bug is fixed.
        if (@available(macOS 10.15, iOS 14.0, *)) {
            bool useSharedMode = gpu_info::IsIntel(this->GetAdapter()->GetPCIInfo().vendorId);
            SetToggle(Toggle::MetalUseSharedModeForCounterSampleBuffer, useSharedMode);
        }
    }

    ResultOrError<BindGroupBase*> Device::CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) {
        return BindGroup::Create(this, descriptor);
    }
    ResultOrError<BindGroupLayoutBase*> Device::CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) {
        return new BindGroupLayout(this, descriptor);
    }
    ResultOrError<Ref<BufferBase>> Device::CreateBufferImpl(const BufferDescriptor* descriptor) {
        return Buffer::Create(this, descriptor);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandEncoder* encoder,
                                                   const CommandBufferDescriptor* descriptor) {
        return new CommandBuffer(encoder, descriptor);
    }
    ResultOrError<ComputePipelineBase*> Device::CreateComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) {
        return ComputePipeline::Create(this, descriptor);
    }
    ResultOrError<PipelineLayoutBase*> Device::CreatePipelineLayoutImpl(
        const PipelineLayoutDescriptor* descriptor) {
        return new PipelineLayout(this, descriptor);
    }
    ResultOrError<QuerySetBase*> Device::CreateQuerySetImpl(const QuerySetDescriptor* descriptor) {
        return QuerySet::Create(this, descriptor);
    }
    ResultOrError<RenderPipelineBase*> Device::CreateRenderPipelineImpl(
        const RenderPipelineDescriptor* descriptor) {
        return RenderPipeline::Create(this, descriptor);
    }
    ResultOrError<SamplerBase*> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
        return Sampler::Create(this, descriptor);
    }
    ResultOrError<ShaderModuleBase*> Device::CreateShaderModuleImpl(
        const ShaderModuleDescriptor* descriptor,
        ShaderModuleParseResult* parseResult) {
        return ShaderModule::Create(this, descriptor, parseResult);
    }
    ResultOrError<SwapChainBase*> Device::CreateSwapChainImpl(
        const SwapChainDescriptor* descriptor) {
        return new OldSwapChain(this, descriptor);
    }
    ResultOrError<NewSwapChainBase*> Device::CreateSwapChainImpl(
        Surface* surface,
        NewSwapChainBase* previousSwapChain,
        const SwapChainDescriptor* descriptor) {
        return SwapChain::Create(this, surface, previousSwapChain, descriptor);
    }
    ResultOrError<Ref<TextureBase>> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
        return AcquireRef(new Texture(this, descriptor));
    }
    ResultOrError<TextureViewBase*> Device::CreateTextureViewImpl(
        TextureBase* texture,
        const TextureViewDescriptor* descriptor) {
        return new TextureView(texture, descriptor);
    }

    ExecutionSerial Device::CheckAndUpdateCompletedSerials() {
        uint64_t frontendCompletedSerial{GetCompletedCommandSerial()};
        if (frontendCompletedSerial > mCompletedSerial) {
            // sometimes we increase the serials, in which case the completed serial in
            // the device base will surpass the completed serial we have in the metal backend, so we
            // must update ours when we see that the completed serial from device base has
            // increased.
            mCompletedSerial = frontendCompletedSerial;
        }
        return ExecutionSerial(mCompletedSerial.load());
    }

    MaybeError Device::TickImpl() {
        if (mCommandContext.GetCommands() != nullptr) {
            SubmitPendingCommandBuffer();
        }

        return {};
    }

    id<MTLDevice> Device::GetMTLDevice() {
        return mMtlDevice.Get();
    }

    id<MTLCommandQueue> Device::GetMTLQueue() {
        return mCommandQueue.Get();
    }

    CommandRecordingContext* Device::GetPendingCommandContext() {
        if (mCommandContext.GetCommands() == nullptr) {
            TRACE_EVENT0(GetPlatform(), General, "[MTLCommandQueue commandBuffer]");
            // The MTLCommandBuffer will be autoreleased by default.
            // The autorelease pool may drain before the command buffer is submitted. Retain so it
            // stays alive.
            mCommandContext = CommandRecordingContext([*mCommandQueue commandBuffer]);
        }
        return &mCommandContext;
    }

    void Device::SubmitPendingCommandBuffer() {
        if (mCommandContext.GetCommands() == nullptr) {
            return;
        }

        IncrementLastSubmittedCommandSerial();

        // Acquire the pending command buffer, which is retained. It must be released later.
        NSPRef<id<MTLCommandBuffer>> pendingCommands = mCommandContext.AcquireCommands();

        // Replace mLastSubmittedCommands with the mutex held so we avoid races between the
        // schedule handler and this code.
        {
            std::lock_guard<std::mutex> lock(mLastSubmittedCommandsMutex);
            mLastSubmittedCommands = pendingCommands;
        }

        // Make a local copy of the pointer to the commands because it's not clear how ObjC blocks
        // handle types with copy / move constructors being referenced in the block..
        id<MTLCommandBuffer> pendingCommandsPointer = pendingCommands.Get();
        [*pendingCommands addScheduledHandler:^(id<MTLCommandBuffer>) {
            // This is DRF because we hold the mutex for mLastSubmittedCommands and pendingCommands
            // is a local value (and not the member itself).
            std::lock_guard<std::mutex> lock(mLastSubmittedCommandsMutex);
            if (this->mLastSubmittedCommands.Get() == pendingCommandsPointer) {
                this->mLastSubmittedCommands = nullptr;
            }
        }];

        // Update the completed serial once the completed handler is fired. Make a local copy of
        // mLastSubmittedSerial so it is captured by value.
        ExecutionSerial pendingSerial = GetLastSubmittedCommandSerial();
        // this ObjC block runs on a different thread
        [*pendingCommands addCompletedHandler:^(id<MTLCommandBuffer>) {
            TRACE_EVENT_ASYNC_END0(GetPlatform(), GPUWork, "DeviceMTL::SubmitPendingCommandBuffer",
                                   uint64_t(pendingSerial));
            ASSERT(uint64_t(pendingSerial) > mCompletedSerial.load());
            this->mCompletedSerial = uint64_t(pendingSerial);
        }];

        TRACE_EVENT_ASYNC_BEGIN0(GetPlatform(), GPUWork, "DeviceMTL::SubmitPendingCommandBuffer",
                                 uint64_t(pendingSerial));
        [*pendingCommands commit];
    }

    ResultOrError<std::unique_ptr<StagingBufferBase>> Device::CreateStagingBuffer(size_t size) {
        std::unique_ptr<StagingBufferBase> stagingBuffer =
            std::make_unique<StagingBuffer>(size, this);
        DAWN_TRY(stagingBuffer->Initialize());
        return std::move(stagingBuffer);
    }

    MaybeError Device::CopyFromStagingToBuffer(StagingBufferBase* source,
                                               uint64_t sourceOffset,
                                               BufferBase* destination,
                                               uint64_t destinationOffset,
                                               uint64_t size) {
        // Metal validation layers forbid  0-sized copies, assert it is skipped prior to calling
        // this function.
        ASSERT(size != 0);

        ToBackend(destination)
            ->EnsureDataInitializedAsDestination(GetPendingCommandContext(), destinationOffset,
                                                 size);

        id<MTLBuffer> uploadBuffer = ToBackend(source)->GetBufferHandle();
        id<MTLBuffer> buffer = ToBackend(destination)->GetMTLBuffer();
        [GetPendingCommandContext()->EnsureBlit() copyFromBuffer:uploadBuffer
                                                    sourceOffset:sourceOffset
                                                        toBuffer:buffer
                                               destinationOffset:destinationOffset
                                                            size:size];
        return {};
    }

    // In Metal we don't write from the CPU to the texture directly which can be done using the
    // replaceRegion function, because the function requires a non-private storage mode and Dawn
    // sets the private storage mode by default for all textures except IOSurfaces on macOS.
    MaybeError Device::CopyFromStagingToTexture(const StagingBufferBase* source,
                                                const TextureDataLayout& dataLayout,
                                                TextureCopy* dst,
                                                const Extent3D& copySizePixels) {
        Texture* texture = ToBackend(dst->texture.Get());

        // This function assumes data is perfectly aligned. Otherwise, it might be necessary
        // to split copying to several stages: see ComputeTextureBufferCopySplit.
        const TexelBlockInfo& blockInfo = texture->GetFormat().GetAspectInfo(dst->aspect).block;
        ASSERT(dataLayout.rowsPerImage == copySizePixels.height / blockInfo.height);
        ASSERT(dataLayout.bytesPerRow ==
               copySizePixels.width / blockInfo.width * blockInfo.byteSize);

        EnsureDestinationTextureInitialized(texture, *dst, copySizePixels);

        // Metal validation layer requires that if the texture's pixel format is a compressed
        // format, the sourceSize must be a multiple of the pixel format's block size or be
        // clamped to the edge of the texture if the block extends outside the bounds of a
        // texture.
        const Extent3D clampedSize =
            texture->ClampToMipLevelVirtualSize(dst->mipLevel, dst->origin, copySizePixels);
        const uint32_t copyBaseLayer = dst->origin.z;
        const uint32_t copyLayerCount = copySizePixels.depth;
        const uint64_t bytesPerImage = dataLayout.rowsPerImage * dataLayout.bytesPerRow;

        MTLBlitOption blitOption = ComputeMTLBlitOption(texture->GetFormat(), dst->aspect);

        uint64_t bufferOffset = dataLayout.offset;
        for (uint32_t copyLayer = copyBaseLayer; copyLayer < copyBaseLayer + copyLayerCount;
             ++copyLayer) {
            [GetPendingCommandContext()->EnsureBlit()
                     copyFromBuffer:ToBackend(source)->GetBufferHandle()
                       sourceOffset:bufferOffset
                  sourceBytesPerRow:dataLayout.bytesPerRow
                sourceBytesPerImage:bytesPerImage
                         sourceSize:MTLSizeMake(clampedSize.width, clampedSize.height, 1)
                          toTexture:texture->GetMTLTexture()
                   destinationSlice:copyLayer
                   destinationLevel:dst->mipLevel
                  destinationOrigin:MTLOriginMake(dst->origin.x, dst->origin.y, 0)
                            options:blitOption];

            bufferOffset += bytesPerImage;
        }

        return {};
    }

    TextureBase* Device::CreateTextureWrappingIOSurface(const ExternalImageDescriptor* descriptor,
                                                        IOSurfaceRef ioSurface,
                                                        uint32_t plane) {
        const TextureDescriptor* textureDescriptor =
            reinterpret_cast<const TextureDescriptor*>(descriptor->cTextureDescriptor);

        if (ConsumedError(ValidateTextureDescriptor(this, textureDescriptor))) {
            return nullptr;
        }
        if (ConsumedError(
                ValidateIOSurfaceCanBeWrapped(this, textureDescriptor, ioSurface, plane))) {
            return nullptr;
        }

        return new Texture(this, descriptor, ioSurface, plane);
    }

    void Device::WaitForCommandsToBeScheduled() {
        SubmitPendingCommandBuffer();

        // Only lock the object while we take a reference to it, otherwise we could block further
        // progress if the driver calls the scheduled handler (which also acquires the lock) before
        // finishing the waitUntilScheduled.
        NSPRef<id<MTLCommandBuffer>> lastSubmittedCommands;
        {
            std::lock_guard<std::mutex> lock(mLastSubmittedCommandsMutex);
            lastSubmittedCommands = mLastSubmittedCommands;
        }
        [*lastSubmittedCommands waitUntilScheduled];
    }

    MaybeError Device::WaitForIdleForDestruction() {
        // Forget all pending commands.
        mCommandContext.AcquireCommands();
        CheckPassedSerials();

        // Wait for all commands to be finished so we can free resources
        while (GetCompletedCommandSerial() != GetLastSubmittedCommandSerial()) {
            usleep(100);
            CheckPassedSerials();
        }

        return {};
    }

    void Device::ShutDownImpl() {
        ASSERT(GetState() == State::Disconnected);

        // Forget all pending commands.
        mCommandContext.AcquireCommands();

        mCommandQueue = nullptr;
        mMtlDevice = nullptr;
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

}}  // namespace dawn_native::metal
