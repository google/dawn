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

#include <vector>

nxt::Device device;

nxt::Buffer indexBuffer;
nxt::Buffer vertexBuffer;

nxt::Queue queue;
nxt::SwapChain swapchain;
nxt::TextureView depthStencilView;
nxt::RenderPipeline pipeline;
nxt::RenderPass renderpass;

void initBuffers() {
    static const uint32_t indexData[3] = {
        0, 1, 2,
    };
    indexBuffer = utils::CreateFrozenBufferFromData(device, indexData, sizeof(indexData), nxt::BufferUsageBit::Index);

    static const float vertexData[12] = {
        0.0f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 1.0f,
    };
    vertexBuffer = utils::CreateFrozenBufferFromData(device, vertexData, sizeof(vertexData), nxt::BufferUsageBit::Vertex);
}

void init() {
    device = CreateCppNXTDevice();

    queue = device.CreateQueueBuilder().GetResult();
    swapchain = GetSwapChain(device);
    swapchain.Configure(nxt::TextureFormat::R8G8B8A8Unorm, 640, 480);

    initBuffers();

    nxt::ShaderModule vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
        #version 450
        layout(location = 0) in vec4 pos;
        void main() {
            gl_Position = pos;
        })"
    );

    nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
        #version 450
        out vec4 fragColor;
        void main() {
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        })"
    );

    auto inputState = device.CreateInputStateBuilder()
        .SetAttribute(0, 0, nxt::VertexFormat::FloatR32G32B32A32, 0)
        .SetInput(0, 4 * sizeof(float), nxt::InputStepMode::Vertex)
        .GetResult();

    renderpass = CreateDefaultRenderPass(device);
    depthStencilView = CreateDefaultDepthStencilView(device);

    pipeline = device.CreateRenderPipelineBuilder()
        .SetSubpass(renderpass, 0)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .SetInputState(inputState)
        .GetResult();
}

void frame() {
    nxt::Texture backbuffer;
    nxt::Framebuffer framebuffer;
    GetNextFramebuffer(device, renderpass, swapchain, depthStencilView, &backbuffer, &framebuffer);

    static const uint32_t vertexBufferOffsets[1] = {0};
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(renderpass, framebuffer)
        .BeginRenderSubpass()
            .SetRenderPipeline(pipeline)
            .SetVertexBuffers(0, 1, &vertexBuffer, vertexBufferOffsets)
            .SetIndexBuffer(indexBuffer, 0, nxt::IndexFormat::Uint32)
            .DrawElements(3, 1, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);
    backbuffer.TransitionUsage(nxt::TextureUsageBit::Present);
    swapchain.Present(backbuffer);
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
