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

#include "SampleUtils.h"

#include "utils/NXTHelpers.h"
#include "utils/SystemUtils.h"

nxtDevice device;
nxtQueue queue;
nxtSwapChain swapchain;
nxtRenderPipeline pipeline;
nxtRenderPass renderpass;

nxtTextureFormat swapChainFormat;

void init() {
    device = CreateCppNXTDevice().Release();

    {
        nxtQueueBuilder builder = nxtDeviceCreateQueueBuilder(device);
        queue = nxtQueueBuilderGetResult(builder);
        nxtQueueBuilderRelease(builder);
    }

    {
        nxtSwapChainBuilder builder = nxtDeviceCreateSwapChainBuilder(device);
        uint64_t swapchainImpl = GetSwapChainImplementation();
        nxtSwapChainBuilderSetImplementation(builder, swapchainImpl);
        swapchain = nxtSwapChainBuilderGetResult(builder);
        nxtSwapChainBuilderRelease(builder);
    }
    swapChainFormat = static_cast<nxtTextureFormat>(GetPreferredSwapChainTextureFormat());
    nxtSwapChainConfigure(swapchain, swapChainFormat, NXT_TEXTURE_USAGE_BIT_OUTPUT_ATTACHMENT, 640,
                          480);

    const char* vs =
        "#version 450\n"
        "const vec2 pos[3] = vec2[3](vec2(0.0f, 0.5f), vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f));\n"
        "void main() {\n"
        "   gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);\n"
        "}\n";
    nxtShaderModule vsModule = utils::CreateShaderModule(nxt::Device(device), nxt::ShaderStage::Vertex, vs).Release();

    const char* fs =
        "#version 450\n"
        "layout(location = 0) out vec4 fragColor;"
        "void main() {\n"
        "   fragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";
    nxtShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, fs).Release();

    {
        nxtRenderPassBuilder builder = nxtDeviceCreateRenderPassBuilder(device);
        nxtRenderPassBuilderSetAttachmentCount(builder, 1);
        nxtRenderPassBuilderAttachmentSetFormat(builder, 0, swapChainFormat);
        nxtRenderPassBuilderSetSubpassCount(builder, 1);
        nxtRenderPassBuilderSubpassSetColorAttachment(builder, 0, 0, 0);
        renderpass = nxtRenderPassBuilderGetResult(builder);
        nxtRenderPassBuilderRelease(builder);
    }
    {
        nxtRenderPipelineBuilder builder = nxtDeviceCreateRenderPipelineBuilder(device);
        nxtRenderPipelineBuilderSetSubpass(builder, renderpass, 0);
        nxtRenderPipelineBuilderSetStage(builder, NXT_SHADER_STAGE_VERTEX, vsModule, "main");
        nxtRenderPipelineBuilderSetStage(builder, NXT_SHADER_STAGE_FRAGMENT, fsModule, "main");
        pipeline = nxtRenderPipelineBuilderGetResult(builder);
        nxtRenderPipelineBuilderRelease(builder);
    }

    nxtShaderModuleRelease(vsModule);
    nxtShaderModuleRelease(fsModule);
}

void frame() {
    nxtTexture backbuffer = nxtSwapChainGetNextTexture(swapchain);
    nxtTextureView backbufferView;
    {
        nxtTextureViewBuilder builder = nxtTextureCreateTextureViewBuilder(backbuffer);
        backbufferView = nxtTextureViewBuilderGetResult(builder);
        nxtTextureViewBuilderRelease(builder);
    }
    nxtFramebuffer framebuffer;
    {
        nxtFramebufferBuilder builder = nxtDeviceCreateFramebufferBuilder(device);
        nxtFramebufferBuilderSetRenderPass(builder, renderpass);
        nxtFramebufferBuilderSetDimensions(builder, 640, 480);
        nxtFramebufferBuilderSetAttachment(builder, 0, backbufferView);
        framebuffer = nxtFramebufferBuilderGetResult(builder);
        nxtFramebufferBuilderRelease(builder);
    }
    nxtCommandBuffer commands;
    {
        nxtCommandBufferBuilder builder = nxtDeviceCreateCommandBufferBuilder(device);
        nxtCommandBufferBuilderBeginRenderPass(builder, renderpass, framebuffer);
        nxtCommandBufferBuilderBeginRenderSubpass(builder);
        nxtCommandBufferBuilderSetRenderPipeline(builder, pipeline);
        nxtCommandBufferBuilderDrawArrays(builder, 3, 1, 0, 0);
        nxtCommandBufferBuilderEndRenderSubpass(builder);
        nxtCommandBufferBuilderEndRenderPass(builder);
        commands = nxtCommandBufferBuilderGetResult(builder);
        nxtCommandBufferBuilderRelease(builder);
    }

    nxtQueueSubmit(queue, 1, &commands);
    nxtCommandBufferRelease(commands);
    nxtTextureTransitionUsage(backbuffer, NXT_TEXTURE_USAGE_BIT_PRESENT);
    nxtSwapChainPresent(swapchain, backbuffer);
    nxtFramebufferRelease(framebuffer);
    nxtTextureViewRelease(backbufferView);

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
