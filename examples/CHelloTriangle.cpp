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

#include "utils/DawnHelpers.h"
#include "utils/SystemUtils.h"

dawnDevice device;
dawnQueue queue;
dawnSwapChain swapchain;
dawnRenderPipeline pipeline;

dawnTextureFormat swapChainFormat;

void init() {
    device = CreateCppDawnDevice().Release();
    queue = dawnDeviceCreateQueue(device);

    {
        dawnSwapChainBuilder builder = dawnDeviceCreateSwapChainBuilder(device);
        uint64_t swapchainImpl = GetSwapChainImplementation();
        dawnSwapChainBuilderSetImplementation(builder, swapchainImpl);
        swapchain = dawnSwapChainBuilderGetResult(builder);
        dawnSwapChainBuilderRelease(builder);
    }
    swapChainFormat = static_cast<dawnTextureFormat>(GetPreferredSwapChainTextureFormat());
    dawnSwapChainConfigure(swapchain, swapChainFormat, DAWN_TEXTURE_USAGE_BIT_OUTPUT_ATTACHMENT, 640,
                          480);

    const char* vs =
        "#version 450\n"
        "const vec2 pos[3] = vec2[3](vec2(0.0f, 0.5f), vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f));\n"
        "void main() {\n"
        "   gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);\n"
        "}\n";
    dawnShaderModule vsModule = utils::CreateShaderModule(dawn::Device(device), dawn::ShaderStage::Vertex, vs).Release();

    const char* fs =
        "#version 450\n"
        "layout(location = 0) out vec4 fragColor;"
        "void main() {\n"
        "   fragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";
    dawnShaderModule fsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, fs).Release();

    {
        dawnRenderPipelineDescriptor descriptor;
        descriptor.nextInChain = nullptr;

        dawnPipelineStageDescriptor vertexStage;
        vertexStage.nextInChain = nullptr;
        vertexStage.module = vsModule;
        vertexStage.entryPoint = "main";
        descriptor.vertexStage = &vertexStage;

        dawnPipelineStageDescriptor fragmentStage;
        fragmentStage.nextInChain = nullptr;
        fragmentStage.module = fsModule;
        fragmentStage.entryPoint = "main";
        descriptor.fragmentStage = &fragmentStage;

        dawnAttachmentsStateDescriptor attachmentsState;
        attachmentsState.nextInChain = nullptr;
        attachmentsState.numColorAttachments = 1;
        dawnAttachmentDescriptor colorAttachment = {nullptr, swapChainFormat};
        attachmentsState.colorAttachments = &colorAttachment;
        attachmentsState.hasDepthStencilAttachment = false;
        // Even with hasDepthStencilAttachment = false, depthStencilAttachment must point to valid
        // data because we don't have optional substructures yet.
        attachmentsState.depthStencilAttachment = &colorAttachment;
        descriptor.attachmentsState = &attachmentsState;

        descriptor.sampleCount = 1;

        descriptor.numBlendStates = 1;
        dawnBlendStateBuilder blendStateBuilder = dawnDeviceCreateBlendStateBuilder(device);
        dawnBlendState blendState = dawnBlendStateBuilderGetResult(blendStateBuilder);
        descriptor.blendStates =  &blendState;
        dawnBlendStateBuilderRelease(blendStateBuilder);

        dawnPipelineLayoutDescriptor pl;
        pl.nextInChain = nullptr;
        pl.numBindGroupLayouts = 0;
        pl.bindGroupLayouts = nullptr;
        descriptor.layout = dawnDeviceCreatePipelineLayout(device, &pl);

        dawnInputStateBuilder inputStateBuilder = dawnDeviceCreateInputStateBuilder(device);
        descriptor.inputState = dawnInputStateBuilderGetResult(inputStateBuilder);
        dawnInputStateBuilderRelease(inputStateBuilder);

        descriptor.indexFormat = DAWN_INDEX_FORMAT_UINT32;
        descriptor.primitiveTopology = DAWN_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        dawnDepthStencilStateBuilder depthStencilBuilder = dawnDeviceCreateDepthStencilStateBuilder(device);
        descriptor.depthStencilState = dawnDepthStencilStateBuilderGetResult(depthStencilBuilder);
        dawnDepthStencilStateBuilderRelease(depthStencilBuilder);

        pipeline = dawnDeviceCreateRenderPipeline(device, &descriptor);

        dawnBlendStateRelease(descriptor.blendStates[0]);
        dawnDepthStencilStateRelease(descriptor.depthStencilState);
        dawnInputStateRelease(descriptor.inputState);
    }

    dawnShaderModuleRelease(vsModule);
    dawnShaderModuleRelease(fsModule);
}

void frame() {
    dawnTexture backbuffer = dawnSwapChainGetNextTexture(swapchain);
    dawnTextureView backbufferView;
    {
        backbufferView = dawnTextureCreateDefaultTextureView(backbuffer);
    }
    dawnRenderPassDescriptor renderpassInfo;
    {
        dawnRenderPassDescriptorBuilder builder = dawnDeviceCreateRenderPassDescriptorBuilder(device);
        dawnRenderPassColorAttachmentDescriptor colorAttachment;
        colorAttachment.attachment = backbufferView;
        colorAttachment.resolveTarget = nullptr;
        colorAttachment.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        colorAttachment.loadOp = DAWN_LOAD_OP_CLEAR;
        colorAttachment.storeOp = DAWN_STORE_OP_STORE;
        dawnRenderPassDescriptorBuilderSetColorAttachments(builder, 1, &colorAttachment);
        renderpassInfo = dawnRenderPassDescriptorBuilderGetResult(builder);
        dawnRenderPassDescriptorBuilderRelease(builder);
    }
    dawnCommandBuffer commands;
    {
        dawnCommandBufferBuilder builder = dawnDeviceCreateCommandBufferBuilder(device);

        dawnRenderPassEncoder pass = dawnCommandBufferBuilderBeginRenderPass(builder, renderpassInfo);
        dawnRenderPassEncoderSetPipeline(pass, pipeline);
        dawnRenderPassEncoderDraw(pass, 3, 1, 0, 0);
        dawnRenderPassEncoderEndPass(pass);
        dawnRenderPassEncoderRelease(pass);

        commands = dawnCommandBufferBuilderGetResult(builder);
        dawnCommandBufferBuilderRelease(builder);
    }

    dawnQueueSubmit(queue, 1, &commands);
    dawnCommandBufferRelease(commands);
    dawnSwapChainPresent(swapchain, backbuffer);
    dawnRenderPassDescriptorRelease(renderpassInfo);
    dawnTextureViewRelease(backbufferView);

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
