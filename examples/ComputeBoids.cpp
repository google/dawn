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

#include <array>
#include <cstring>
#include <random>

#include <glm/glm.hpp>

nxt::Device device;
nxt::Queue queue;
nxt::SwapChain swapchain;
nxt::TextureView depthStencilView;

nxt::Buffer modelBuffer;
std::array<nxt::Buffer, 2> particleBuffers;

nxt::RenderPipeline renderPipeline;
nxt::RenderPass renderpass;

nxt::Buffer updateParams;
nxt::ComputePipeline updatePipeline;
std::array<nxt::BindGroup, 2> updateBGs;

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
    modelBuffer = utils::CreateFrozenBufferFromData(device, model, sizeof(model), nxt::BufferUsageBit::Vertex);

    SimParams params = { 0.04f, 0.1f, 0.025f, 0.025f, 0.02f, 0.05f, 0.005f, kNumParticles };
    updateParams = utils::CreateFrozenBufferFromData(device, &params, sizeof(params), nxt::BufferUsageBit::Uniform);

    std::vector<Particle> initialParticles(kNumParticles);
    {
        std::mt19937 generator;
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        for (auto& p : initialParticles)
        {
            p.pos = glm::vec2(dist(generator), dist(generator));
            p.vel = glm::vec2(dist(generator), dist(generator)) * 0.1f;
        }
    }

    for (size_t i = 0; i < 2; i++) {
        particleBuffers[i] = device.CreateBufferBuilder()
            .SetAllowedUsage(nxt::BufferUsageBit::TransferDst | nxt::BufferUsageBit::Vertex | nxt::BufferUsageBit::Storage)
            .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
            .SetSize(sizeof(Particle) * kNumParticles)
            .GetResult();

        particleBuffers[i].SetSubData(0,
            sizeof(Particle) * kNumParticles / sizeof(uint32_t),
            reinterpret_cast<uint32_t*>(initialParticles.data()));
    }
}

void initRender() {
    nxt::ShaderModule vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
        #version 450
        layout(location = 0) in vec2 a_particlePos;
        layout(location = 1) in vec2 a_particleVel;
        layout(location = 2) in vec2 a_pos;
        void main() {
            float angle = -atan(a_particleVel.x, a_particleVel.y);
            vec2 pos = vec2(a_pos.x * cos(angle) - a_pos.y * sin(angle),
                            a_pos.x * sin(angle) + a_pos.y * cos(angle));
            gl_Position = vec4(pos + a_particlePos, 0, 1);
        }
    )");

    nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
        #version 450
        out vec4 fragColor;
        void main() {
            fragColor = vec4(1.0);
        }
    )");

    nxt::InputState inputState = device.CreateInputStateBuilder()
        .SetAttribute(0, 0, nxt::VertexFormat::FloatR32G32, offsetof(Particle, pos))
        .SetAttribute(1, 0, nxt::VertexFormat::FloatR32G32, offsetof(Particle, vel))
        .SetInput(0, sizeof(Particle), nxt::InputStepMode::Instance)
        .SetAttribute(2, 1, nxt::VertexFormat::FloatR32G32, 0)
        .SetInput(1, sizeof(glm::vec2), nxt::InputStepMode::Vertex)
        .GetResult();

    renderpass = CreateDefaultRenderPass(device);
    depthStencilView = CreateDefaultDepthStencilView(device);

    renderPipeline = device.CreateRenderPipelineBuilder()
        .SetSubpass(renderpass, 0)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .SetInputState(inputState)
        .GetResult();
}

void initSim() {
    nxt::ShaderModule module = utils::CreateShaderModule(device, nxt::ShaderStage::Compute, R"(
        #version 450

        struct Particle {
            vec2 pos;
            vec2 vel;
        };

        layout(std140, set = 0, binding = 0) uniform SimParams {
            float deltaT;
            float rule1Distance;
            float rule2Distance;
            float rule3Distance;
            float rule1Scale;
            float rule2Scale;
            float rule3Scale;
            int particleCount;
        } params;

        layout(std140, set = 0, binding = 1) buffer ParticlesA {
            Particle particlesA[1000];
        };

        layout(std140, set = 0, binding = 2) buffer ParticlesB {
            Particle particlesB[1000];
        };

        void main() {
            // https://github.com/austinEng/Project6-Vulkan-Flocking/blob/master/data/shaders/computeparticles/particle.comp

            uint index = gl_GlobalInvocationID.x;
            if (index >= params.particleCount) { return; }

            vec2 vPos = particlesA[index].pos;
            vec2 vVel = particlesA[index].vel;

            vec2 cMass = vec2(0.0, 0.0);
            vec2 cVel = vec2(0.0, 0.0);
            vec2 colVel = vec2(0.0, 0.0);
            int cMassCount = 0;
            int cVelCount = 0;

            vec2 pos;
            vec2 vel;
            for (int i = 0; i < params.particleCount; ++i) {
                if (i == index) { continue; }
                pos = particlesA[i].pos.xy;
                vel = particlesA[i].vel.xy;

                if (distance(pos, vPos) < params.rule1Distance) {
                    cMass += pos;
                    cMassCount++;
                }
                if (distance(pos, vPos) < params.rule2Distance) {
                    colVel -= (pos - vPos);
                }
                if (distance(pos, vPos) < params.rule3Distance) {
                    cVel += vel;
                    cVelCount++;
                }
            }
            if (cMassCount > 0) {
                cMass = cMass / cMassCount - vPos;
            }
            if (cVelCount > 0) {
                cVel = cVel / cVelCount;
            }

            vVel += cMass * params.rule1Scale + colVel * params.rule2Scale + cVel * params.rule3Scale;

            // clamp velocity for a more pleasing simulation.
            vVel = normalize(vVel) * clamp(length(vVel), 0.0, 0.1);

            // kinematic update
            vPos += vVel * params.deltaT;

            // Wrap around boundary
            if (vPos.x < -1.0) vPos.x = 1.0;
            if (vPos.x > 1.0) vPos.x = -1.0;
            if (vPos.y < -1.0) vPos.y = 1.0;
            if (vPos.y > 1.0) vPos.y = -1.0;

            particlesB[index].pos = vPos;

            // Write back
            particlesB[index].vel = vVel;
        }
    )");

    nxt::BindGroupLayout bgl = device.CreateBindGroupLayoutBuilder()
        .SetBindingsType(nxt::ShaderStageBit::Compute, nxt::BindingType::UniformBuffer, 0, 1)
        .SetBindingsType(nxt::ShaderStageBit::Compute, nxt::BindingType::StorageBuffer, 1, 2)
        .GetResult();

    nxt::PipelineLayout pl = device.CreatePipelineLayoutBuilder()
        .SetBindGroupLayout(0, bgl)
        .GetResult();

    updatePipeline = device.CreateComputePipelineBuilder()
        .SetLayout(pl)
        .SetStage(nxt::ShaderStage::Compute, module, "main")
        .GetResult();

    nxt::BufferView updateParamsView = updateParams.CreateBufferViewBuilder()
        .SetExtent(0, sizeof(SimParams))
        .GetResult();

    std::array<nxt::BufferView, 2> views;
    for (uint32_t i = 0; i < 2; ++i) {
        views[i] = particleBuffers[i].CreateBufferViewBuilder()
            .SetExtent(0, kNumParticles * sizeof(Particle))
            .GetResult();
    }

    for (uint32_t i = 0; i < 2; ++i) {
        updateBGs[i] = device.CreateBindGroupBuilder()
            .SetLayout(bgl)
            .SetUsage(nxt::BindGroupUsage::Frozen)
            .SetBufferViews(0, 1, &updateParamsView)
            .SetBufferViews(1, 1, &views[i])
            .SetBufferViews(2, 1, &views[(i + 1) % 2])
            .GetResult();
    }
}

nxt::CommandBuffer createCommandBuffer(const nxt::Framebuffer& framebuffer, size_t i) {
    static const uint32_t zeroOffsets[1] = {0};
    auto& bufferSrc = particleBuffers[i];
    auto& bufferDst = particleBuffers[(i + 1) % 2];
    return device.CreateCommandBufferBuilder()
        .BeginComputePass()
            .SetComputePipeline(updatePipeline)
            .TransitionBufferUsage(bufferSrc, nxt::BufferUsageBit::Storage)
            .TransitionBufferUsage(bufferDst, nxt::BufferUsageBit::Storage)
            .SetBindGroup(0, updateBGs[i])
            .Dispatch(kNumParticles, 1, 1)
        .EndComputePass()

        .BeginRenderPass(renderpass, framebuffer)
        .BeginRenderSubpass()
            .SetRenderPipeline(renderPipeline)
            .TransitionBufferUsage(bufferDst, nxt::BufferUsageBit::Vertex)
            .SetVertexBuffers(0, 1, &bufferDst, zeroOffsets)
            .SetVertexBuffers(1, 1, &modelBuffer, zeroOffsets)
            .DrawArrays(3, kNumParticles, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()

        .GetResult();
}

void init() {
    device = CreateCppNXTDevice();

    queue = device.CreateQueueBuilder().GetResult();
    swapchain = GetSwapChain(device);
    swapchain.Configure(nxt::TextureFormat::R8G8B8A8Unorm, 640, 480);

    initBuffers();
    initRender();
    initSim();
}

void frame() {
    nxt::Texture backbuffer;
    nxt::Framebuffer framebuffer;
    GetNextFramebuffer(device, renderpass, swapchain, depthStencilView, &backbuffer, &framebuffer);

    nxt::CommandBuffer commandBuffer = createCommandBuffer(framebuffer, pingpong);
    queue.Submit(1, &commandBuffer);
    backbuffer.TransitionUsage(nxt::TextureUsageBit::Present);
    swapchain.Present(backbuffer);
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
