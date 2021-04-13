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

#include "SampleUtils.h"

#include "utils/SystemUtils.h"
#include "utils/WGPUHelpers.h"

WGPUDevice device;
WGPUQueue queue;
WGPUSwapChain swapchain;
WGPURenderPipeline pipeline;

WGPUTextureFormat swapChainFormat;

void init() {
    device = CreateCppDawnDevice().Release();
    queue = wgpuDeviceGetQueue(device);

    {
        WGPUSwapChainDescriptor descriptor = {};
        descriptor.implementation = GetSwapChainImplementation();
        swapchain = wgpuDeviceCreateSwapChain(device, nullptr, &descriptor);
    }
    swapChainFormat = static_cast<WGPUTextureFormat>(GetPreferredSwapChainTextureFormat());
    wgpuSwapChainConfigure(swapchain, swapChainFormat, WGPUTextureUsage_RenderAttachment, 640, 480);

    const char* vs =
        "let pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(\n"
        "    vec2<f32>( 0.0,  0.5),\n"
        "    vec2<f32>(-0.5, -0.5),\n"
        "    vec2<f32>( 0.5, -0.5)\n"
        ");\n"
        "[[stage(vertex)]] fn main(\n"
        "    [[builtin(vertex_index)]] VertexIndex : u32\n"
        ") -> [[builtin(position)]] vec4<f32> {\n"
        "    return vec4<f32>(pos[VertexIndex], 0.0, 1.0);\n"
        "}\n";
    WGPUShaderModule vsModule = utils::CreateShaderModule(device, vs).Release();

    const char* fs =
        "[[stage(fragment)]] fn main() -> [[location(0)]] vec4<f32> {\n"
        "    return vec4<f32>(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";
    WGPUShaderModule fsModule = utils::CreateShaderModule(device, fs).Release();

    {
        WGPURenderPipelineDescriptor2 descriptor = {};

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

        pipeline = wgpuDeviceCreateRenderPipeline2(device, &descriptor);
    }

    wgpuShaderModuleRelease(vsModule);
    wgpuShaderModuleRelease(fsModule);
}

void frame() {
    WGPUTextureView backbufferView = wgpuSwapChainGetCurrentTextureView(swapchain);
    WGPURenderPassDescriptor renderpassInfo = {};
    WGPURenderPassColorAttachmentDescriptor colorAttachment = {};
    {
        colorAttachment.attachment = backbufferView;
        colorAttachment.resolveTarget = nullptr;
        colorAttachment.clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
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
        wgpuRenderPassEncoderEndPass(pass);
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
        frame();
        utils::USleep(16000);
    }

    // TODO release stuff
}
