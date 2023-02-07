// Copyright 2017 The Dawn Authors
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

#include "dawn/samples/SampleUtils.h"

#include "dawn/utils/ScopedAutoreleasePool.h"
#include "dawn/utils/SystemUtils.h"
#include "dawn/utils/WGPUHelpers.h"

WGPUDevice device;
WGPUQueue queue;
WGPUSwapChain swapchain;
WGPURenderPipeline pipeline;

WGPUTextureFormat swapChainFormat;

void init() {
    device = CreateCppDawnDevice().Release();
    queue = wgpuDeviceGetQueue(device);
    swapchain = GetSwapChain().Release();
    swapChainFormat = static_cast<WGPUTextureFormat>(GetPreferredSwapChainTextureFormat());

    const char* vs = R"(
        @vertex fn main(
            @builtin(vertex_index) VertexIndex : u32
        ) -> @builtin(position) vec4f {
            var pos = array(
                vec2f( 0.0,  0.5),
                vec2f(-0.5, -0.5),
                vec2f( 0.5, -0.5)
            );
            return vec4f(pos[VertexIndex], 0.0, 1.0);
        })";
    WGPUShaderModule vsModule = utils::CreateShaderModule(device, vs).Release();

    const char* fs = R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(1.0, 0.0, 0.0, 1.0);
        })";
    WGPUShaderModule fsModule = utils::CreateShaderModule(device, fs).Release();

    {
        WGPURenderPipelineDescriptor descriptor = {};

        // Fragment state
        WGPUBlendState blend = {};
        blend.color.operation = WGPUBlendOperation_Add;
        blend.color.srcFactor = WGPUBlendFactor_One;
        blend.color.dstFactor = WGPUBlendFactor_One;
        blend.alpha.operation = WGPUBlendOperation_Add;
        blend.alpha.srcFactor = WGPUBlendFactor_One;
        blend.alpha.dstFactor = WGPUBlendFactor_One;

        WGPUColorTargetState colorTarget = {};
        colorTarget.format = swapChainFormat;
        colorTarget.blend = &blend;
        colorTarget.writeMask = WGPUColorWriteMask_All;

        WGPUFragmentState fragment = {};
        fragment.module = fsModule;
        fragment.entryPoint = "main";
        fragment.targetCount = 1;
        fragment.targets = &colorTarget;
        descriptor.fragment = &fragment;

        // Other state
        descriptor.layout = nullptr;
        descriptor.depthStencil = nullptr;

        descriptor.vertex.module = vsModule;
        descriptor.vertex.entryPoint = "main";
        descriptor.vertex.bufferCount = 0;
        descriptor.vertex.buffers = nullptr;

        descriptor.multisample.count = 1;
        descriptor.multisample.mask = 0xFFFFFFFF;
        descriptor.multisample.alphaToCoverageEnabled = false;

        descriptor.primitive.frontFace = WGPUFrontFace_CCW;
        descriptor.primitive.cullMode = WGPUCullMode_None;
        descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
        descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;

        pipeline = wgpuDeviceCreateRenderPipeline(device, &descriptor);
    }

    wgpuShaderModuleRelease(vsModule);
    wgpuShaderModuleRelease(fsModule);
}

void frame() {
    WGPUTextureView backbufferView = wgpuSwapChainGetCurrentTextureView(swapchain);
    WGPURenderPassDescriptor renderpassInfo = {};
    WGPURenderPassColorAttachment colorAttachment = {};
    {
        colorAttachment.view = backbufferView;
        colorAttachment.resolveTarget = nullptr;
        colorAttachment.clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
        colorAttachment.loadOp = WGPULoadOp_Clear;
        colorAttachment.storeOp = WGPUStoreOp_Store;
        renderpassInfo.colorAttachmentCount = 1;
        renderpassInfo.colorAttachments = &colorAttachment;
        renderpassInfo.depthStencilAttachment = nullptr;
    }
    WGPUCommandBuffer commands;
    {
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, nullptr);

        WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &renderpassInfo);
        wgpuRenderPassEncoderSetPipeline(pass, pipeline);
        wgpuRenderPassEncoderDraw(pass, 3, 1, 0, 0);
        wgpuRenderPassEncoderEnd(pass);
        wgpuRenderPassEncoderRelease(pass);

        commands = wgpuCommandEncoderFinish(encoder, nullptr);
        wgpuCommandEncoderRelease(encoder);
    }

    wgpuQueueSubmit(queue, 1, &commands);
    wgpuCommandBufferRelease(commands);
    wgpuSwapChainPresent(swapchain);
    wgpuTextureViewRelease(backbufferView);

    DoFlush();
}

int main(int argc, const char* argv[]) {
    if (!InitSample(argc, argv)) {
        return 1;
    }
    init();

    while (!ShouldQuit()) {
        utils::ScopedAutoreleasePool pool;
        frame();
        utils::USleep(16000);
    }
}
