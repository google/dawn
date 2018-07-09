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

nxt::Texture texture;
nxt::Sampler sampler;

nxt::Queue queue;
nxt::SwapChain swapchain;
nxt::TextureView depthStencilView;
nxt::RenderPipeline pipeline;
nxt::BindGroup bindGroup;

void initBuffers() {
    static const uint32_t indexData[3] = {
        0, 1, 2,
    };
    indexBuffer = utils::CreateBufferFromData(device, indexData, sizeof(indexData), nxt::BufferUsageBit::Index);

    static const float vertexData[12] = {
        0.0f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 1.0f,
    };
    vertexBuffer = utils::CreateBufferFromData(device, vertexData, sizeof(vertexData), nxt::BufferUsageBit::Vertex);
}

void initTextures() {
    texture = device.CreateTextureBuilder()
        .SetDimension(nxt::TextureDimension::e2D)
        .SetExtent(1024, 1024, 1)
        .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
        .SetMipLevels(1)
        .SetAllowedUsage(nxt::TextureUsageBit::TransferDst | nxt::TextureUsageBit::Sampled)
        .GetResult();

    nxt::SamplerDescriptor samplerDesc = utils::GetDefaultSamplerDescriptor();
    sampler = device.CreateSampler(&samplerDesc);

    // Initialize the texture with arbitrary data until we can load images
    std::vector<uint8_t> data(4 * 1024 * 1024, 0);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>(i % 253);
    }


    nxt::Buffer stagingBuffer = utils::CreateBufferFromData(device, data.data(), static_cast<uint32_t>(data.size()), nxt::BufferUsageBit::TransferSrc);
    nxt::CommandBuffer copy = device.CreateCommandBufferBuilder()
        .CopyBufferToTexture(stagingBuffer, 0, 0, texture, 0, 0, 0, 1024, 1024, 1, 0)
        .GetResult();

    queue.Submit(1, &copy);
}

void init() {
    device = CreateCppNXTDevice();

    queue = device.CreateQueue();
    swapchain = GetSwapChain(device);
    swapchain.Configure(GetPreferredSwapChainTextureFormat(),
                        nxt::TextureUsageBit::OutputAttachment, 640, 480);

    initBuffers();
    initTextures();

    nxt::ShaderModule vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
        #version 450
        layout(location = 0) in vec4 pos;
        void main() {
            gl_Position = pos;
        })"
    );

    nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
        #version 450
        layout(set = 0, binding = 0) uniform sampler mySampler;
        layout(set = 0, binding = 1) uniform texture2D myTexture;

        layout(location = 0) out vec4 fragColor;
        void main() {
            fragColor = texture(sampler2D(myTexture, mySampler), gl_FragCoord.xy / vec2(640.0, 480.0));
        })");

    auto inputState = device.CreateInputStateBuilder()
        .SetAttribute(0, 0, nxt::VertexFormat::FloatR32G32B32A32, 0)
        .SetInput(0, 4 * sizeof(float), nxt::InputStepMode::Vertex)
        .GetResult();

    auto bgl = utils::MakeBindGroupLayout(
        device, {
                    {0, nxt::ShaderStageBit::Fragment, nxt::BindingType::Sampler},
                    {1, nxt::ShaderStageBit::Fragment, nxt::BindingType::SampledTexture},
                });

    nxt::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);

    depthStencilView = CreateDefaultDepthStencilView(device);

    pipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, GetPreferredSwapChainTextureFormat())
        .SetDepthStencilAttachmentFormat(nxt::TextureFormat::D32FloatS8Uint)
        .SetLayout(pl)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .SetIndexFormat(nxt::IndexFormat::Uint32)
        .SetInputState(inputState)
        .GetResult();

    nxt::TextureView view = texture.CreateTextureViewBuilder().GetResult();

    bindGroup = device.CreateBindGroupBuilder()
        .SetLayout(bgl)
        .SetUsage(nxt::BindGroupUsage::Frozen)
        .SetSamplers(0, 1, &sampler)
        .SetTextureViews(1, 1, &view)
        .GetResult();
}

struct {uint32_t a; float b;} s;
void frame() {
    s.a = (s.a + 1) % 256;
    s.b += 0.02f;
    if (s.b >= 1.0f) {s.b = 0.0f;}

    nxt::Texture backbuffer;
    nxt::RenderPassDescriptor renderPass;
    GetNextRenderPassDescriptor(device, swapchain, depthStencilView, &backbuffer, &renderPass);

    static const uint32_t vertexBufferOffsets[1] = {0};
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(renderPass)
            .SetRenderPipeline(pipeline)
            .SetBindGroup(0, bindGroup)
            .SetVertexBuffers(0, 1, &vertexBuffer, vertexBufferOffsets)
            .SetIndexBuffer(indexBuffer, 0)
            .DrawElements(3, 1, 0, 0)
        .EndRenderPass()
        .GetResult();

    queue.Submit(1, &commands);
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
