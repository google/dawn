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
    class ResourceUploader;

    class Device : public DeviceBase {
      public:
        Device(id<MTLDevice> mtlDevice);
        ~Device();

        BindGroupBase* CreateBindGroup(BindGroupBuilder* builder) override;
        BlendStateBase* CreateBlendState(BlendStateBuilder* builder) override;
        BufferViewBase* CreateBufferView(BufferViewBuilder* builder) override;
        CommandBufferBase* CreateCommandBuffer(CommandBufferBuilder* builder) override;
        DepthStencilStateBase* CreateDepthStencilState(DepthStencilStateBuilder* builder) override;
        InputStateBase* CreateInputState(InputStateBuilder* builder) override;
        RenderPassDescriptorBase* CreateRenderPassDescriptor(
            RenderPassDescriptorBuilder* builder) override;
        RenderPipelineBase* CreateRenderPipeline(RenderPipelineBuilder* builder) override;
        SwapChainBase* CreateSwapChain(SwapChainBuilder* builder) override;

        Serial GetCompletedCommandSerial() const final override;
        Serial GetLastSubmittedCommandSerial() const final override;
        void TickImpl() override;

        const dawn_native::PCIInfo& GetPCIInfo() const override;

        id<MTLDevice> GetMTLDevice();

        id<MTLCommandBuffer> GetPendingCommandBuffer();
        Serial GetPendingCommandSerial() const;
        void SubmitPendingCommandBuffer();

        MapRequestTracker* GetMapTracker() const;
        ResourceUploader* GetResourceUploader() const;

      private:
        ResultOrError<BindGroupLayoutBase*> CreateBindGroupLayoutImpl(
            const BindGroupLayoutDescriptor* descriptor) override;
        ResultOrError<BufferBase*> CreateBufferImpl(const BufferDescriptor* descriptor) override;
        ResultOrError<ComputePipelineBase*> CreateComputePipelineImpl(
            const ComputePipelineDescriptor* descriptor) override;
        ResultOrError<PipelineLayoutBase*> CreatePipelineLayoutImpl(
            const PipelineLayoutDescriptor* descriptor) override;
        ResultOrError<QueueBase*> CreateQueueImpl() override;
        ResultOrError<SamplerBase*> CreateSamplerImpl(const SamplerDescriptor* descriptor) override;
        ResultOrError<ShaderModuleBase*> CreateShaderModuleImpl(
            const ShaderModuleDescriptor* descriptor) override;
        ResultOrError<TextureBase*> CreateTextureImpl(const TextureDescriptor* descriptor) override;
        ResultOrError<TextureViewBase*> CreateTextureViewImpl(
            TextureBase* texture,
            const TextureViewDescriptor* descriptor) override;
        void CollectPCIInfo();

        void OnCompletedHandler();

        id<MTLDevice> mMtlDevice = nil;
        id<MTLCommandQueue> mCommandQueue = nil;
        std::unique_ptr<MapRequestTracker> mMapTracker;
        std::unique_ptr<ResourceUploader> mResourceUploader;

        Serial mCompletedSerial = 0;
        Serial mLastSubmittedSerial = 0;
        id<MTLCommandBuffer> mPendingCommands = nil;

        dawn_native::PCIInfo mPCIInfo;
    };

}}  // namespace dawn_native::metal

#endif  // DAWNNATIVE_METAL_DEVICEMTL_H_
