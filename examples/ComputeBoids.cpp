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

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/SystemUtils.h"
#include "utils/WGPUHelpers.h"

#include <array>
#include <cstring>
#include <random>

#include <glm/glm.hpp>

wgpu::Device device;
wgpu::Queue queue;
wgpu::SwapChain swapchain;
wgpu::TextureView depthStencilView;

wgpu::Buffer modelBuffer;
std::array<wgpu::Buffer, 2> particleBuffers;

wgpu::RenderPipeline renderPipeline;

wgpu::Buffer updateParams;
wgpu::ComputePipeline updatePipeline;
std::array<wgpu::BindGroup, 2> updateBGs;

size_t pingpong = 0;

static const uint32_t kNumParticles = 1000;

struct Particle {
    glm::vec2 pos;
    glm::vec2 vel;
};

struct SimParams {
    float deltaT;
    float rule1Distance;
    float rule2Distance;
    float rule3Distance;
    float rule1Scale;
    float rule2Scale;
    float rule3Scale;
    int particleCount;
};

void initBuffers() {
    glm::vec2 model[3] = {
        {-0.01, -0.02},
        {0.01, -0.02},
        {0.00, 0.02},
    };
    modelBuffer =
        utils::CreateBufferFromData(device, model, sizeof(model), wgpu::BufferUsage::Vertex);

    SimParams params = {0.04f, 0.1f, 0.025f, 0.025f, 0.02f, 0.05f, 0.005f, kNumParticles};
    updateParams =
        utils::CreateBufferFromData(device, &params, sizeof(params), wgpu::BufferUsage::Uniform);

    std::vector<Particle> initialParticles(kNumParticles);
    {
        std::mt19937 generator;
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        for (auto& p : initialParticles) {
            p.pos = glm::vec2(dist(generator), dist(generator));
            p.vel = glm::vec2(dist(generator), dist(generator)) * 0.1f;
        }
    }

    for (size_t i = 0; i < 2; i++) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = sizeof(Particle) * kNumParticles;
        descriptor.usage =
            wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Storage;
        particleBuffers[i] = device.CreateBuffer(&descriptor);

        queue.WriteBuffer(particleBuffers[i], 0,
                          reinterpret_cast<uint8_t*>(initialParticles.data()),
                          sizeof(Particle) * kNumParticles);
    }
}

void initRender() {
    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[location(0)]] var<in> a_particlePos : vec2<f32>;
        [[location(1)]] var<in> a_particleVel : vec2<f32>;
        [[location(2)]] var<in> a_pos : vec2<f32>;
        [[builtin(position)]] var<out> Position : vec4<f32>;

        [[stage(vertex)]]
        fn main() -> void {
            var angle : f32 = -atan2(a_particleVel.x, a_particleVel.y);
            var pos : vec2<f32> = vec2<f32>(
                (a_pos.x * cos(angle)) - (a_pos.y * sin(angle)),
                (a_pos.x * sin(angle)) + (a_pos.y * cos(angle)));
            Position = vec4<f32>(pos + a_particlePos, 0.0, 1.0);
            return;
        }
    )");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[location(0)]] var<out> FragColor : vec4<f32>;
        [[stage(fragment)]]
        fn main() -> void {
            FragColor = vec4<f32>(1.0, 1.0, 1.0, 1.0);
            return;
        }
    )");

    depthStencilView = CreateDefaultDepthStencilView(device);

    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.vertexStage.module = vsModule;
    descriptor.cFragmentStage.module = fsModule;

    descriptor.cVertexState.vertexBufferCount = 2;
    descriptor.cVertexState.cVertexBuffers[0].arrayStride = sizeof(Particle);
    descriptor.cVertexState.cVertexBuffers[0].stepMode = wgpu::InputStepMode::Instance;
    descriptor.cVertexState.cVertexBuffers[0].attributeCount = 2;
    descriptor.cVertexState.cAttributes[0].offset = offsetof(Particle, pos);
    descriptor.cVertexState.cAttributes[0].format = wgpu::VertexFormat::Float2;
    descriptor.cVertexState.cAttributes[1].shaderLocation = 1;
    descriptor.cVertexState.cAttributes[1].offset = offsetof(Particle, vel);
    descriptor.cVertexState.cAttributes[1].format = wgpu::VertexFormat::Float2;
    descriptor.cVertexState.cVertexBuffers[1].arrayStride = sizeof(glm::vec2);
    descriptor.cVertexState.cVertexBuffers[1].attributeCount = 1;
    descriptor.cVertexState.cVertexBuffers[1].attributes = &descriptor.cVertexState.cAttributes[2];
    descriptor.cVertexState.cAttributes[2].shaderLocation = 2;
    descriptor.cVertexState.cAttributes[2].format = wgpu::VertexFormat::Float2;
    descriptor.depthStencilState = &descriptor.cDepthStencilState;
    descriptor.cDepthStencilState.format = wgpu::TextureFormat::Depth24PlusStencil8;
    descriptor.cColorStates[0].format = GetPreferredSwapChainTextureFormat();

    renderPipeline = device.CreateRenderPipeline(&descriptor);
}

void initSim() {
    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct Particle {
            [[offset(0)]] pos : vec2<f32>;
            [[offset(8)]] vel : vec2<f32>;
        };
        [[block]] struct SimParams {
            [[offset(0)]] deltaT : f32;
            [[offset(4)]] rule1Distance : f32;
            [[offset(8)]] rule2Distance : f32;
            [[offset(12)]] rule3Distance : f32;
            [[offset(16)]] rule1Scale : f32;
            [[offset(20)]] rule2Scale : f32;
            [[offset(24)]] rule3Scale : f32;
            [[offset(28)]] particleCount : u32;
        };
        [[block]] struct Particles {
            [[offset(0)]] particles : [[stride(16)]] array<Particle>;
        };
        [[binding(0), group(0)]] var<uniform> params : SimParams;
        [[binding(1), group(0)]] var<storage_buffer> particlesA : [[access(read)]] Particles;
        [[binding(2), group(0)]] var<storage_buffer> particlesB : [[access(read_write)]] Particles;
        [[builtin(global_invocation_id)]] var<in> GlobalInvocationID : vec3<u32>;

        // https://github.com/austinEng/Project6-Vulkan-Flocking/blob/master/data/shaders/computeparticles/particle.comp
        [[stage(compute)]]
        fn main() -> void {
            var index : u32 = GlobalInvocationID.x;
            if (index >= params.particleCount) {
                return;
            }
            var vPos : vec2<f32> = particlesA.particles[index].pos;
            var vVel : vec2<f32> = particlesA.particles[index].vel;
            var cMass : vec2<f32> = vec2<f32>(0.0, 0.0);
            var cVel : vec2<f32> = vec2<f32>(0.0, 0.0);
            var colVel : vec2<f32> = vec2<f32>(0.0, 0.0);
            var cMassCount : u32 = 0u;
            var cVelCount : u32 = 0u;
            var pos : vec2<f32>;
            var vel : vec2<f32>;

            for (var i : u32 = 0u; i < params.particleCount; i = i + 1u) {
                if (i == index) {
                    continue;
                }

                pos = particlesA.particles[i].pos.xy;
                vel = particlesA.particles[i].vel.xy;
                if (distance(pos, vPos) < params.rule1Distance) {
                    cMass = cMass + pos;
                    cMassCount = cMassCount + 1u;
                }
                if (distance(pos, vPos) < params.rule2Distance) {
                    colVel = colVel - (pos - vPos);
                }
                if (distance(pos, vPos) < params.rule3Distance) {
                    cVel = cVel + vel;
                    cVelCount = cVelCount + 1u;
                }
            }

            if (cMassCount > 0u) {
                cMass = (cMass / vec2<f32>(f32(cMassCount), f32(cMassCount))) - vPos;
            }

            if (cVelCount > 0u) {
                cVel = cVel / vec2<f32>(f32(cVelCount), f32(cVelCount));
            }
            vVel = vVel + (cMass * params.rule1Scale) + (colVel * params.rule2Scale) +
                (cVel * params.rule3Scale);

            // clamp velocity for a more pleasing simulation
            vVel = normalize(vVel) * clamp(length(vVel), 0.0, 0.1);
            // kinematic update
            vPos = vPos + (vVel * params.deltaT);

            // Wrap around boundary
            if (vPos.x < -1.0) {
                vPos.x = 1.0;
            }
            if (vPos.x > 1.0) {
                vPos.x = -1.0;
            }
            if (vPos.y < -1.0) {
                vPos.y = 1.0;
            }
            if (vPos.y > 1.0) {
                vPos.y = -1.0;
            }

            // Write back
            particlesB.particles[index].pos = vPos;
            particlesB.particles[index].vel = vVel;
            return;
        }
    )");

    auto bgl = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::BindingType::UniformBuffer},
                    {1, wgpu::ShaderStage::Compute, wgpu::BindingType::StorageBuffer},
                    {2, wgpu::ShaderStage::Compute, wgpu::BindingType::StorageBuffer},
                });

    wgpu::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = pl;
    csDesc.computeStage.module = module;
    csDesc.computeStage.entryPoint = "main";
    updatePipeline = device.CreateComputePipeline(&csDesc);

    for (uint32_t i = 0; i < 2; ++i) {
        updateBGs[i] = utils::MakeBindGroup(
            device, bgl,
            {
                {0, updateParams, 0, sizeof(SimParams)},
                {1, particleBuffers[i], 0, kNumParticles * sizeof(Particle)},
                {2, particleBuffers[(i + 1) % 2], 0, kNumParticles * sizeof(Particle)},
            });
    }
}

wgpu::CommandBuffer createCommandBuffer(const wgpu::TextureView backbufferView, size_t i) {
    auto& bufferDst = particleBuffers[(i + 1) % 2];
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(updatePipeline);
        pass.SetBindGroup(0, updateBGs[i]);
        pass.Dispatch(kNumParticles);
        pass.EndPass();
    }

    {
        utils::ComboRenderPassDescriptor renderPass({backbufferView}, depthStencilView);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(renderPipeline);
        pass.SetVertexBuffer(0, bufferDst);
        pass.SetVertexBuffer(1, modelBuffer);
        pass.Draw(3, kNumParticles);
        pass.EndPass();
    }

    return encoder.Finish();
}

void init() {
    device = CreateCppDawnDevice();

    queue = device.GetDefaultQueue();
    swapchain = GetSwapChain(device);
    swapchain.Configure(GetPreferredSwapChainTextureFormat(), wgpu::TextureUsage::RenderAttachment,
                        640, 480);

    initBuffers();
    initRender();
    initSim();
}

void frame() {
    wgpu::TextureView backbufferView = swapchain.GetCurrentTextureView();

    wgpu::CommandBuffer commandBuffer = createCommandBuffer(backbufferView, pingpong);
    queue.Submit(1, &commandBuffer);
    swapchain.Present();
    DoFlush();

    pingpong = (pingpong + 1) % 2;
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
