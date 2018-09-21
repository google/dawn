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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

dawn::Device device;

dawn::Buffer indexBuffer;
dawn::Buffer vertexBuffer;
dawn::Buffer planeBuffer;
dawn::Buffer cameraBuffer;
dawn::Buffer transformBuffer[2];

dawn::BindGroup cameraBindGroup;
dawn::BindGroup bindGroup[2];
dawn::BindGroup cubeTransformBindGroup[2];

dawn::Queue queue;
dawn::SwapChain swapchain;
dawn::TextureView depthStencilView;
dawn::RenderPipeline pipeline;
dawn::RenderPipeline planePipeline;
dawn::RenderPipeline reflectionPipeline;

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
    indexBuffer = utils::CreateBufferFromData(device, indexData, sizeof(indexData), dawn::BufferUsageBit::Index);

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
    vertexBuffer = utils::CreateBufferFromData(device, vertexData, sizeof(vertexData), dawn::BufferUsageBit::Vertex);

    static const float planeData[6 * 4] = {
        -2.0, -1.0, -2.0,    0.5, 0.5, 0.5,
        2.0, -1.0, -2.0,    0.5, 0.5, 0.5,
        2.0, -1.0,  2.0,    0.5, 0.5, 0.5,
        -2.0, -1.0,  2.0,    0.5, 0.5, 0.5,
    };
    planeBuffer = utils::CreateBufferFromData(device, planeData, sizeof(planeData), dawn::BufferUsageBit::Vertex);
}

struct CameraData {
    glm::mat4 view;
    glm::mat4 proj;
} cameraData;

void init() {
    device = CreateCppDawnDevice();

    queue = device.CreateQueue();
    swapchain = GetSwapChain(device);
    swapchain.Configure(GetPreferredSwapChainTextureFormat(),
                        dawn::TextureUsageBit::OutputAttachment, 640, 480);

    initBuffers();

    dawn::ShaderModule vsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
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

    dawn::ShaderModule fsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
        #version 450
        layout(location = 2) in vec3 f_col;
        layout(location = 0) out vec4 fragColor;
        void main() {
            fragColor = vec4(f_col, 1.0);
        })");

    dawn::ShaderModule fsReflectionModule =
        utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
        #version 450
        layout(location = 2) in vec3 f_col;
        layout(location = 0) out vec4 fragColor;
        void main() {
            fragColor = vec4(mix(f_col, vec3(0.5, 0.5, 0.5), 0.5), 1.0);
        })");

    auto inputState = device.CreateInputStateBuilder()
        .SetAttribute(0, 0, dawn::VertexFormat::FloatR32G32B32, 0)
        .SetAttribute(1, 0, dawn::VertexFormat::FloatR32G32B32, 3 * sizeof(float))
        .SetInput(0, 6 * sizeof(float), dawn::InputStepMode::Vertex)
        .GetResult();

    auto bgl = utils::MakeBindGroupLayout(
        device, {
                    {0, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
                    {1, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
                });

    dawn::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);

    dawn::BufferDescriptor cameraBufDesc;
    cameraBufDesc.size = sizeof(CameraData);
    cameraBufDesc.usage = dawn::BufferUsageBit::TransferDst | dawn::BufferUsageBit::Uniform;
    cameraBuffer = device.CreateBuffer(&cameraBufDesc);

    glm::mat4 transform(1.0);
    transformBuffer[0] = utils::CreateBufferFromData(device, &transform, sizeof(glm::mat4), dawn::BufferUsageBit::Uniform);

    transform = glm::translate(transform, glm::vec3(0.f, -2.f, 0.f));
    transformBuffer[1] = utils::CreateBufferFromData(device, &transform, sizeof(glm::mat4), dawn::BufferUsageBit::Uniform);

    dawn::BufferView cameraBufferView = cameraBuffer.CreateBufferViewBuilder()
        .SetExtent(0, sizeof(CameraData))
        .GetResult();

    dawn::BufferView transformBufferView[2] = {
        transformBuffer[0].CreateBufferViewBuilder()
            .SetExtent(0, sizeof(glm::mat4))
            .GetResult(),
        transformBuffer[1].CreateBufferViewBuilder()
            .SetExtent(0, sizeof(glm::mat4))
            .GetResult(),
    };

    bindGroup[0] = device.CreateBindGroupBuilder()
        .SetLayout(bgl)
        .SetBufferViews(0, 1, &cameraBufferView)
        .SetBufferViews(1, 1, &transformBufferView[0])
        .GetResult();

    bindGroup[1] = device.CreateBindGroupBuilder()
        .SetLayout(bgl)
        .SetBufferViews(0, 1, &cameraBufferView)
        .SetBufferViews(1, 1, &transformBufferView[1])
        .GetResult();

    depthStencilView = CreateDefaultDepthStencilView(device);

    auto depthStencilState = device.CreateDepthStencilStateBuilder()
        .SetDepthCompareFunction(dawn::CompareFunction::Less)
        .SetDepthWriteEnabled(true)
        .GetResult();

    pipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, GetPreferredSwapChainTextureFormat())
        .SetDepthStencilAttachmentFormat(dawn::TextureFormat::D32FloatS8Uint)
        .SetLayout(pl)
        .SetStage(dawn::ShaderStage::Vertex, vsModule, "main")
        .SetStage(dawn::ShaderStage::Fragment, fsModule, "main")
        .SetIndexFormat(dawn::IndexFormat::Uint32)
        .SetInputState(inputState)
        .SetDepthStencilState(depthStencilState)
        .GetResult();

    auto planeStencilState = device.CreateDepthStencilStateBuilder()
        .SetDepthCompareFunction(dawn::CompareFunction::Less)
        .SetDepthWriteEnabled(false)
        .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Always, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Replace)
        .GetResult();

    planePipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, GetPreferredSwapChainTextureFormat())
        .SetDepthStencilAttachmentFormat(dawn::TextureFormat::D32FloatS8Uint)
        .SetLayout(pl)
        .SetStage(dawn::ShaderStage::Vertex, vsModule, "main")
        .SetStage(dawn::ShaderStage::Fragment, fsModule, "main")
        .SetInputState(inputState)
        .SetDepthStencilState(planeStencilState)
        .GetResult();

    auto reflectionStencilState = device.CreateDepthStencilStateBuilder()
        .SetDepthCompareFunction(dawn::CompareFunction::Less)
        .SetDepthWriteEnabled(true)
        .SetStencilFunction(dawn::Face::Both, dawn::CompareFunction::Equal, dawn::StencilOperation::Keep, dawn::StencilOperation::Keep, dawn::StencilOperation::Replace)
        .GetResult();

    reflectionPipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, GetPreferredSwapChainTextureFormat())
        .SetDepthStencilAttachmentFormat(dawn::TextureFormat::D32FloatS8Uint)
        .SetLayout(pl)
        .SetStage(dawn::ShaderStage::Vertex, vsModule, "main")
        .SetStage(dawn::ShaderStage::Fragment, fsReflectionModule, "main")
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
        glm::vec3(0.0f, -1.0f, 0.0f)
    );

    cameraBuffer.SetSubData(0, sizeof(CameraData), reinterpret_cast<uint8_t*>(&cameraData));

    dawn::Texture backbuffer;
    dawn::RenderPassDescriptor renderPass;
    GetNextRenderPassDescriptor(device, swapchain, depthStencilView, &backbuffer, &renderPass);

    dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass);
        pass.SetRenderPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup[0]);
        pass.SetVertexBuffers(0, 1, &vertexBuffer, vertexBufferOffsets);
        pass.SetIndexBuffer(indexBuffer, 0);
        pass.DrawElements(36, 1, 0, 0);

        pass.SetStencilReference(0x1);
        pass.SetRenderPipeline(planePipeline);
        pass.SetBindGroup(0, bindGroup[0]);
        pass.SetVertexBuffers(0, 1, &planeBuffer, vertexBufferOffsets);
        pass.DrawElements(6, 1, 0, 0);

        pass.SetRenderPipeline(reflectionPipeline);
        pass.SetVertexBuffers(0, 1, &vertexBuffer, vertexBufferOffsets);
        pass.SetBindGroup(0, bindGroup[1]);
        pass.DrawElements(36, 1, 0, 0);

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
