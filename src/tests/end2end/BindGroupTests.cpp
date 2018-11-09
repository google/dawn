// Copyright 2018 The Dawn Authors
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

#include "common/Assert.h"
#include "common/Constants.h"
#include "tests/DawnTest.h"
#include "utils/DawnHelpers.h"

constexpr static unsigned int kRTSize = 8;

class BindGroupTests : public DawnTest {
protected:
    dawn::CommandBuffer CreateSimpleComputeCommandBuffer(
            const dawn::ComputePipeline& pipeline, const dawn::BindGroup& bindGroup) {
        dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
        dawn::ComputePassEncoder pass = builder.BeginComputePass();
        pass.SetComputePipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Dispatch(1, 1, 1);
        pass.EndPass();
        return builder.GetResult();
    }
};

// Test a bindgroup reused in two command buffers in the same call to queue.Submit().
// This test passes by not asserting or crashing.
TEST_P(BindGroupTests, ReusedBindGroupSingleSubmit) {
    dawn::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device,
        {
            {0, dawn::ShaderStageBit::Compute, dawn::BindingType::UniformBuffer},
        }
    );
    dawn::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);

    const char* shader = R"(
        #version 450
        layout(std140, set = 0, binding = 0) uniform Contents {
            float f;
        } contents;
        void main() {
        }
    )";

    dawn::ShaderModule module =
        utils::CreateShaderModule(device, dawn::ShaderStage::Compute, shader);
    dawn::ComputePipelineDescriptor cpDesc;
    cpDesc.module = module;
    cpDesc.entryPoint = "main";
    cpDesc.layout = pl;
    dawn::ComputePipeline cp = device.CreateComputePipeline(&cpDesc);

    dawn::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(float);
    bufferDesc.usage = dawn::BufferUsageBit::TransferDst |
                       dawn::BufferUsageBit::Uniform;
    dawn::Buffer buffer = device.CreateBuffer(&bufferDesc);
    dawn::BufferView bufferView =
        buffer.CreateBufferViewBuilder().SetExtent(0, sizeof(float)).GetResult();
    dawn::BindGroup bindGroup = device.CreateBindGroupBuilder()
        .SetLayout(bgl)
        .SetBufferViews(0, 1, &bufferView)
        .GetResult();

    dawn::CommandBuffer cb[2];
    cb[0] = CreateSimpleComputeCommandBuffer(cp, bindGroup);
    cb[1] = CreateSimpleComputeCommandBuffer(cp, bindGroup);
    queue.Submit(2, cb);
}

// Test a bindgroup containing a UBO which is used in both the vertex and fragment shader.
// It contains a transformation matrix for the VS and the fragment color for the FS.
// These must result in different register offsets in the native APIs.
TEST_P(BindGroupTests, ReusedUBO) {
    // TODO(jiawei.shao@intel.com): find out why this test fails on Metal
    DAWN_SKIP_TEST_IF(IsMetal());

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    dawn::ShaderModule vsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
        #version 450
        layout (set = 0, binding = 0) uniform vertexUniformBuffer {
            mat2 transform;
        };
        void main() {
            const vec2 pos[3] = vec2[3](vec2(-1.f, -1.f), vec2(1.f, -1.f), vec2(-1.f, 1.f));
            gl_Position = vec4(transform * pos[gl_VertexIndex], 0.f, 1.f);
        })"
    );

    dawn::ShaderModule fsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
        #version 450
        layout (set = 0, binding = 1) uniform fragmentUniformBuffer {
            vec4 color;
        };
        layout(location = 0) out vec4 fragColor;
        void main() {
            fragColor = color;
        })"
    );

    dawn::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device,
        {
            {0, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
            {1, dawn::ShaderStageBit::Fragment, dawn::BindingType::UniformBuffer},
        }
    );
    dawn::PipelineLayout pipelineLayout = utils::MakeBasicPipelineLayout(device, &bgl);

    dawn::RenderPipeline pipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, renderPass.colorFormat)
        .SetLayout(pipelineLayout)
        .SetPrimitiveTopology(dawn::PrimitiveTopology::TriangleList)
        .SetStage(dawn::ShaderStage::Vertex, vsModule, "main")
        .SetStage(dawn::ShaderStage::Fragment, fsModule, "main")
        .GetResult();
    struct Data {
        float transform[8];
        char padding[256 - 8 * sizeof(float)];
        float color[4];
    };
    ASSERT(offsetof(Data, color) == 256);
    constexpr float dummy = 0.0f;
    Data data {
        { 1.f, 0.f, dummy, dummy, 0.f, 1.0f, dummy, dummy },
        { 0 },
        { 0.f, 1.f, 0.f, 1.f },
    };
    dawn::Buffer buffer = utils::CreateBufferFromData(device, &data, sizeof(data), dawn::BufferUsageBit::Uniform);
    dawn::BufferView vertUBOBufferView =
        buffer.CreateBufferViewBuilder().SetExtent(0, sizeof(Data::transform)).GetResult();
    dawn::BufferView fragUBOBufferView =
        buffer.CreateBufferViewBuilder().SetExtent(256, sizeof(Data::color)).GetResult();
    dawn::BindGroup bindGroup = device.CreateBindGroupBuilder()
        .SetLayout(bgl)
        .SetBufferViews(0, 1, &vertUBOBufferView)
        .SetBufferViews(1, 1, &fragUBOBufferView)
        .GetResult();

    dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
    dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass.renderPassInfo);
    pass.SetRenderPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DrawArrays(3, 1, 0, 0);
    pass.EndPass();

    dawn::CommandBuffer commands = builder.GetResult();
    queue.Submit(1, &commands);

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);
    int min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color,    min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color,    max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color,    min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Test a bindgroup containing a UBO in the vertex shader and a sampler and texture in the fragment shader.
// In D3D12 for example, these different types of bindings end up in different namespaces, but the register
// offsets used must match between the shader module and descriptor range.
TEST_P(BindGroupTests, UBOSamplerAndTexture) {
    // TODO(jiawei.shao@intel.com): find out why this test fails on Metal
    DAWN_SKIP_TEST_IF(IsMetal());

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    dawn::ShaderModule vsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
        #version 450
        layout (set = 0, binding = 0) uniform vertexUniformBuffer {
            mat2 transform;
        };
        void main() {
            const vec2 pos[3] = vec2[3](vec2(-1.f, -1.f), vec2(1.f, -1.f), vec2(-1.f, 1.f));
            gl_Position = vec4(transform * pos[gl_VertexIndex], 0.f, 1.f);
        })"
    );

    dawn::ShaderModule fsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
        #version 450
        layout (set = 0, binding = 1) uniform sampler samp;
        layout (set = 0, binding = 2) uniform texture2D tex;
        layout (location = 0) out vec4 fragColor;
        void main() {
            fragColor = texture(sampler2D(tex, samp), gl_FragCoord.xy);
        })"
    );

    dawn::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device,
        {
            {0, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
            {1, dawn::ShaderStageBit::Fragment, dawn::BindingType::Sampler},
            {2, dawn::ShaderStageBit::Fragment, dawn::BindingType::SampledTexture},
        }
    );
    dawn::PipelineLayout pipelineLayout = utils::MakeBasicPipelineLayout(device, &bgl);

    dawn::RenderPipeline pipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, renderPass.colorFormat)
        .SetLayout(pipelineLayout)
        .SetPrimitiveTopology(dawn::PrimitiveTopology::TriangleList)
        .SetStage(dawn::ShaderStage::Vertex, vsModule, "main")
        .SetStage(dawn::ShaderStage::Fragment, fsModule, "main")
        .GetResult();
    constexpr float dummy = 0.0f;
    constexpr float transform[] = { 1.f, 0.f, dummy, dummy, 0.f, 1.f, dummy, dummy };
    dawn::Buffer buffer = utils::CreateBufferFromData(device, &transform, sizeof(transform), dawn::BufferUsageBit::Uniform);
    dawn::BufferView vertUBOBufferView =
        buffer.CreateBufferViewBuilder().SetExtent(0, sizeof(transform)).GetResult();
    dawn::SamplerDescriptor samplerDescriptor;
    samplerDescriptor.minFilter = dawn::FilterMode::Nearest;
    samplerDescriptor.magFilter = dawn::FilterMode::Nearest;
    samplerDescriptor.mipmapFilter = dawn::FilterMode::Nearest;
    samplerDescriptor.addressModeU = dawn::AddressMode::ClampToEdge;
    samplerDescriptor.addressModeV = dawn::AddressMode::ClampToEdge;
    samplerDescriptor.addressModeW = dawn::AddressMode::ClampToEdge;
    dawn::Sampler sampler = device.CreateSampler(&samplerDescriptor);

    dawn::TextureDescriptor descriptor;
    descriptor.dimension = dawn::TextureDimension::e2D;
    descriptor.size.width = kRTSize;
    descriptor.size.height = kRTSize;
    descriptor.size.depth = 1;
    descriptor.arrayLayer = 1;
    descriptor.format = dawn::TextureFormat::R8G8B8A8Unorm;
    descriptor.levelCount = 1;
    descriptor.usage = dawn::TextureUsageBit::TransferDst | dawn::TextureUsageBit::Sampled;
    dawn::Texture texture = device.CreateTexture(&descriptor);
    dawn::TextureView textureView = texture.CreateDefaultTextureView();
    int width = kRTSize, height = kRTSize;
    int widthInBytes = width * sizeof(RGBA8);
    widthInBytes = (widthInBytes + 255) & ~255;
    int sizeInBytes = widthInBytes * height;
    int size = sizeInBytes / sizeof(RGBA8);
    std::vector<RGBA8> data = std::vector<RGBA8>(size);
    for (int i = 0; i < size; i++) {
        data[i] = RGBA8(0, 255, 0, 255);
    }
    dawn::Buffer stagingBuffer = utils::CreateBufferFromData(device, data.data(), sizeInBytes, dawn::BufferUsageBit::TransferSrc);
    dawn::BindGroup bindGroup = device.CreateBindGroupBuilder()
        .SetLayout(bgl)
        .SetBufferViews(0, 1, &vertUBOBufferView)
        .SetSamplers(1, 1, &sampler)
        .SetTextureViews(2, 1, &textureView)
        .GetResult();

    dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
    builder.CopyBufferToTexture(stagingBuffer, 0, widthInBytes, texture, 0, 0, 0, width, height, 1, 0, 0);
    dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass.renderPassInfo);
    pass.SetRenderPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DrawArrays(3, 1, 0, 0);
    pass.EndPass();

    dawn::CommandBuffer commands = builder.GetResult();
    queue.Submit(1, &commands);

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);
    int min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color,    min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color,    max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color,    min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

DAWN_INSTANTIATE_TEST(BindGroupTests, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend);
