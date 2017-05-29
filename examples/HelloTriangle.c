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

#include "Utils.h"

nxtDevice device;
nxtQueue queue;
nxtPipeline pipeline;
nxtRenderPass renderpass;
nxtFramebuffer framebuffer;

void init() {
    device = CreateNXTDevice();

    {
        nxtQueueBuilder builder = nxtDeviceCreateQueueBuilder(device);
        queue = nxtQueueBuilderGetResult(builder);
        nxtQueueBuilderRelease(builder);
    }

    const char* vs =
        "#version 450\n"
        "const vec2 pos[3] = vec2[3](vec2(0.0f, 0.5f), vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f));\n"
        "void main() {\n"
        "   gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);\n"
        "}\n";
    nxtShaderModule vsModule = CreateShaderModule(device, NXT_SHADER_STAGE_VERTEX, vs);

    const char* fs =
        "#version 450\n"
        "out vec4 fragColor;"
        "void main() {\n"
        "   fragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";
    nxtShaderModule fsModule = CreateShaderModule(device, NXT_SHADER_STAGE_FRAGMENT, fs);

    {
        nxtRenderPassBuilder builder = nxtDeviceCreateRenderPassBuilder(device);
        nxtRenderPassBuilderSetAttachmentCount(builder, 1);
        nxtRenderPassBuilderAttachmentSetFormat(builder, 0, NXT_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM);
        nxtRenderPassBuilderSetSubpassCount(builder, 1);
        nxtRenderPassBuilderSubpassSetColorAttachment(builder, 0, 0, 0);
        renderpass = nxtRenderPassBuilderGetResult(builder);
        nxtRenderPassBuilderRelease(builder);
    }
    {
        nxtFramebufferBuilder builder = nxtDeviceCreateFramebufferBuilder(device);
        nxtFramebufferBuilderSetRenderPass(builder, renderpass);
        nxtFramebufferBuilderSetDimensions(builder, 640, 480);
        framebuffer = nxtFramebufferBuilderGetResult(builder);
        nxtFramebufferBuilderRelease(builder);
    }
    {
        nxtPipelineBuilder builder = nxtDeviceCreatePipelineBuilder(device);
        nxtPipelineBuilderSetSubpass(builder, renderpass, 0);
        nxtPipelineBuilderSetStage(builder, NXT_SHADER_STAGE_VERTEX, vsModule, "main");
        nxtPipelineBuilderSetStage(builder, NXT_SHADER_STAGE_FRAGMENT, fsModule, "main");
        pipeline = nxtPipelineBuilderGetResult(builder);
        nxtPipelineBuilderRelease(builder);
    }

    nxtShaderModuleRelease(vsModule);
    nxtShaderModuleRelease(fsModule);
}

void frame() {
    nxtCommandBuffer commands;
    {
        nxtCommandBufferBuilder builder = nxtDeviceCreateCommandBufferBuilder(device);
        nxtCommandBufferBuilderBeginRenderPass(builder, renderpass, framebuffer);
        nxtCommandBufferBuilderSetPipeline(builder, pipeline);
        nxtCommandBufferBuilderDrawArrays(builder, 3, 1, 0, 0);
        nxtCommandBufferBuilderEndRenderPass(builder);
        commands = nxtCommandBufferBuilderGetResult(builder);
        nxtCommandBufferBuilderRelease(builder);
    }

    nxtQueueSubmit(queue, 1, &commands);
    nxtCommandBufferRelease(commands);

    DoSwapBuffers();
}

int main(int argc, const char* argv[]) {
    if (!InitUtils(argc, argv)) {
        return 1;
    }
    init();

    while (!ShouldQuit()) {
        frame();
        USleep(16000);
    }

    // TODO release stuff
}
