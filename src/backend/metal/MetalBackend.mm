// Copyright 2017 The NXT Authors
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

// TODO(kainino@chromium.org): split this backend into many files

#include "MetalBackend.h"

#include "BufferMTL.h"
#include "CommandBufferMTL.h"
#include "DepthStencilStateMTL.h"
#include "InputStateMTL.h"
#include "PipelineMTL.h"
#include "PipelineLayoutMTL.h"
#include "ResourceUploader.h"
#include "SamplerMTL.h"
#include "ShaderModuleMTL.h"
#include "TextureMTL.h"

namespace backend {
namespace metal {
    nxtProcTable GetNonValidatingProcs();
    nxtProcTable GetValidatingProcs();

    void Init(id<MTLDevice> metalDevice, nxtProcTable* procs, nxtDevice* device) {
        *device = nullptr;

        *procs = GetValidatingProcs();
        *device = reinterpret_cast<nxtDevice>(new Device(metalDevice));
    }

    void SetNextDrawable(nxtDevice device, id<CAMetalDrawable> drawable) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        backendDevice->SetNextDrawable(drawable);
    }

    void Present(nxtDevice device) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        backendDevice->Present();
    }

    // Device

    Device::Device(id<MTLDevice> mtlDevice)
        : mtlDevice(mtlDevice), mapReadTracker(new MapReadRequestTracker(this)),
            resourceUploader(new ResourceUploader(this)) {
        [mtlDevice retain];
        commandQueue = [mtlDevice newCommandQueue];
    }

    Device::~Device() {
        [pendingCommands release];
        pendingCommands = nil;

        delete mapReadTracker;
        mapReadTracker = nullptr;

        delete resourceUploader;
        resourceUploader = nullptr;

        [mtlDevice release];
        mtlDevice = nil;

        [commandQueue release];
        commandQueue = nil;

        [currentTexture release];
        currentTexture = nil;

        [currentDepthTexture release];
        currentDepthTexture = nil;
    }

    BindGroupBase* Device::CreateBindGroup(BindGroupBuilder* builder) {
        return new BindGroup(builder);
    }
    BindGroupLayoutBase* Device::CreateBindGroupLayout(BindGroupLayoutBuilder* builder) {
        return new BindGroupLayout(builder);
    }
    BufferBase* Device::CreateBuffer(BufferBuilder* builder) {
        return new Buffer(builder);
    }
    BufferViewBase* Device::CreateBufferView(BufferViewBuilder* builder) {
        return new BufferView(builder);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(builder);
    }
    DepthStencilStateBase* Device::CreateDepthStencilState(DepthStencilStateBuilder* builder) {
        return new DepthStencilState(builder);
    }
    InputStateBase* Device::CreateInputState(InputStateBuilder* builder) {
        return new InputState(builder);
    }
    FramebufferBase* Device::CreateFramebuffer(FramebufferBuilder* builder) {
        return new Framebuffer(builder);
    }
    PipelineBase* Device::CreatePipeline(PipelineBuilder* builder) {
        return new Pipeline(builder);
    }
    PipelineLayoutBase* Device::CreatePipelineLayout(PipelineLayoutBuilder* builder) {
        return new PipelineLayout(builder);
    }
    QueueBase* Device::CreateQueue(QueueBuilder* builder) {
        return new Queue(builder);
    }
    RenderPassBase* Device::CreateRenderPass(RenderPassBuilder* builder) {
        return new RenderPass(builder);
    }
    SamplerBase* Device::CreateSampler(SamplerBuilder* builder) {
        return new Sampler(builder);
    }
    ShaderModuleBase* Device::CreateShaderModule(ShaderModuleBuilder* builder) {
        return new ShaderModule(builder);
    }
    TextureBase* Device::CreateTexture(TextureBuilder* builder) {
        return new Texture(builder);
    }
    TextureViewBase* Device::CreateTextureView(TextureViewBuilder* builder) {
        return new TextureView(builder);
    }

    void Device::TickImpl() {
        resourceUploader->Tick(finishedCommandSerial);
        mapReadTracker->Tick(finishedCommandSerial);

        // Code above might have added GPU work, submit it. This also makes sure
        // that even when no GPU work is happening, the serial number keeps incrementing.
        SubmitPendingCommandBuffer();
    }

    void Device::SetNextDrawable(id<CAMetalDrawable> drawable) {
        [currentDrawable release];
        currentDrawable = drawable;
        [currentDrawable retain];

        [currentTexture release];
        currentTexture = drawable.texture;
        [currentTexture retain];

        if (currentDepthTexture == nil ||
                currentTexture.width != currentDepthTexture.width ||
                currentTexture.height != currentDepthTexture.height) {
            if (currentDepthTexture != nil) {
                [currentDepthTexture release];
            }
            MTLTextureDescriptor* depthDescriptor = [MTLTextureDescriptor
                texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                width:currentTexture.width
                height:currentTexture.height
                mipmapped:NO];
            depthDescriptor.textureType = MTLTextureType2D;
            depthDescriptor.usage = MTLTextureUsageRenderTarget;
            depthDescriptor.storageMode = MTLStorageModePrivate;
            currentDepthTexture = [mtlDevice newTextureWithDescriptor:depthDescriptor];
        }

        MTLRenderPassDescriptor* passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        passDescriptor.colorAttachments[0].texture = currentTexture;
        passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
        passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
        passDescriptor.depthAttachment.texture = currentDepthTexture;
        passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
        passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
        passDescriptor.depthAttachment.clearDepth = 1.0;


        id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
        id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer
            renderCommandEncoderWithDescriptor:passDescriptor];
        [commandEncoder endEncoding];
        [commandBuffer commit];
    }

    void Device::Present() {
        id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
        [commandBuffer presentDrawable: currentDrawable];
        [commandBuffer commit];
    }

    id<MTLDevice> Device::GetMTLDevice() {
        return mtlDevice;
    }

    id<MTLTexture> Device::GetCurrentTexture() {
        return currentTexture;
    }

    id<MTLTexture> Device::GetCurrentDepthTexture() {
        return currentDepthTexture;
    }

    id<MTLCommandBuffer> Device::GetPendingCommandBuffer() {
        if (pendingCommands == nil) {
            pendingCommands = [commandQueue commandBuffer];
        }
        return pendingCommands;
    }

    void Device::SubmitPendingCommandBuffer() {
        if (pendingCommands == nil) {
            return;
        }

        // Ok, ObjC blocks are weird. My understanding is that local variables are captured by value
        // so this-> works as expected. However it is unclear how members are captured, (are they
        // captured using this-> or by value?) so we make a copy of the pendingCommandSerial on the stack.
        Serial pendingSerial = pendingCommandSerial;
        [pendingCommands addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer) {
            this->finishedCommandSerial = pendingSerial;
        }];

        [pendingCommands commit];
        pendingCommands = nil;
        pendingCommandSerial ++;
    }

    uint64_t Device::GetPendingCommandSerial() {
        // If this is called, then it means some piece of code somewhere will wait for this serial to
        // complete. Make sure the pending command buffer is created so that it is on the worst case
        // enqueued on the next Tick() and eventually increments the serial. Otherwise if no GPU work
        // happens we could be waiting for this serial forever.
        GetPendingCommandBuffer();
        return pendingCommandSerial;
    }

    MapReadRequestTracker* Device::GetMapReadTracker() const {
        return mapReadTracker;
    }

    ResourceUploader* Device::GetResourceUploader() const {
        return resourceUploader;
    }

    void Device::Reference() {
    }

    void Device::Release() {
    }

    // Bind Group

    BindGroup::BindGroup(BindGroupBuilder* builder)
        : BindGroupBase(builder) {
    }

    // Bind Group Layout

    BindGroupLayout::BindGroupLayout(BindGroupLayoutBuilder* builder)
        : BindGroupLayoutBase(builder) {
    }

    // Framebuffer

    Framebuffer::Framebuffer(FramebufferBuilder* builder)
        : FramebufferBase(builder) {
    }

    Framebuffer::~Framebuffer() {
    }

    // Queue

    Queue::Queue(QueueBuilder* builder)
        : QueueBase(builder) {
        Device* device = ToBackend(builder->GetDevice());
        commandQueue = [device->GetMTLDevice() newCommandQueue];
    }

    Queue::~Queue() {
        [commandQueue release];
        commandQueue = nil;
    }

    id<MTLCommandQueue> Queue::GetMTLCommandQueue() {
        return commandQueue;
    }

    void Queue::Submit(uint32_t numCommands, CommandBuffer* const * commands) {
        Device* device = ToBackend(GetDevice());
        id<MTLCommandBuffer> commandBuffer = device->GetPendingCommandBuffer();

        for (uint32_t i = 0; i < numCommands; ++i) {
            commands[i]->FillCommands(commandBuffer);
        }

        device->SubmitPendingCommandBuffer();
    }

    // RenderPass

    RenderPass::RenderPass(RenderPassBuilder* builder)
        : RenderPassBase(builder) {
    }

    RenderPass::~RenderPass() {
    }

}
}
