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

#include "dawn_native/BackendConnection.h"
#include "dawn_native/BindGroup.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/DynamicUploader.h"
#include "dawn_native/RenderPassDescriptor.h"
#include "dawn_native/metal/BufferMTL.h"
#include "dawn_native/metal/CommandBufferMTL.h"
#include "dawn_native/metal/ComputePipelineMTL.h"
#include "dawn_native/metal/InputStateMTL.h"
#include "dawn_native/metal/PipelineLayoutMTL.h"
#include "dawn_native/metal/QueueMTL.h"
#include "dawn_native/metal/RenderPipelineMTL.h"
#include "dawn_native/metal/SamplerMTL.h"
#include "dawn_native/metal/ShaderModuleMTL.h"
#include "dawn_native/metal/StagingBufferMTL.h"
#include "dawn_native/metal/SwapChainMTL.h"
#include "dawn_native/metal/TextureMTL.h"

namespace dawn_native { namespace metal {

    Device::Device(AdapterBase* adapter, id<MTLDevice> mtlDevice)
        : DeviceBase(adapter),
          mMtlDevice([mtlDevice retain]),
          mMapTracker(new MapRequestTracker(this)) {
        [mMtlDevice retain];
        mCommandQueue = [mMtlDevice newCommandQueue];
    }

    Device::~Device() {
        // Wait for all commands to be finished so we can free resources SubmitPendingCommandBuffer
        // may not increment the pendingCommandSerial if there are no pending commands, so we can't
        // store the pendingSerial before SubmitPendingCommandBuffer then wait for it to be passed.
        // Instead we submit and wait for the serial before the next pendingCommandSerial.
        SubmitPendingCommandBuffer();
        while (mCompletedSerial != mLastSubmittedSerial) {
            usleep(100);
        }
        Tick();

        [mPendingCommands release];
        mPendingCommands = nil;

        mMapTracker = nullptr;
        mDynamicUploader = nullptr;

        [mCommandQueue release];
        mCommandQueue = nil;

        [mMtlDevice release];
        mMtlDevice = nil;
    }

    ResultOrError<BindGroupBase*> Device::CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) {
        return new BindGroup(this, descriptor);
    }
    ResultOrError<BindGroupLayoutBase*> Device::CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) {
        return new BindGroupLayout(this, descriptor);
    }
    ResultOrError<BufferBase*> Device::CreateBufferImpl(const BufferDescriptor* descriptor) {
        return new Buffer(this, descriptor);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(builder);
    }
    ResultOrError<ComputePipelineBase*> Device::CreateComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) {
        return new ComputePipeline(this, descriptor);
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
    ResultOrError<QueueBase*> Device::CreateQueueImpl() {
        return new Queue(this);
    }
    ResultOrError<RenderPipelineBase*> Device::CreateRenderPipelineImpl(
        const RenderPipelineDescriptor* descriptor) {
        return new RenderPipeline(this, descriptor);
    }
    ResultOrError<SamplerBase*> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
        return new Sampler(this, descriptor);
    }
    ResultOrError<ShaderModuleBase*> Device::CreateShaderModuleImpl(
        const ShaderModuleDescriptor* descriptor) {
        return new ShaderModule(this, descriptor);
    }
    ResultOrError<SwapChainBase*> Device::CreateSwapChainImpl(
        const SwapChainDescriptor* descriptor) {
        return new SwapChain(this, descriptor);
    }
    ResultOrError<TextureBase*> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
        return new Texture(this, descriptor);
    }
    ResultOrError<TextureViewBase*> Device::CreateTextureViewImpl(
        TextureBase* texture,
        const TextureViewDescriptor* descriptor) {
        return new TextureView(texture, descriptor);
    }

    Serial Device::GetCompletedCommandSerial() const {
        return mCompletedSerial;
    }

    Serial Device::GetLastSubmittedCommandSerial() const {
        return mLastSubmittedSerial;
    }

    Serial Device::GetPendingCommandSerial() const {
        return mLastSubmittedSerial + 1;
    }

    void Device::TickImpl() {
        mDynamicUploader->Tick(mCompletedSerial);
        mMapTracker->Tick(mCompletedSerial);

        if (mPendingCommands != nil) {
            SubmitPendingCommandBuffer();
        } else if (mCompletedSerial == mLastSubmittedSerial) {
            // If there's no GPU work in flight we still need to artificially increment the serial
            // so that CPU operations waiting on GPU completion can know they don't have to wait.
            mCompletedSerial++;
            mLastSubmittedSerial++;
        }
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
        mLastSubmittedSerial++;
        Serial pendingSerial = mLastSubmittedSerial;
        [mPendingCommands addCompletedHandler:^(id<MTLCommandBuffer>) {
            this->mCompletedSerial = pendingSerial;
        }];

        [mPendingCommands commit];
        [mPendingCommands release];
        mPendingCommands = nil;
    }

    MapRequestTracker* Device::GetMapTracker() const {
        return mMapTracker.get();
    }

    ResultOrError<std::unique_ptr<StagingBufferBase>> Device::CreateStagingBuffer(size_t size) {
        std::unique_ptr<StagingBufferBase> stagingBuffer =
            std::make_unique<StagingBuffer>(size, this);
        return std::move(stagingBuffer);
    }

    MaybeError Device::CopyFromStagingToBuffer(StagingBufferBase* source,
                                               uint32_t sourceOffset,
                                               BufferBase* destination,
                                               uint32_t destinationOffset,
                                               uint32_t size) {
        id<MTLBuffer> uploadBuffer = ToBackend(source)->GetBufferHandle();
        id<MTLBuffer> buffer = ToBackend(destination)->GetMTLBuffer();
        id<MTLCommandBuffer> commandBuffer = GetPendingCommandBuffer();
        id<MTLBlitCommandEncoder> encoder = [commandBuffer blitCommandEncoder];
        [encoder copyFromBuffer:uploadBuffer
                   sourceOffset:sourceOffset
                       toBuffer:buffer
              destinationOffset:destinationOffset
                           size:size];
        [encoder endEncoding];

        return {};
    }

}}  // namespace dawn_native::metal
