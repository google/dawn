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

#include <vector>

dawn::Device device;

dawn::Buffer indexBuffer;
dawn::Buffer vertexBuffer;

dawn::Texture texture;
dawn::Sampler sampler;

dawn::Queue queue;
dawn::SwapChain swapchain;
dawn::TextureView depthStencilView;
dawn::RenderPipeline pipeline;
dawn::BindGroup bindGroup;

void initBuffers() {
    static const uint32_t indexData[3] = {
        0, 1, 2,
    };
    indexBuffer = utils::CreateBufferFromData(device, indexData, sizeof(indexData), dawn::BufferUsageBit::Index);

    static const float vertexData[12] = {
        0.0f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 1.0f,
    };
    vertexBuffer = utils::CreateBufferFromData(device, vertexData, sizeof(vertexData), dawn::BufferUsageBit::Vertex);
}

void initTextures() {
    dawn::TextureDescriptor descriptor;
    descriptor.dimension = dawn::TextureDimension::e2D;
    descriptor.size.width = 1024;
    descriptor.size.height = 1024;
    descriptor.size.depth = 1;
    descriptor.arrayLayer = 1;
    descriptor.format = dawn::TextureFormat::R8G8B8A8Unorm;
    descriptor.levelCount = 1;
    descriptor.usage = dawn::TextureUsageBit::TransferDst | dawn::TextureUsageBit::Sampled;
    texture = device.CreateTexture(&descriptor);

    dawn::SamplerDescriptor samplerDesc = utils::GetDefaultSamplerDescriptor();
    sampler = device.CreateSampler(&samplerDesc);

    // Initialize the texture with arbitrary data until we can load images
    std::vector<uint8_t> data(4 * 1024 * 1024, 0);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>(i % 253);
    }

    dawn::Buffer stagingBuffer = utils::CreateBufferFromData(device, data.data(), static_cast<uint32_t>(data.size()), dawn::BufferUsageBit::TransferSrc);
    dawn::BufferCopyView bufferCopyView = utils::CreateBufferCopyView(stagingBuffer, 0, 0, 0);
    dawn::TextureCopyView textureCopyView =
        utils::CreateTextureCopyView(texture, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color);
    dawn::Extent3D copySize = {1024, 1024, 1};
    dawn::CommandBuffer copy =
        device.CreateCommandBufferBuilder()
            .CopyBufferToTexture(&bufferCopyView, &textureCopyView, &copySize)
            .GetResult();

    queue.Submit(1, &copy);
}

void init() {
    device = CreateCppDawnDevice();

    queue = device.CreateQueue();
    swapchain = GetSwapChain(device);
    swapchain.Configure(GetPreferredSwapChainTextureFormat(),
                        dawn::TextureUsageBit::OutputAttachment, 640, 480);

    initBuffers();
    initTextures();

    dawn::ShaderModule vsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
        #version 450
        layout(location = 0) in vec4 pos;
        void main() {
            gl_Position = pos;
        })"
    );

    dawn::ShaderModule fsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
        #version 450
        layout(set = 0, binding = 0) uniform sampler mySampler;
        layout(set = 0, binding = 1) uniform texture2D myTexture;

        layout(location = 0) out vec4 fragColor;
        void main() {
            fragColor = texture(sampler2D(myTexture, mySampler), gl_FragCoord.xy / vec2(640.0, 480.0));
        })");

    auto inputState = device.CreateInputStateBuilder()
        .SetAttribute(0, 0, dawn::VertexFormat::FloatR32G32B32A32, 0)
        .SetInput(0, 4 * sizeof(float), dawn::InputStepMode::Vertex)
        .GetResult();

    auto bgl = utils::MakeBindGroupLayout(
        device, {
                    {0, dawn::ShaderStageBit::Fragment, dawn::BindingType::Sampler},
                    {1, dawn::ShaderStageBit::Fragment, dawn::BindingType::SampledTexture},
                });

    dawn::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);

    depthStencilView = CreateDefaultDepthStencilView(device);

    pipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, GetPreferredSwapChainTextureFormat())
        .SetDepthStencilAttachmentFormat(dawn::TextureFormat::D32FloatS8Uint)
        .SetLayout(pl)
        .SetStage(dawn::ShaderStage::Vertex, vsModule, "main")
        .SetStage(dawn::ShaderStage::Fragment, fsModule, "main")
        .SetIndexFormat(dawn::IndexFormat::Uint32)
        .SetInputState(inputState)
        .GetResult();

    dawn::TextureView view = texture.CreateDefaultTextureView();

    bindGroup = device.CreateBindGroupBuilder()
        .SetLayout(bgl)
        .SetSamplers(0, 1, &sampler)
        .SetTextureViews(1, 1, &view)
        .GetResult();
}

struct {uint32_t a; float b;} s;
void frame() {
    s.a = (s.a + 1) % 256;
    s.b += 0.02f;
    if (s.b >= 1.0f) {s.b = 0.0f;}

    dawn::Texture backbuffer;
    dawn::RenderPassDescriptor renderPass;
    GetNextRenderPassDescriptor(device, swapchain, depthStencilView, &backbuffer, &renderPass);

    static const uint32_t vertexBufferOffsets[1] = {0};
    dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass);
        pass.SetRenderPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.SetVertexBuffers(0, 1, &vertexBuffer, vertexBufferOffsets);
        pass.SetIndexBuffer(indexBuffer, 0);
        pass.DrawElements(3, 1, 0, 0);
        pass.EndPass();
    }

    dawn::CommandBuffer commands = builder.GetResult();
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
