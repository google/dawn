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

#include "dawn_native/BindGroup.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/MetalBackend.h"
#include "dawn_native/RenderPassDescriptor.h"
#include "dawn_native/metal/BlendStateMTL.h"
#include "dawn_native/metal/BufferMTL.h"
#include "dawn_native/metal/CommandBufferMTL.h"
#include "dawn_native/metal/ComputePipelineMTL.h"
#include "dawn_native/metal/DepthStencilStateMTL.h"
#include "dawn_native/metal/InputStateMTL.h"
#include "dawn_native/metal/PipelineLayoutMTL.h"
#include "dawn_native/metal/QueueMTL.h"
#include "dawn_native/metal/RenderPipelineMTL.h"
#include "dawn_native/metal/ResourceUploader.h"
#include "dawn_native/metal/SamplerMTL.h"
#include "dawn_native/metal/ShaderModuleMTL.h"
#include "dawn_native/metal/SwapChainMTL.h"
#include "dawn_native/metal/TextureMTL.h"

#include <unistd.h>

namespace dawn_native { namespace metal {

    dawnDevice CreateDevice(id<MTLDevice> metalDevice) {
        return reinterpret_cast<dawnDevice>(new Device(metalDevice));
    }

    // Device

    Device::Device(id<MTLDevice> mtlDevice)
        : mMtlDevice(mtlDevice),
          mMapTracker(new MapRequestTracker(this)),
          mResourceUploader(new ResourceUploader(this)) {
        [mMtlDevice retain];
        mCommandQueue = [mMtlDevice newCommandQueue];
    }

    Device::~Device() {
        // Wait for all commands to be finished so we can free resources SubmitPendingCommandBuffer
        // may not increment the pendingCommandSerial if there are no pending commands, so we can't
        // store the pendingSerial before SubmitPendingCommandBuffer then wait for it to be passed.
        // Instead we submit and wait for the serial before the next pendingCommandSerial.
        SubmitPendingCommandBuffer();
        while (mFinishedCommandSerial != mPendingCommandSerial - 1) {
            usleep(100);
        }
        Tick();

        [mPendingCommands release];
        mPendingCommands = nil;

        mMapTracker = nullptr;
        mResourceUploader = nullptr;

        [mMtlDevice release];
        mMtlDevice = nil;

        [mCommandQueue release];
        mCommandQueue = nil;
    }

    BindGroupBase* Device::CreateBindGroup(BindGroupBuilder* builder) {
        return new BindGroup(builder);
    }
    ResultOrError<BindGroupLayoutBase*> Device::CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) {
        return new BindGroupLayout(this, descriptor);
    }
    BlendStateBase* Device::CreateBlendState(BlendStateBuilder* builder) {
        return new BlendState(builder);
    }
    ResultOrError<BufferBase*> Device::CreateBufferImpl(const BufferDescriptor* descriptor) {
        return new Buffer(this, descriptor);
    }
    BufferViewBase* Device::CreateBufferView(BufferViewBuilder* builder) {
        return new BufferView(builder);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(builder);
    }
    ResultOrError<ComputePipelineBase*> Device::CreateComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) {
        return new ComputePipeline(this, descriptor);
    }
    DepthStencilStateBase* Device::CreateDepthStencilState(DepthStencilStateBuilder* builder) {
        return new DepthStencilState(builder);
    }
    InputStateBase* Device::CreateInputState(InputStateBuilder* builder) {
        return new InputState(builder);
    }
    ResultOrError<PipelineLayoutBase*> Device::CreatePipelineLayoutImpl(
        const PipelineLayoutDescriptor* descriptor) {
        return new PipelineLayout(this, descriptor);
    }
    RenderPassDescriptorBase* Device::CreateRenderPassDescriptor(
        RenderPassDescriptorBuilder* builder) {
        return new RenderPassDescriptor(builder);
    }
    RenderPipelineBase* Device::CreateRenderPipeline(RenderPipelineBuilder* builder) {
        return new RenderPipeline(builder);
    }
    ResultOrError<QueueBase*> Device::CreateQueueImpl() {
        return new Queue(this);
    }
    ResultOrError<SamplerBase*> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
        return new Sampler(this, descriptor);
    }
    ResultOrError<ShaderModuleBase*> Device::CreateShaderModuleImpl(
        const ShaderModuleDescriptor* descriptor) {
        return new ShaderModule(this, descriptor);
    }
    SwapChainBase* Device::CreateSwapChain(SwapChainBuilder* builder) {
        return new SwapChain(builder);
    }
    ResultOrError<TextureBase*> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
        return new Texture(this, descriptor);
    }
    TextureViewBase* Device::CreateDefaultTextureView(TextureBase* texture) {
        return new TextureView(texture);
    }

    void Device::TickImpl() {
        mResourceUploader->Tick(mFinishedCommandSerial);
        mMapTracker->Tick(mFinishedCommandSerial);

        // Code above might have added GPU work, submit it. This also makes sure
        // that even when no GPU work is happening, the serial number keeps incrementing.
        SubmitPendingCommandBuffer();
    }

    id<MTLDevice> Device::GetMTLDevice() {
        return mMtlDevice;
    }

    id<MTLCommandBuffer> Device::GetPendingCommandBuffer() {
        if (mPendingCommands == nil) {
            mPendingCommands = [mCommandQueue commandBuffer];
            [mPendingCommands retain];
        }
        return mPendingCommands;
    }

    void Device::SubmitPendingCommandBuffer() {
        if (mPendingCommands == nil) {
            return;
        }

        // Ok, ObjC blocks are weird. My understanding is that local variables are captured by value
        // so this-> works as expected. However it is unclear how members are captured, (are they
        // captured using this-> or by value?) so we make a copy of the pendingCommandSerial on the
        // stack.
        Serial pendingSerial = mPendingCommandSerial;
        [mPendingCommands addCompletedHandler:^(id<MTLCommandBuffer>) {
            this->mFinishedCommandSerial = pendingSerial;
        }];

        [mPendingCommands commit];
        [mPendingCommands release];
        mPendingCommands = nil;
        mPendingCommandSerial++;
    }

    uint64_t Device::GetPendingCommandSerial() {
        // If this is called, then it means some piece of code somewhere will wait for this serial
        // to complete. Make sure the pending command buffer is created so that it is on the worst
        // case enqueued on the next Tick() and eventually increments the serial. Otherwise if no
        // GPU work happens we could be waiting for this serial forever.
        GetPendingCommandBuffer();
        return mPendingCommandSerial;
    }

    MapRequestTracker* Device::GetMapTracker() const {
        return mMapTracker.get();
    }

    ResourceUploader* Device::GetResourceUploader() const {
        return mResourceUploader.get();
    }

}}  // namespace dawn_native::metal
