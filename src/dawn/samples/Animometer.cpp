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

#include <cstdio>
#include <cstdlib>
#include <vector>

#include "dawn/samples/SampleUtils.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/ScopedAutoreleasePool.h"
#include "dawn/utils/SystemUtils.h"
#include "dawn/utils/WGPUHelpers.h"

wgpu::Device device;
wgpu::Queue queue;
wgpu::SwapChain swapchain;
wgpu::RenderPipeline pipeline;
wgpu::BindGroup bindGroup;
wgpu::Buffer ubo;

float RandomFloat(float min, float max) {
    // NOLINTNEXTLINE(runtime/threadsafe_fn)
    float zeroOne = rand() / static_cast<float>(RAND_MAX);
    return zeroOne * (max - min) + min;
}

constexpr size_t kNumTriangles = 10000;

// Aligned as minUniformBufferOffsetAlignment
struct alignas(256) ShaderData {
    float scale;
    float time;
    float offsetX;
    float offsetY;
    float scalar;
    float scalarOffset;
};

static std::vector<ShaderData> shaderData;

void init() {
    device = CreateCppDawnDevice();

    queue = device.GetQueue();
    swapchain = GetSwapChain();

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        struct Constants {
            scale : f32,
            time : f32,
            offsetX : f32,
            offsetY : f32,
            scalar : f32,
            scalarOffset : f32,
        };
        @group(0) @binding(0) var<uniform> c : Constants;

        struct VertexOut {
            @location(0) v_color : vec4f,
            @builtin(position) Position : vec4f,
        };

        @vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> VertexOut {
            var positions : array<vec4f, 3> = array(
                vec4f( 0.0,  0.1, 0.0, 1.0),
                vec4f(-0.1, -0.1, 0.0, 1.0),
                vec4f( 0.1, -0.1, 0.0, 1.0)
            );

            var colors : array<vec4f, 3> = array(
                vec4f(1.0, 0.0, 0.0, 1.0),
                vec4f(0.0, 1.0, 0.0, 1.0),
                vec4f(0.0, 0.0, 1.0, 1.0)
            );

            var position : vec4f = positions[VertexIndex];
            var color : vec4f = colors[VertexIndex];

            // TODO(dawn:572): Revisit once modf has been reworked in WGSL.
            var fade : f32 = c.scalarOffset + c.time * c.scalar / 10.0;
            fade = fade - floor(fade);
            if (fade < 0.5) {
                fade = fade * 2.0;
            } else {
                fade = (1.0 - fade) * 2.0;
            }

            var xpos : f32 = position.x * c.scale;
            var ypos : f32 = position.y * c.scale;
            let angle : f32 = 3.14159 * 2.0 * fade;
            let xrot : f32 = xpos * cos(angle) - ypos * sin(angle);
            let yrot : f32 = xpos * sin(angle) + ypos * cos(angle);
            xpos = xrot + c.offsetX;
            ypos = yrot + c.offsetY;

            var output : VertexOut;
            output.v_color = vec4f(fade, 1.0 - fade, 0.0, 1.0) + color;
            output.Position = vec4f(xpos, ypos, 0.0, 1.0);
            return output;
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main(@location(0) v_color : vec4f) -> @location(0) vec4f {
            return v_color;
        })");

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform, true}});

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cTargets[0].format = GetPreferredSwapChainTextureFormat();

    pipeline = device.CreateRenderPipeline(&descriptor);

    shaderData.resize(kNumTriangles);
    for (auto& data : shaderData) {
        data.scale = RandomFloat(0.2f, 0.4f);
        data.time = 0.0;
        data.offsetX = RandomFloat(-0.9f, 0.9f);
        data.offsetY = RandomFloat(-0.9f, 0.9f);
        data.scalar = RandomFloat(0.5f, 2.0f);
        data.scalarOffset = RandomFloat(0.0f, 10.0f);
    }

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = kNumTriangles * sizeof(ShaderData);
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
    ubo = device.CreateBuffer(&bufferDesc);

    bindGroup = utils::MakeBindGroup(device, bgl, {{0, ubo, 0, sizeof(ShaderData)}});
}

void frame() {
    wgpu::TextureView backbufferView = swapchain.GetCurrentTextureView();

    static int f = 0;
    f++;
    for (auto& data : shaderData) {
        data.time = f / 60.0f;
    }
    queue.WriteBuffer(ubo, 0, shaderData.data(), kNumTriangles * sizeof(ShaderData));

    utils::ComboRenderPassDescriptor renderPass({backbufferView});
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);

        for (size_t i = 0; i < kNumTriangles; i++) {
            uint32_t offset = i * sizeof(ShaderData);
            pass.SetBindGroup(0, bindGroup, 1, &offset);
            pass.Draw(3);
        }

        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
    swapchain.Present();
    DoFlush();
    fprintf(stderr, "frame %i\n", f);
}

int main(int argc, const char* argv[]) {
    if (!InitSample(argc, argv)) {
        return 1;
    }
    init();

    while (!ShouldQuit()) {
        utils::ScopedAutoreleasePool pool;
        frame();
        utils::USleep(16000);
    }
}
