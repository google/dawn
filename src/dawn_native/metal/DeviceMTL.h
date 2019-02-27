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

#ifndef DAWNNATIVE_METAL_DEVICEMTL_H_
#define DAWNNATIVE_METAL_DEVICEMTL_H_

#include "dawn_native/dawn_platform.h"

#include "common/Serial.h"
#include "dawn_native/Device.h"
#include "dawn_native/metal/Forward.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <memory>
#include <type_traits>

namespace dawn_native { namespace metal {

    class MapRequestTracker;

    class Device : public DeviceBase {
      public:
        Device(AdapterBase* adapter, id<MTLDevice> mtlDevice);
        ~Device();

        CommandBufferBase* CreateCommandBuffer(CommandEncoderBase* encoder) override;
        InputStateBase* CreateInputState(InputStateBuilder* builder) override;

        Serial GetCompletedCommandSerial() const final override;
        Serial GetLastSubmittedCommandSerial() const final override;
        void TickImpl() override;

        id<MTLDevice> GetMTLDevice();

        id<MTLCommandBuffer> GetPendingCommandBuffer();
        Serial GetPendingCommandSerial() const override;
        void SubmitPendingCommandBuffer();

        MapRequestTracker* GetMapTracker() const;

        ResultOrError<std::unique_ptr<StagingBufferBase>> CreateStagingBuffer(size_t size) override;
        MaybeError CopyFromStagingToBuffer(StagingBufferBase* source,
                                           uint32_t sourceOffset,
                                           BufferBase* destination,
                                           uint32_t destinationOffset,
                                           uint32_t size) override;

      private:
        ResultOrError<BindGroupBase*> CreateBindGroupImpl(
            const BindGroupDescriptor* descriptor) override;
        ResultOrError<BindGroupLayoutBase*> CreateBindGroupLayoutImpl(
            const BindGroupLayoutDescriptor* descriptor) override;
        ResultOrError<BufferBase*> CreateBufferImpl(const BufferDescriptor* descriptor) override;
        ResultOrError<ComputePipelineBase*> CreateComputePipelineImpl(
            const ComputePipelineDescriptor* descriptor) override;
        ResultOrError<PipelineLayoutBase*> CreatePipelineLayoutImpl(
            const PipelineLayoutDescriptor* descriptor) override;
        ResultOrError<QueueBase*> CreateQueueImpl() override;
        ResultOrError<RenderPipelineBase*> CreateRenderPipelineImpl(
            const RenderPipelineDescriptor* descriptor) override;
        ResultOrError<SamplerBase*> CreateSamplerImpl(const SamplerDescriptor* descriptor) override;
        ResultOrError<ShaderModuleBase*> CreateShaderModuleImpl(
            const ShaderModuleDescriptor* descriptor) override;
        ResultOrError<SwapChainBase*> CreateSwapChainImpl(
            const SwapChainDescriptor* descriptor) override;
        ResultOrError<TextureBase*> CreateTextureImpl(const TextureDescriptor* descriptor) override;
        ResultOrError<TextureViewBase*> CreateTextureViewImpl(
            TextureBase* texture,
            const TextureViewDescriptor* descriptor) override;

        void OnCompletedHandler();

        id<MTLDevice> mMtlDevice = nil;
        id<MTLCommandQueue> mCommandQueue = nil;
        std::unique_ptr<MapRequestTracker> mMapTracker;

        Serial mCompletedSerial = 0;
        Serial mLastSubmittedSerial = 0;
        id<MTLCommandBuffer> mPendingCommands = nil;
    };

}}  // namespace dawn_native::metal

#endif  // DAWNNATIVE_METAL_DEVICEMTL_H_
