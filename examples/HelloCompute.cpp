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

#include <string.h>

nxt::Device device;
nxt::Queue queue;
nxt::SwapChain swapchain;
nxt::TextureView depthStencilView;
nxt::Buffer buffer;
nxt::RenderPipeline renderPipeline;
nxt::BindGroup renderBindGroup;
nxt::RenderPass renderpass;
nxt::ComputePipeline computePipeline;
nxt::BindGroup computeBindGroup;

void init() {
    device = CreateCppNXTDevice();

    queue = device.CreateQueueBuilder().GetResult();
    swapchain = GetSwapChain(device);
    swapchain.Configure(GetPreferredSwapChainTextureFormat(),
                        nxt::TextureUsageBit::OutputAttachment, 640, 480);

    struct {uint32_t a; float b;} s;
    memset(&s, 0, sizeof(s));
    buffer = device.CreateBufferBuilder()
        .SetAllowedUsage(nxt::BufferUsageBit::TransferDst | nxt::BufferUsageBit::Uniform | nxt::BufferUsageBit::Storage)
        .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
        .SetSize(sizeof(s))
        .GetResult();
    buffer.SetSubData(0, sizeof(s), reinterpret_cast<uint8_t*>(&s));

    nxt::BufferView view = buffer.CreateBufferViewBuilder()
        .SetExtent(0, sizeof(s))
        .GetResult();

    {
        nxt::ShaderModule module = utils::CreateShaderModule(device, nxt::ShaderStage::Compute, R"(
            #version 450
            layout(set = 0, binding = 0) buffer myBlock {
                int a;
                float b;
            } myStorage;
            void main() {
                myStorage.a = (myStorage.a + 1) % 256;
                myStorage.b = mod((myStorage.b + 0.02), 1.0);
            })"
        );

        nxt::BindGroupLayout bgl = device.CreateBindGroupLayoutBuilder()
            .SetBindingsType(nxt::ShaderStageBit::Compute, nxt::BindingType::StorageBuffer, 0, 1)
            .GetResult();

        nxt::PipelineLayout pl = device.CreatePipelineLayoutBuilder()
            .SetBindGroupLayout(0, bgl)
            .GetResult();

        computePipeline = device.CreateComputePipelineBuilder()
            .SetLayout(pl)
            .SetStage(nxt::ShaderStage::Compute, module, "main")
            .GetResult();

        computeBindGroup = device.CreateBindGroupBuilder()
            .SetLayout(bgl)
            .SetUsage(nxt::BindGroupUsage::Frozen)
            .SetBufferViews(0, 1, &view)
            .GetResult();
    }

    {
        nxt::ShaderModule vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
            #version 450
            const vec2 pos[3] = vec2[3](vec2(0.0f, 0.5f), vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f));
            void main() {
                gl_Position = vec4(pos[gl_VertexIndex], 0.5, 1.0);
            })"
        );

        nxt::ShaderModule fsModule =
            utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
            #version 450
            layout(set = 0, binding = 0) uniform myBlock {
                int a;
                float b;
            } myUbo;
            layout(location = 0) out vec4 fragColor;
            void main() {
                fragColor = vec4(1.0, myUbo.a / 255.0, myUbo.b, 1.0);
            })");

        nxt::BindGroupLayout bgl = device.CreateBindGroupLayoutBuilder()
            .SetBindingsType(nxt::ShaderStageBit::Fragment, nxt::BindingType::UniformBuffer, 0, 1)
            .GetResult();

        nxt::PipelineLayout pl = device.CreatePipelineLayoutBuilder()
            .SetBindGroupLayout(0, bgl)
            .GetResult();

        renderpass = CreateDefaultRenderPass(device);
        depthStencilView = CreateDefaultDepthStencilView(device);

        renderPipeline = device.CreateRenderPipelineBuilder()
            .SetSubpass(renderpass, 0)
            .SetLayout(pl)
            .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
            .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
            .GetResult();

        renderBindGroup = device.CreateBindGroupBuilder()
            .SetLayout(bgl)
            .SetUsage(nxt::BindGroupUsage::Frozen)
            .SetBufferViews(0, 1, &view)
            .GetResult();
    }
}

void frame() {
    nxt::Texture backbuffer;
    nxt::Framebuffer framebuffer;
    GetNextFramebuffer(device, renderpass, swapchain, depthStencilView, &backbuffer, &framebuffer);

    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .TransitionBufferUsage(buffer, nxt::BufferUsageBit::Storage)
        .BeginComputePass()
            .SetComputePipeline(computePipeline)
            .SetBindGroup(0, computeBindGroup)
            .Dispatch(1, 1, 1)
        .EndComputePass()

        .TransitionBufferUsage(buffer, nxt::BufferUsageBit::Uniform)
        .BeginRenderPass(renderpass, framebuffer)
        .BeginRenderSubpass()
            .SetRenderPipeline(renderPipeline)
            .SetBindGroup(0, renderBindGroup)
            .DrawArrays(3, 1, 0, 0)
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
