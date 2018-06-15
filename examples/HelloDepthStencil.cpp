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
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

nxt::Device device;

nxt::Buffer indexBuffer;
nxt::Buffer vertexBuffer;
nxt::Buffer planeBuffer;
nxt::Buffer cameraBuffer;
nxt::Buffer transformBuffer[2];

nxt::BindGroup cameraBindGroup;
nxt::BindGroup bindGroup[2];
nxt::BindGroup cubeTransformBindGroup[2];

nxt::Queue queue;
nxt::SwapChain swapchain;
nxt::TextureView depthStencilView;
nxt::RenderPipeline pipeline;
nxt::RenderPipeline planePipeline;
nxt::RenderPipeline reflectionPipeline;

void initBuffers() {
    static const uint32_t indexData[6*6] = {
        0, 1, 2,
        0, 2, 3,

        4, 5, 6,
        4, 6, 7,

        8, 9, 10,
        8, 10, 11,

        12, 13, 14,
        12, 14, 15,

        16, 17, 18,
        16, 18, 19,

        20, 21, 22,
        20, 22, 23
    };
    indexBuffer = utils::CreateFrozenBufferFromData(device, indexData, sizeof(indexData), nxt::BufferUsageBit::Index);

    static const float vertexData[6 * 4 * 6] = {
        -1.0, -1.0,  1.0,    1.0, 0.0, 0.0,
        1.0, -1.0,  1.0,    1.0, 0.0, 0.0,
        1.0,  1.0,  1.0,    1.0, 0.0, 0.0,
        -1.0,  1.0,  1.0,    1.0, 0.0, 0.0,

        -1.0, -1.0, -1.0,    1.0, 1.0, 0.0,
        -1.0,  1.0, -1.0,    1.0, 1.0, 0.0,
        1.0,  1.0, -1.0,    1.0, 1.0, 0.0,
        1.0, -1.0, -1.0,    1.0, 1.0, 0.0,

        -1.0,  1.0, -1.0,    1.0, 0.0, 1.0,
        -1.0,  1.0,  1.0,    1.0, 0.0, 1.0,
        1.0,  1.0,  1.0,    1.0, 0.0, 1.0,
        1.0,  1.0, -1.0,    1.0, 0.0, 1.0,

        -1.0, -1.0, -1.0,    0.0, 1.0, 0.0,
        1.0, -1.0, -1.0,    0.0, 1.0, 0.0,
        1.0, -1.0,  1.0,    0.0, 1.0, 0.0,
        -1.0, -1.0,  1.0,    0.0, 1.0, 0.0,

        1.0, -1.0, -1.0,    0.0, 1.0, 1.0,
        1.0,  1.0, -1.0,    0.0, 1.0, 1.0,
        1.0,  1.0,  1.0,    0.0, 1.0, 1.0,
        1.0, -1.0,  1.0,    0.0, 1.0, 1.0,

        -1.0, -1.0, -1.0,    1.0, 1.0, 1.0,
        -1.0, -1.0,  1.0,    1.0, 1.0, 1.0,
        -1.0,  1.0,  1.0,    1.0, 1.0, 1.0,
        -1.0,  1.0, -1.0,    1.0, 1.0, 1.0
    };
    vertexBuffer = utils::CreateFrozenBufferFromData(device, vertexData, sizeof(vertexData), nxt::BufferUsageBit::Vertex);

    static const float planeData[6 * 4] = {
        -2.0, -1.0, -2.0,    0.5, 0.5, 0.5,
        2.0, -1.0, -2.0,    0.5, 0.5, 0.5,
        2.0, -1.0,  2.0,    0.5, 0.5, 0.5,
        -2.0, -1.0,  2.0,    0.5, 0.5, 0.5,
    };
    planeBuffer = utils::CreateFrozenBufferFromData(device, planeData, sizeof(planeData), nxt::BufferUsageBit::Vertex);
}

struct CameraData {
    glm::mat4 view;
    glm::mat4 proj;
} cameraData;

void init() {
    device = CreateCppNXTDevice();

    queue = device.CreateQueue();
    swapchain = GetSwapChain(device);
    swapchain.Configure(GetPreferredSwapChainTextureFormat(),
                        nxt::TextureUsageBit::OutputAttachment, 640, 480);

    initBuffers();

    nxt::ShaderModule vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
        #version 450
        layout(set = 0, binding = 0) uniform cameraData {
            mat4 view;
            mat4 proj;
        } camera;
        layout(set = 0, binding = 1) uniform modelData {
            mat4 modelMatrix;
        };
        layout(location = 0) in vec3 pos;
        layout(location = 1) in vec3 col;
        layout(location = 2) out vec3 f_col;
        void main() {
            f_col = col;
            gl_Position = camera.proj * camera.view * modelMatrix * vec4(pos, 1.0);
        })"
    );

    nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
        #version 450
        layout(location = 2) in vec3 f_col;
        layout(location = 0) out vec4 fragColor;
        void main() {
            fragColor = vec4(f_col, 1.0);
        })");

    nxt::ShaderModule fsReflectionModule =
        utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
        #version 450
        layout(location = 2) in vec3 f_col;
        layout(location = 0) out vec4 fragColor;
        void main() {
            fragColor = vec4(mix(f_col, vec3(0.5, 0.5, 0.5), 0.5), 1.0);
        })");

    auto inputState = device.CreateInputStateBuilder()
        .SetAttribute(0, 0, nxt::VertexFormat::FloatR32G32B32, 0)
        .SetAttribute(1, 0, nxt::VertexFormat::FloatR32G32B32, 3 * sizeof(float))
        .SetInput(0, 6 * sizeof(float), nxt::InputStepMode::Vertex)
        .GetResult();

    nxt::BindGroupLayout bgl = device.CreateBindGroupLayoutBuilder()
        .SetBindingsType(nxt::ShaderStageBit::Vertex, nxt::BindingType::UniformBuffer, 0, 2)
        .GetResult();

    nxt::PipelineLayout pl = device.CreatePipelineLayoutBuilder()
        .SetBindGroupLayout(0, bgl)
        .GetResult();

    cameraBuffer = device.CreateBufferBuilder()
        .SetAllowedUsage(nxt::BufferUsageBit::TransferDst | nxt::BufferUsageBit::Uniform)
        .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
        .SetSize(sizeof(CameraData))
        .GetResult();

    glm::mat4 transform(1.0);
    transformBuffer[0] = utils::CreateFrozenBufferFromData(device, &transform, sizeof(glm::mat4), nxt::BufferUsageBit::Uniform);

    transform = glm::translate(transform, glm::vec3(0.f, -2.f, 0.f));
    transformBuffer[1] = utils::CreateFrozenBufferFromData(device, &transform, sizeof(glm::mat4), nxt::BufferUsageBit::Uniform);

    nxt::BufferView cameraBufferView = cameraBuffer.CreateBufferViewBuilder()
        .SetExtent(0, sizeof(CameraData))
        .GetResult();

    nxt::BufferView transformBufferView[2] = {
        transformBuffer[0].CreateBufferViewBuilder()
            .SetExtent(0, sizeof(glm::mat4))
            .GetResult(),
        transformBuffer[1].CreateBufferViewBuilder()
            .SetExtent(0, sizeof(glm::mat4))
            .GetResult(),
    };

    bindGroup[0] = device.CreateBindGroupBuilder()
        .SetLayout(bgl)
        .SetUsage(nxt::BindGroupUsage::Frozen)
        .SetBufferViews(0, 1, &cameraBufferView)
        .SetBufferViews(1, 1, &transformBufferView[0])
        .GetResult();

    bindGroup[1] = device.CreateBindGroupBuilder()
        .SetLayout(bgl)
        .SetUsage(nxt::BindGroupUsage::Frozen)
        .SetBufferViews(0, 1, &cameraBufferView)
        .SetBufferViews(1, 1, &transformBufferView[1])
        .GetResult();

    depthStencilView = CreateDefaultDepthStencilView(device);

    auto depthStencilState = device.CreateDepthStencilStateBuilder()
        .SetDepthCompareFunction(nxt::CompareFunction::Less)
        .SetDepthWriteEnabled(true)
        .GetResult();

    pipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, GetPreferredSwapChainTextureFormat())
        .SetDepthStencilAttachmentFormat(nxt::TextureFormat::D32FloatS8Uint)
        .SetLayout(pl)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .SetIndexFormat(nxt::IndexFormat::Uint32)
        .SetInputState(inputState)
        .SetDepthStencilState(depthStencilState)
        .GetResult();

    auto planeStencilState = device.CreateDepthStencilStateBuilder()
        .SetDepthCompareFunction(nxt::CompareFunction::Less)
        .SetDepthWriteEnabled(false)
        .SetStencilFunction(nxt::Face::Both, nxt::CompareFunction::Always, nxt::StencilOperation::Keep, nxt::StencilOperation::Keep, nxt::StencilOperation::Replace)
        .GetResult();

    planePipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, GetPreferredSwapChainTextureFormat())
        .SetDepthStencilAttachmentFormat(nxt::TextureFormat::D32FloatS8Uint)
        .SetLayout(pl)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .SetInputState(inputState)
        .SetDepthStencilState(planeStencilState)
        .GetResult();

    auto reflectionStencilState = device.CreateDepthStencilStateBuilder()
        .SetDepthCompareFunction(nxt::CompareFunction::Less)
        .SetDepthWriteEnabled(true)
        .SetStencilFunction(nxt::Face::Both, nxt::CompareFunction::Equal, nxt::StencilOperation::Keep, nxt::StencilOperation::Keep, nxt::StencilOperation::Replace)
        .GetResult();

    reflectionPipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, GetPreferredSwapChainTextureFormat())
        .SetDepthStencilAttachmentFormat(nxt::TextureFormat::D32FloatS8Uint)
        .SetLayout(pl)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsReflectionModule, "main")
        .SetInputState(inputState)
        .SetDepthStencilState(reflectionStencilState)
        .GetResult();

    cameraData.proj = glm::perspective(glm::radians(45.0f), 1.f, 1.0f, 100.0f);
}

struct {uint32_t a; float b;} s;
void frame() {
    s.a = (s.a + 1) % 256;
    s.b += 0.01f;
    if (s.b >= 1.0f) {s.b = 0.0f;}
    static const uint32_t vertexBufferOffsets[1] = {0};

    cameraData.view = glm::lookAt(
        glm::vec3(8.f * std::sin(glm::radians(s.b * 360.f)), 2.f, 8.f * std::cos(glm::radians(s.b * 360.f))),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    cameraBuffer.TransitionUsage(nxt::BufferUsageBit::TransferDst);
    cameraBuffer.SetSubData(0, sizeof(CameraData), reinterpret_cast<uint8_t*>(&cameraData));

    nxt::Texture backbuffer;
    nxt::RenderPassDescriptor renderPass;
    GetNextRenderPassDescriptor(device, swapchain, depthStencilView, &backbuffer, &renderPass);

    nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
        .BeginRenderPass(renderPass)
            .SetRenderPipeline(pipeline)
            .TransitionBufferUsage(cameraBuffer, nxt::BufferUsageBit::Uniform)
            .SetBindGroup(0, bindGroup[0])
            .SetVertexBuffers(0, 1, &vertexBuffer, vertexBufferOffsets)
            .SetIndexBuffer(indexBuffer, 0)
            .DrawElements(36, 1, 0, 0)

            .SetStencilReference(0x1)
            .SetRenderPipeline(planePipeline)
            .SetBindGroup(0, bindGroup[0])
            .SetVertexBuffers(0, 1, &planeBuffer, vertexBufferOffsets)
            .DrawElements(6, 1, 0, 0)

            .SetRenderPipeline(reflectionPipeline)
            .SetVertexBuffers(0, 1, &vertexBuffer, vertexBufferOffsets)
            .SetBindGroup(0, bindGroup[1])
            .DrawElements(36, 1, 0, 0)
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
