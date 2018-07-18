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

#include "utils/NXTHelpers.h"
#include "utils/SystemUtils.h"

nxtDevice device;
nxtQueue queue;
nxtSwapChain swapchain;
nxtRenderPipeline pipeline;

nxtTextureFormat swapChainFormat;

void init() {
    device = CreateCppNXTDevice().Release();
    queue = nxtDeviceCreateQueue(device);

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
    nxtShaderModule vsModule = utils::CreateShaderModule(dawn::Device(device), dawn::ShaderStage::Vertex, vs).Release();

    const char* fs =
        "#version 450\n"
        "layout(location = 0) out vec4 fragColor;"
        "void main() {\n"
        "   fragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";
    nxtShaderModule fsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, fs).Release();

    {
        nxtRenderPipelineBuilder builder = nxtDeviceCreateRenderPipelineBuilder(device);
        nxtRenderPipelineBuilderSetColorAttachmentFormat(builder, 0, swapChainFormat);
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
    nxtRenderPassDescriptor renderpassInfo;
    {
        nxtRenderPassDescriptorBuilder builder = nxtDeviceCreateRenderPassDescriptorBuilder(device);
        nxtRenderPassDescriptorBuilderSetColorAttachment(builder, 0, backbufferView, NXT_LOAD_OP_CLEAR);
        renderpassInfo = nxtRenderPassDescriptorBuilderGetResult(builder);
        nxtRenderPassDescriptorBuilderRelease(builder);
    }
    nxtCommandBuffer commands;
    {
        nxtCommandBufferBuilder builder = nxtDeviceCreateCommandBufferBuilder(device);
        nxtCommandBufferBuilderBeginRenderPass(builder, renderpassInfo);
        nxtCommandBufferBuilderSetRenderPipeline(builder, pipeline);
        nxtCommandBufferBuilderDrawArrays(builder, 3, 1, 0, 0);
        nxtCommandBufferBuilderEndRenderPass(builder);
        commands = nxtCommandBufferBuilderGetResult(builder);
        nxtCommandBufferBuilderRelease(builder);
    }

    nxtQueueSubmit(queue, 1, &commands);
    nxtCommandBufferRelease(commands);
    nxtSwapChainPresent(swapchain, backbuffer);
    nxtRenderPassDescriptorRelease(renderpassInfo);
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
