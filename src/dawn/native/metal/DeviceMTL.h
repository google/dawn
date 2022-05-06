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

#ifndef SRC_DAWN_NATIVE_METAL_DEVICEMTL_H_
#define SRC_DAWN_NATIVE_METAL_DEVICEMTL_H_

#include <atomic>
#include <memory>
#include <mutex>

#include "dawn/native/dawn_platform.h"

#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/metal/CommandRecordingContext.h"
#include "dawn/native/metal/Forward.h"

#import <IOSurface/IOSurfaceRef.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

namespace dawn::native::metal {

struct KalmanInfo;

class Device final : public DeviceBase {
  public:
    static ResultOrError<Ref<Device>> Create(AdapterBase* adapter,
                                             NSPRef<id<MTLDevice>> mtlDevice,
                                             const DeviceDescriptor* descriptor);
    ~Device() override;

    MaybeError Initialize(const DeviceDescriptor* descriptor);

    MaybeError TickImpl() override;

    id<MTLDevice> GetMTLDevice();
    id<MTLCommandQueue> GetMTLQueue();

    CommandRecordingContext* GetPendingCommandContext();
    MaybeError SubmitPendingCommandBuffer();

    Ref<Texture> CreateTextureWrappingIOSurface(const ExternalImageDescriptor* descriptor,
                                                IOSurfaceRef ioSurface);
    void WaitForCommandsToBeScheduled();

    ResultOrError<std::unique_ptr<StagingBufferBase>> CreateStagingBuffer(size_t size) override;
    MaybeError CopyFromStagingToBuffer(StagingBufferBase* source,
                                       uint64_t sourceOffset,
                                       BufferBase* destination,
                                       uint64_t destinationOffset,
                                       uint64_t size) override;
    MaybeError CopyFromStagingToTexture(const StagingBufferBase* source,
                                        const TextureDataLayout& dataLayout,
                                        TextureCopy* dst,
                                        const Extent3D& copySizePixels) override;

    uint32_t GetOptimalBytesPerRowAlignment() const override;
    uint64_t GetOptimalBufferToTextureCopyOffsetAlignment() const override;

    float GetTimestampPeriodInNS() const override;

  private:
    Device(AdapterBase* adapter,
           NSPRef<id<MTLDevice>> mtlDevice,
           const DeviceDescriptor* descriptor);

    ResultOrError<Ref<BindGroupBase>> CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) override;
    ResultOrError<Ref<BindGroupLayoutBase>> CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor,
        PipelineCompatibilityToken pipelineCompatibilityToken) override;
    ResultOrError<Ref<BufferBase>> CreateBufferImpl(const BufferDescriptor* descriptor) override;
    ResultOrError<Ref<CommandBufferBase>> CreateCommandBuffer(
        CommandEncoder* encoder,
        const CommandBufferDescriptor* descriptor) override;
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
    void InitializeComputePipelineAsyncImpl(Ref<ComputePipelineBase> computePipeline,
                                            WGPUCreateComputePipelineAsyncCallback callback,
                                            void* userdata) override;
    void InitializeRenderPipelineAsyncImpl(Ref<RenderPipelineBase> renderPipeline,
                                           WGPUCreateRenderPipelineAsyncCallback callback,
                                           void* userdata) override;

    void InitTogglesFromDriver();
    void DestroyImpl() override;
    MaybeError WaitForIdleForDestruction() override;
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override;

    NSPRef<id<MTLDevice>> mMtlDevice;
    NSPRef<id<MTLCommandQueue>> mCommandQueue;

    CommandRecordingContext mCommandContext;

    // The completed serial is updated in a Metal completion handler that can be fired on a
    // different thread, so it needs to be atomic.
    std::atomic<uint64_t> mCompletedSerial;

    // mLastSubmittedCommands will be accessed in a Metal schedule handler that can be fired on
    // a different thread so we guard access to it with a mutex.
    std::mutex mLastSubmittedCommandsMutex;
    NSPRef<id<MTLCommandBuffer>> mLastSubmittedCommands;

    // The current estimation of timestamp period
    float mTimestampPeriod = 1.0f;
    // The base of CPU timestamp and GPU timestamp to measure the linear regression between GPU
    // and CPU timestamps.
    MTLTimestamp mCpuTimestamp API_AVAILABLE(macos(10.15), ios(14.0)) = 0;
    MTLTimestamp mGpuTimestamp API_AVAILABLE(macos(10.15), ios(14.0)) = 0;
    // The parameters for kalman filter
    std::unique_ptr<KalmanInfo> mKalmanInfo;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_DEVICEMTL_H_
