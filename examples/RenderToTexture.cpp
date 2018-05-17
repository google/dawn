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

nxt::Buffer vertexBuffer;
nxt::Buffer vertexBufferQuad;

nxt::Texture renderTarget;
nxt::TextureView renderTargetView;
nxt::Sampler samplerPost;

nxt::Queue queue;
nxt::SwapChain swapchain;
nxt::RenderPipeline pipeline;
nxt::RenderPipeline pipelinePost;
nxt::BindGroup bindGroup;

void initBuffers() {
    static const float vertexData[12] = {
        0.0f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 1.0f,
    };
    vertexBuffer = utils::CreateFrozenBufferFromData(device, vertexData, sizeof(vertexData), nxt::BufferUsageBit::Vertex);

    static const float vertexDataQuad[24] = {
        -1.0f, -1.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,
    };
    vertexBufferQuad = utils::CreateFrozenBufferFromData(device, vertexDataQuad, sizeof(vertexDataQuad), nxt::BufferUsageBit::Vertex);
}

void initTextures() {
    renderTarget = device.CreateTextureBuilder()
        .SetDimension(nxt::TextureDimension::e2D)
        .SetExtent(640, 480, 1)
        .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
        .SetMipLevels(1)
        .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment | nxt::TextureUsageBit::Sampled)
        .SetInitialUsage(nxt::TextureUsageBit::OutputAttachment)
        .GetResult();
    renderTargetView = renderTarget.CreateTextureViewBuilder().GetResult();

    {
        nxt::SamplerDescriptor desc;
        desc.minFilter = nxt::FilterMode::Linear;
        desc.magFilter = nxt::FilterMode::Linear;
        desc.mipmapFilter = nxt::FilterMode::Linear;
        desc.addressModeU = nxt::AddressMode::Repeat;
        desc.addressModeV = nxt::AddressMode::Repeat;
        desc.addressModeW = nxt::AddressMode::Repeat;
        samplerPost = device.CreateSampler(&desc);
    }
}

void initPipeline() {
    nxt::ShaderModule vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
        #version 450
        layout(location = 0) in vec4 pos;
        void main() {
            gl_Position = pos;
        })"
    );

    nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
        #version 450
        layout(location = 0) out vec4 fragColor;
        void main() {
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        })"
    );

    auto inputState = device.CreateInputStateBuilder()
        .SetAttribute(0, 0, nxt::VertexFormat::FloatR32G32B32A32, 0)
        .SetInput(0, 4 * sizeof(float), nxt::InputStepMode::Vertex)
        .GetResult();

    pipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .SetInputState(inputState)
        .GetResult();
}

void initPipelinePost() {
    nxt::ShaderModule vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
        #version 450
        layout(location = 0) in vec4 pos;
        void main() {
            gl_Position = pos;
        })"
    );

    nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
        #version 450
        layout(set = 0, binding = 0) uniform sampler samp;
        layout(set = 0, binding = 1) uniform texture2D tex;
        layout(location = 0) out vec4 fragColor;
        void main() {
            fragColor = texture(sampler2D(tex, samp), gl_FragCoord.xy / vec2(640.0, 480.0)) + vec4(0.0, 0.0, 0.5, 0.0);
        })"
    );

    auto inputState = device.CreateInputStateBuilder()
        .SetAttribute(0, 0, nxt::VertexFormat::FloatR32G32B32A32, 0)
        .SetInput(0, 4 * sizeof(float), nxt::InputStepMode::Vertex)
        .GetResult();

    nxt::BindGroupLayout bgl = device.CreateBindGroupLayoutBuilder()
        .SetBindingsType(nxt::ShaderStageBit::Fragment, nxt::BindingType::Sampler, 0, 1)
        .SetBindingsType(nxt::ShaderStageBit::Fragment, nxt::BindingType::SampledTexture, 1, 1)
        .GetResult();

    nxt::PipelineLayout pl = device.CreatePipelineLayoutBuilder()
        .SetBindGroupLayout(0, bgl)
        .GetResult();

    pipelinePost = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .SetLayout(pl)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .SetInputState(inputState)
        .GetResult();

    bindGroup = device.CreateBindGroupBuilder()
        .SetLayout(bgl)
        .SetUsage(nxt::BindGroupUsage::Frozen)
        .SetSamplers(0, 1, &samplerPost)
        .SetTextureViews(1, 1, &renderTargetView)
        .GetResult();
}

void init() {
    device = CreateCppNXTDevice();

    queue = device.CreateQueueBuilder().GetResult();
    swapchain = GetSwapChain(device);
    swapchain.Configure(GetPreferredSwapChainTextureFormat(),
                        nxt::TextureUsageBit::OutputAttachment, 640, 480);

    initBuffers();
    initTextures();
    initPipeline();
    initPipelinePost();
}

void frame() {
    nxt::Texture backbuffer = swapchain.GetNextTexture();
    auto backbufferView = backbuffer.CreateTextureViewBuilder().GetResult();

    nxt::RenderPassDescriptor renderPass1 = device.CreateRenderPassDescriptorBuilder()
        .SetColorAttachment(0, renderTargetView, nxt::LoadOp::Clear)
        .GetResult();

    nxt::RenderPassDescriptor renderPass2 = device.CreateRenderPassDescriptorBuilder()
        .SetColorAttachment(0, backbufferView, nxt::LoadOp::Clear)
        .GetResult();

    static const uint32_t vertexBufferOffsets[1] = {0};
    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(renderPass1)
            // renderTarget implicitly locked to to Attachment usage (if not already frozen)
            .SetRenderPipeline(pipeline)
            .SetVertexBuffers(0, 1, &vertexBuffer, vertexBufferOffsets)
            .DrawArrays(3, 1, 0, 0)
        .EndRenderPass()
        // renderTarget usage unlocked, but left in Attachment usage
        .TransitionTextureUsage(renderTarget, nxt::TextureUsageBit::Sampled)
        .BeginRenderPass(renderPass2)
            .SetRenderPipeline(pipelinePost)
            .SetVertexBuffers(0, 1, &vertexBufferQuad, vertexBufferOffsets)
            .SetBindGroup(0, bindGroup)
            .DrawArrays(6, 1, 0, 0)
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
