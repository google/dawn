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

#include <vector>

nxt::Device device;

nxt::Buffer vertexBuffer;
nxt::Buffer instanceBuffer;

nxt::Queue queue;
nxt::Pipeline pipeline;
nxt::RenderPass renderpass;
nxt::Framebuffer framebuffer;

void initBuffers() {
    static const float vertexData[12] = {
        0.0f, 0.1f, 0.0f, 1.0f,
        -0.1f, -0.1f, 0.0f, 1.0f,
        0.1f, -0.1f, 0.0f, 1.0f,
    };
    vertexBuffer = CreateFrozenBufferFromData(device, vertexData, sizeof(vertexData), nxt::BufferUsageBit::Vertex);

    static const float instanceData[8] = {
        -0.5f, -0.5f,
        -0.5f, 0.5f,
        0.5f, -0.5f,
        0.5f, 0.5f,
    };
    instanceBuffer = CreateFrozenBufferFromData(device, instanceData, sizeof(instanceData), nxt::BufferUsageBit::Vertex);
}

void init() {
    device = CreateCppNXTDevice();

    queue = device.CreateQueueBuilder().GetResult();

    initBuffers();

    nxt::ShaderModule vsModule = CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
        #version 450
        layout(location = 0) in vec4 pos;
        layout(location = 1) in vec2 instance;
        void main() {
            gl_Position = vec4(pos.xy + instance, pos.zw);
        })"
    );

    nxt::ShaderModule fsModule = CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
        #version 450
        out vec4 fragColor;
        void main() {
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        })"
    );

    auto inputState = device.CreateInputStateBuilder()
        .SetAttribute(0, 0, nxt::VertexFormat::FloatR32G32B32A32, 0)
        .SetInput(0, 4 * sizeof(float), nxt::InputStepMode::Vertex)
        .SetAttribute(1, 1, nxt::VertexFormat::FloatR32G32, 0)
        .SetInput(1, 2 * sizeof(float), nxt::InputStepMode::Instance)
        .GetResult();

    CreateDefaultRenderPass(device, &renderpass, &framebuffer);
    pipeline = device.CreatePipelineBuilder()
        .SetSubpass(renderpass, 0)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .SetInputState(inputState)
        .GetResult();
}

void frame() {
    static const uint32_t vertexBufferOffsets[1] = {0};
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(renderpass, framebuffer)
            .SetPipeline(pipeline)
            .SetVertexBuffers(0, 1, &vertexBuffer, vertexBufferOffsets)
            .SetVertexBuffers(1, 1, &instanceBuffer, vertexBufferOffsets)
            .DrawArrays(3, 4, 0, 0)
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);
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
