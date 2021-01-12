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
#include "common/Math.h"
#include "tests/DawnTest.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

constexpr static uint32_t kRTSize = 8;

class BindGroupTests : public DawnTest {
  protected:
    wgpu::CommandBuffer CreateSimpleComputeCommandBuffer(const wgpu::ComputePipeline& pipeline,
                                                         const wgpu::BindGroup& bindGroup) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Dispatch(1);
        pass.EndPass();
        return encoder.Finish();
    }

    wgpu::PipelineLayout MakeBasicPipelineLayout(
        std::vector<wgpu::BindGroupLayout> bindingInitializer) const {
        wgpu::PipelineLayoutDescriptor descriptor;

        descriptor.bindGroupLayoutCount = bindingInitializer.size();
        descriptor.bindGroupLayouts = bindingInitializer.data();

        return device.CreatePipelineLayout(&descriptor);
    }

    wgpu::ShaderModule MakeSimpleVSModule() const {
        return utils::CreateShaderModuleFromWGSL(device, R"(
        [[builtin(vertex_idx)]] var<in> VertexIndex : u32;
        [[builtin(position)]] var<out> Position : vec4<f32>;

        [[stage(vertex)]] fn main() -> void {
             const pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
                vec2<f32>(-1.0, 1.0),
                vec2<f32>( 1.0, 1.0),
                vec2<f32>(-1.0, -1.0));

            Position = vec4<f32>(pos[VertexIndex], 0.0, 1.0);
        })");
    }

    wgpu::ShaderModule MakeFSModule(std::vector<wgpu::BufferBindingType> bindingTypes) const {
        ASSERT(bindingTypes.size() <= kMaxBindGroups);

        std::ostringstream fs;
        fs << "[[location(0)]] var<out> fragColor : vec4<f32>;\n";

        for (size_t i = 0; i < bindingTypes.size(); ++i) {
            fs << "[[block]] struct Buffer" << i << R"( {
                [[offset(0)]] color : vec4<f32>;
            };)";

            switch (bindingTypes[i]) {
                case wgpu::BufferBindingType::Uniform:
                    fs << "\n[[set(" << i << "), binding(0)]] var<uniform> buffer" << i
                       << " : Buffer" << i << ";";
                    break;
                case wgpu::BufferBindingType::Storage:
                    fs << "\n[[set(" << i << "), binding(0)]] var<storage_buffer> buffer" << i
                       << " : [[access(read)]] Buffer" << i << ";";
                    break;
                default:
                    UNREACHABLE();
            }
        }

        fs << "\n[[stage(fragment)]] fn main() -> void {\n";
        for (size_t i = 0; i < bindingTypes.size(); ++i) {
            fs << "fragColor = fragColor + buffer" << i << ".color;\n";
        }
        fs << "}\n";
        return utils::CreateShaderModuleFromWGSL(device, fs.str().c_str());
    }

    wgpu::RenderPipeline MakeTestPipeline(const utils::BasicRenderPass& renderPass,
                                          std::vector<wgpu::BufferBindingType> bindingTypes,
                                          std::vector<wgpu::BindGroupLayout> bindGroupLayouts) {
        wgpu::ShaderModule vsModule = MakeSimpleVSModule();
        wgpu::ShaderModule fsModule = MakeFSModule(bindingTypes);

        wgpu::PipelineLayout pipelineLayout = MakeBasicPipelineLayout(bindGroupLayouts);

        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);
        pipelineDescriptor.layout = pipelineLayout;
        pipelineDescriptor.vertexStage.module = vsModule;
        pipelineDescriptor.cFragmentStage.module = fsModule;
        pipelineDescriptor.cColorStates[0].format = renderPass.colorFormat;
        pipelineDescriptor.cColorStates[0].colorBlend.operation = wgpu::BlendOperation::Add;
        pipelineDescriptor.cColorStates[0].colorBlend.srcFactor = wgpu::BlendFactor::One;
        pipelineDescriptor.cColorStates[0].colorBlend.dstFactor = wgpu::BlendFactor::One;
        pipelineDescriptor.cColorStates[0].alphaBlend.operation = wgpu::BlendOperation::Add;
        pipelineDescriptor.cColorStates[0].alphaBlend.srcFactor = wgpu::BlendFactor::One;
        pipelineDescriptor.cColorStates[0].alphaBlend.dstFactor = wgpu::BlendFactor::One;

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }
};

// Test a bindgroup reused in two command buffers in the same call to queue.Submit().
// This test passes by not asserting or crashing.
TEST_P(BindGroupTests, ReusedBindGroupSingleSubmit) {
    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct Contents {
            [[offset(0)]] f : f32;
        };
        [[set(0), binding(0)]] var <uniform> contents: Contents;

        [[stage(compute)]] fn main() -> void {
        })");

    wgpu::ComputePipelineDescriptor cpDesc;
    cpDesc.computeStage.module = module;
    cpDesc.computeStage.entryPoint = "main";
    wgpu::ComputePipeline cp = device.CreateComputePipeline(&cpDesc);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(float);
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, cp.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer cb[2];
    cb[0] = CreateSimpleComputeCommandBuffer(cp, bindGroup);
    cb[1] = CreateSimpleComputeCommandBuffer(cp, bindGroup);
    queue.Submit(2, cb);
}

// Test a bindgroup containing a UBO which is used in both the vertex and fragment shader.
// It contains a transformation matrix for the VS and the fragment color for the FS.
// These must result in different register offsets in the native APIs.
TEST_P(BindGroupTests, ReusedUBO) {
    // TODO(crbug.com/dawn/571): Fix failures using Tint.
    DAWN_SKIP_TEST_IF(HasToggleEnabled("use_tint_generator") &&
                      (IsVulkan() || IsOpenGL() || IsOpenGLES()));

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        // TODO(crbug.com/tint/369): Use a mat2x2 when Tint translates it correctly.
        [[block]] struct VertexUniformBuffer {
            [[offset(0)]] transform : vec4<f32>;
        };

        [[set(0), binding(0)]] var <uniform> vertexUbo : VertexUniformBuffer;

        [[builtin(vertex_idx)]] var<in> VertexIndex : u32;
        [[builtin(position)]] var<out> Position : vec4<f32>;

        [[stage(vertex)]] fn main() -> void {
            const pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
                vec2<f32>(-1.0, 1.0),
                vec2<f32>( 1.0, 1.0),
                vec2<f32>(-1.0, -1.0));

            var transform : mat2x2<f32> = mat2x2<f32>(
                vec2<f32>(vertexUbo.transform[0], vertexUbo.transform[1]),
                vec2<f32>(vertexUbo.transform[2], vertexUbo.transform[3]));
            Position = vec4<f32>(transform * pos[VertexIndex], 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct FragmentUniformBuffer {
            [[offset(0)]] color : vec4<f32>;
        };
        [[set(0), binding(1)]] var <uniform> fragmentUbo : FragmentUniformBuffer;

        [[location(0)]] var<out> fragColor : vec4<f32>;

        [[stage(fragment)]] fn main() -> void {
            fragColor = fragmentUbo.color;
        })");

    utils::ComboRenderPipelineDescriptor textureDescriptor(device);
    textureDescriptor.vertexStage.module = vsModule;
    textureDescriptor.cFragmentStage.module = fsModule;
    textureDescriptor.cColorStates[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&textureDescriptor);

    struct Data {
        float transform[8];
        char padding[256 - 8 * sizeof(float)];
        float color[4];
    };
    ASSERT(offsetof(Data, color) == 256);
    Data data{
        {1.f, 0.f, 0.f, 1.0f},
        {0},
        {0.f, 1.f, 0.f, 1.f},
    };
    wgpu::Buffer buffer =
        utils::CreateBufferFromData(device, &data, sizeof(data), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(
        device, pipeline.GetBindGroupLayout(0),
        {{0, buffer, 0, sizeof(Data::transform)}, {1, buffer, 256, sizeof(Data::color)}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(3);
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Test a bindgroup containing a UBO in the vertex shader and a sampler and texture in the fragment
// shader. In D3D12 for example, these different types of bindings end up in different namespaces,
// but the register offsets used must match between the shader module and descriptor range.
TEST_P(BindGroupTests, UBOSamplerAndTexture) {
    // TODO(crbug.com/dawn/571): Fix failures using Tint.
    DAWN_SKIP_TEST_IF(HasToggleEnabled("use_tint_generator") &&
                      (IsVulkan() || IsOpenGL() || IsOpenGLES()));

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        // TODO(crbug.com/tint/369): Use a mat2x2 when Tint translates it correctly.
        [[block]] struct VertexUniformBuffer {
            [[offset(0)]] transform : vec4<f32>;
        };
        [[set(0), binding(0)]] var <uniform> vertexUbo : VertexUniformBuffer;

        [[builtin(vertex_idx)]] var<in> VertexIndex : u32;
        [[builtin(position)]] var<out> Position : vec4<f32>;

        [[stage(vertex)]] fn main() -> void {
            const pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
                vec2<f32>(-1.0, 1.0),
                vec2<f32>( 1.0, 1.0),
                vec2<f32>(-1.0, -1.0));

            var transform : mat2x2<f32> = mat2x2<f32>(
                vec2<f32>(vertexUbo.transform[0], vertexUbo.transform[1]),
                vec2<f32>(vertexUbo.transform[2], vertexUbo.transform[3]));
            Position = vec4<f32>(transform * pos[VertexIndex], 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[set(0), binding(1)]] var <uniform_constant> samp : sampler;
        [[set(0), binding(2)]] var <uniform_constant> tex : texture_2d<f32>;
        [[builtin(frag_coord)]] var<in> FragCoord : vec4<f32>;

        [[location(0)]] var<out> fragColor : vec4<f32>;

        [[stage(fragment)]] fn main() -> void {
            fragColor = textureSample(tex, samp, FragCoord.xy);
        })");

    utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);
    pipelineDescriptor.vertexStage.module = vsModule;
    pipelineDescriptor.cFragmentStage.module = fsModule;
    pipelineDescriptor.cColorStates[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    constexpr float transform[] = {1.f, 0.f, 0.f, 1.f};
    wgpu::Buffer buffer = utils::CreateBufferFromData(device, &transform, sizeof(transform),
                                                      wgpu::BufferUsage::Uniform);

    wgpu::SamplerDescriptor samplerDescriptor = {};
    samplerDescriptor.minFilter = wgpu::FilterMode::Nearest;
    samplerDescriptor.magFilter = wgpu::FilterMode::Nearest;
    samplerDescriptor.mipmapFilter = wgpu::FilterMode::Nearest;
    samplerDescriptor.addressModeU = wgpu::AddressMode::ClampToEdge;
    samplerDescriptor.addressModeV = wgpu::AddressMode::ClampToEdge;
    samplerDescriptor.addressModeW = wgpu::AddressMode::ClampToEdge;

    wgpu::Sampler sampler = device.CreateSampler(&samplerDescriptor);

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = kRTSize;
    descriptor.size.height = kRTSize;
    descriptor.size.depth = 1;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::Sampled;
    wgpu::Texture texture = device.CreateTexture(&descriptor);
    wgpu::TextureView textureView = texture.CreateView();

    uint32_t width = kRTSize, height = kRTSize;
    uint32_t widthInBytes = width * sizeof(RGBA8);
    widthInBytes = (widthInBytes + 255) & ~255;
    uint32_t sizeInBytes = widthInBytes * height;
    uint32_t size = sizeInBytes / sizeof(RGBA8);
    std::vector<RGBA8> data = std::vector<RGBA8>(size);
    for (uint32_t i = 0; i < size; i++) {
        data[i] = RGBA8(0, 255, 0, 255);
    }
    wgpu::Buffer stagingBuffer =
        utils::CreateBufferFromData(device, data.data(), sizeInBytes, wgpu::BufferUsage::CopySrc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                             {{0, buffer, 0, sizeof(transform)}, {1, sampler}, {2, textureView}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::BufferCopyView bufferCopyView =
        utils::CreateBufferCopyView(stagingBuffer, 0, widthInBytes);
    wgpu::TextureCopyView textureCopyView = utils::CreateTextureCopyView(texture, 0, {0, 0, 0});
    wgpu::Extent3D copySize = {width, height, 1};
    encoder.CopyBufferToTexture(&bufferCopyView, &textureCopyView, &copySize);
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(3);
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    RGBA8 filled(0, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

TEST_P(BindGroupTests, MultipleBindLayouts) {
    // TODO(crbug.com/tint/403):
    // error: line 74: Expected Result Type to be a scalar type
    // %44 = OpVectorExtractDynamic %v2float %30 %uint_0_0
    DAWN_SKIP_TEST_IF(HasToggleEnabled("use_tint_generator") &&
                      (IsVulkan() || IsOpenGL() || IsOpenGLES()));

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        // TODO(crbug.com/tint/369): Use a mat2x2 when Tint translates it correctly.
        // TODO(crbug.com/tint/386): Use the same struct.
        [[block]] struct VertexUniformBuffer1 {
            [[offset(0)]] transform : vec4<f32>;
        };

        [[block]] struct VertexUniformBuffer2 {
            [[offset(0)]] transform : vec4<f32>;
        };

        // TODO(crbug.com/tint/386): Use the same struct definition.
        [[set(0), binding(0)]] var <uniform> vertexUbo1 : VertexUniformBuffer1;
        [[set(1), binding(0)]] var <uniform> vertexUbo2 : VertexUniformBuffer2;

        [[builtin(vertex_idx)]] var<in> VertexIndex : u32;
        [[builtin(position)]] var<out> Position : vec4<f32>;

        [[stage(vertex)]] fn main() -> void {
            const pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
                vec2<f32>(-1.0, 1.0),
                vec2<f32>( 1.0, 1.0),
                vec2<f32>(-1.0, -1.0));

            Position = vec4<f32>(mat2x2<f32>(
                vertexUbo1.transform.xy + vertexUbo2.transform.xy,
                vertexUbo1.transform.zw + vertexUbo2.transform.zw
            ) * pos[VertexIndex], 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        // TODO(crbug.com/tint/386): Use the same struct
        [[block]] struct FragmentUniformBuffer1 {
            [[offset(0)]] color : vec4<f32>;
        };

        [[block]] struct FragmentUniformBuffer2 {
            [[offset(0)]] color : vec4<f32>;
        };

        // TODO(crbug.com/tint/386): Use the same struct definition.
        [[set(0), binding(1)]] var <uniform> fragmentUbo1 : FragmentUniformBuffer1;
        [[set(1), binding(1)]] var <uniform> fragmentUbo2 : FragmentUniformBuffer2;

        [[location(0)]] var<out> fragColor : vec4<f32>;

        [[stage(fragment)]] fn main() -> void {
            fragColor = fragmentUbo1.color + fragmentUbo2.color;
        })");

    utils::ComboRenderPipelineDescriptor textureDescriptor(device);
    textureDescriptor.vertexStage.module = vsModule;
    textureDescriptor.cFragmentStage.module = fsModule;
    textureDescriptor.cColorStates[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&textureDescriptor);

    struct Data {
        float transform[4];
        char padding[256 - 4 * sizeof(float)];
        float color[4];
    };
    ASSERT(offsetof(Data, color) == 256);

    std::vector<Data> data;
    std::vector<wgpu::Buffer> buffers;
    std::vector<wgpu::BindGroup> bindGroups;

    data.push_back({{1.0f, 0.0f, 0.0f, 0.0f}, {0}, {0.0f, 1.0f, 0.0f, 1.0f}});

    data.push_back({{0.0f, 0.0f, 0.0f, 1.0f}, {0}, {1.0f, 0.0f, 0.0f, 1.0f}});

    for (int i = 0; i < 2; i++) {
        wgpu::Buffer buffer =
            utils::CreateBufferFromData(device, &data[i], sizeof(Data), wgpu::BufferUsage::Uniform);
        buffers.push_back(buffer);
        bindGroups.push_back(utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {{0, buffers[i], 0, sizeof(Data::transform)},
                                                   {1, buffers[i], 256, sizeof(Data::color)}}));
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroups[0]);
    pass.SetBindGroup(1, bindGroups[1]);
    pass.Draw(3);
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    RGBA8 filled(255, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// This test reproduces an out-of-bound bug on D3D12 backends when calling draw command twice with
// one pipeline that has 4 bind group sets in one render pass.
TEST_P(BindGroupTests, DrawTwiceInSamePipelineWithFourBindGroupSets) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});

    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(renderPass,
                         {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Uniform,
                          wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Uniform},
                         {layout, layout, layout, layout});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    pass.SetPipeline(pipeline);

    // The color will be added 8 times, so the value should be 0.125. But we choose 0.126
    // because of precision issues on some devices (for example NVIDIA bots).
    std::array<float, 4> color = {0.126, 0, 0, 0.126};
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, &color, sizeof(color), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, layout, {{0, uniformBuffer, 0, sizeof(color)}});

    pass.SetBindGroup(0, bindGroup);
    pass.SetBindGroup(1, bindGroup);
    pass.SetBindGroup(2, bindGroup);
    pass.SetBindGroup(3, bindGroup);
    pass.Draw(3);

    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    RGBA8 filled(255, 0, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Test that bind groups can be set before the pipeline.
TEST_P(BindGroupTests, SetBindGroupBeforePipeline) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Create a bind group layout which uses a single uniform buffer.
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});

    // Create a pipeline that uses the uniform bind group layout.
    wgpu::RenderPipeline pipeline =
        MakeTestPipeline(renderPass, {wgpu::BufferBindingType::Uniform}, {layout});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    // Create a bind group with a uniform buffer and fill it with RGBAunorm(1, 0, 0, 1).
    std::array<float, 4> color = {1, 0, 0, 1};
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, &color, sizeof(color), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, layout, {{0, uniformBuffer, 0, sizeof(color)}});

    // Set the bind group, then the pipeline, and draw.
    pass.SetBindGroup(0, bindGroup);
    pass.SetPipeline(pipeline);
    pass.Draw(3);

    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The result should be red.
    RGBA8 filled(255, 0, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Test that dynamic bind groups can be set before the pipeline.
TEST_P(BindGroupTests, SetDynamicBindGroupBeforePipeline) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Create a bind group layout which uses a single dynamic uniform buffer.
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, true}});

    // Create a pipeline that uses the dynamic uniform bind group layout for two bind groups.
    wgpu::RenderPipeline pipeline = MakeTestPipeline(
        renderPass, {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Uniform},
        {layout, layout});

    // Prepare data RGBAunorm(1, 0, 0, 0.5) and RGBAunorm(0, 1, 0, 0.5). They will be added in the
    // shader.
    std::array<float, 4> color0 = {1, 0, 0, 0.501};
    std::array<float, 4> color1 = {0, 1, 0, 0.501};

    size_t color1Offset = Align(sizeof(color0), kMinDynamicBufferOffsetAlignment);

    std::vector<uint8_t> data(color1Offset + sizeof(color1));
    memcpy(data.data(), color0.data(), sizeof(color0));
    memcpy(data.data() + color1Offset, color1.data(), sizeof(color1));

    // Create a bind group and uniform buffer with the color data. It will be bound at the offset
    // to each color.
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, data.data(), data.size(), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, layout, {{0, uniformBuffer, 0, 4 * sizeof(float)}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    // Set the first dynamic bind group.
    uint32_t dynamicOffset = 0;
    pass.SetBindGroup(0, bindGroup, 1, &dynamicOffset);

    // Set the second dynamic bind group.
    dynamicOffset = color1Offset;
    pass.SetBindGroup(1, bindGroup, 1, &dynamicOffset);

    // Set the pipeline and draw.
    pass.SetPipeline(pipeline);
    pass.Draw(3);

    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The result should be RGBAunorm(1, 0, 0, 0.5) + RGBAunorm(0, 1, 0, 0.5)
    RGBA8 filled(255, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Test that bind groups set for one pipeline are still set when the pipeline changes.
TEST_P(BindGroupTests, BindGroupsPersistAfterPipelineChange) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Create a bind group layout which uses a single dynamic uniform buffer.
    wgpu::BindGroupLayout uniformLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, true}});

    // Create a bind group layout which uses a single dynamic storage buffer.
    wgpu::BindGroupLayout storageLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage, true}});

    // Create a pipeline which uses the uniform buffer and storage buffer bind groups.
    wgpu::RenderPipeline pipeline0 = MakeTestPipeline(
        renderPass, {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Storage},
        {uniformLayout, storageLayout});

    // Create a pipeline which uses the uniform buffer bind group twice.
    wgpu::RenderPipeline pipeline1 = MakeTestPipeline(
        renderPass, {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Uniform},
        {uniformLayout, uniformLayout});

    // Prepare data RGBAunorm(1, 0, 0, 0.5) and RGBAunorm(0, 1, 0, 0.5). They will be added in the
    // shader.
    std::array<float, 4> color0 = {1, 0, 0, 0.5};
    std::array<float, 4> color1 = {0, 1, 0, 0.5};

    size_t color1Offset = Align(sizeof(color0), kMinDynamicBufferOffsetAlignment);

    std::vector<uint8_t> data(color1Offset + sizeof(color1));
    memcpy(data.data(), color0.data(), sizeof(color0));
    memcpy(data.data() + color1Offset, color1.data(), sizeof(color1));

    // Create a bind group and uniform buffer with the color data. It will be bound at the offset
    // to each color.
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, data.data(), data.size(), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, uniformLayout, {{0, uniformBuffer, 0, 4 * sizeof(float)}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    // Set the first pipeline (uniform, storage).
    pass.SetPipeline(pipeline0);

    // Set the first bind group at a dynamic offset.
    // This bind group matches the slot in the pipeline layout.
    uint32_t dynamicOffset = 0;
    pass.SetBindGroup(0, bindGroup, 1, &dynamicOffset);

    // Set the second bind group at a dynamic offset.
    // This bind group does not match the slot in the pipeline layout.
    dynamicOffset = color1Offset;
    pass.SetBindGroup(1, bindGroup, 1, &dynamicOffset);

    // Set the second pipeline (uniform, uniform).
    // Both bind groups match the pipeline.
    // They should persist and not need to be bound again.
    pass.SetPipeline(pipeline1);
    pass.Draw(3);

    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The result should be RGBAunorm(1, 0, 0, 0.5) + RGBAunorm(0, 1, 0, 0.5)
    RGBA8 filled(255, 255, 0, 255);
    RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Do a successful draw. Then, change the pipeline and one bind group.
// Draw to check that the all bind groups are set.
TEST_P(BindGroupTests, DrawThenChangePipelineAndBindGroup) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Create a bind group layout which uses a single dynamic uniform buffer.
    wgpu::BindGroupLayout uniformLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, true}});

    // Create a bind group layout which uses a single dynamic storage buffer.
    wgpu::BindGroupLayout storageLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage, true}});

    // Create a pipeline with pipeline layout (uniform, uniform, storage).
    wgpu::RenderPipeline pipeline0 =
        MakeTestPipeline(renderPass,
                         {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Uniform,
                          wgpu::BufferBindingType::Storage},
                         {uniformLayout, uniformLayout, storageLayout});

    // Create a pipeline with pipeline layout (uniform, storage, storage).
    wgpu::RenderPipeline pipeline1 =
        MakeTestPipeline(renderPass,
                         {wgpu::BufferBindingType::Uniform, wgpu::BufferBindingType::Storage,
                          wgpu::BufferBindingType::Storage},
                         {uniformLayout, storageLayout, storageLayout});

    // Prepare color data.
    // The first draw will use { color0, color1, color2 }.
    // The second draw will use { color0, color3, color2 }.
    // The pipeline uses additive color and alpha blending so the result of two draws should be
    // { 2 * color0 + color1 + 2 * color2 + color3} = RGBAunorm(1, 1, 1, 1)
    std::array<float, 4> color0 = {0.501, 0, 0, 0};
    std::array<float, 4> color1 = {0, 1, 0, 0};
    std::array<float, 4> color2 = {0, 0, 0, 0.501};
    std::array<float, 4> color3 = {0, 0, 1, 0};

    size_t color1Offset = Align(sizeof(color0), kMinDynamicBufferOffsetAlignment);
    size_t color2Offset = Align(color1Offset + sizeof(color1), kMinDynamicBufferOffsetAlignment);
    size_t color3Offset = Align(color2Offset + sizeof(color2), kMinDynamicBufferOffsetAlignment);

    std::vector<uint8_t> data(color3Offset + sizeof(color3), 0);
    memcpy(data.data(), color0.data(), sizeof(color0));
    memcpy(data.data() + color1Offset, color1.data(), sizeof(color1));
    memcpy(data.data() + color2Offset, color2.data(), sizeof(color2));
    memcpy(data.data() + color3Offset, color3.data(), sizeof(color3));

    // Create a uniform and storage buffer bind groups to bind the color data.
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, data.data(), data.size(), wgpu::BufferUsage::Uniform);

    wgpu::Buffer storageBuffer =
        utils::CreateBufferFromData(device, data.data(), data.size(), wgpu::BufferUsage::Storage);

    wgpu::BindGroup uniformBindGroup =
        utils::MakeBindGroup(device, uniformLayout, {{0, uniformBuffer, 0, 4 * sizeof(float)}});
    wgpu::BindGroup storageBindGroup =
        utils::MakeBindGroup(device, storageLayout, {{0, storageBuffer, 0, 4 * sizeof(float)}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    // Set the pipeline to (uniform, uniform, storage)
    pass.SetPipeline(pipeline0);

    // Set the first bind group to color0 in the dynamic uniform buffer.
    uint32_t dynamicOffset = 0;
    pass.SetBindGroup(0, uniformBindGroup, 1, &dynamicOffset);

    // Set the first bind group to color1 in the dynamic uniform buffer.
    dynamicOffset = color1Offset;
    pass.SetBindGroup(1, uniformBindGroup, 1, &dynamicOffset);

    // Set the first bind group to color2 in the dynamic storage buffer.
    dynamicOffset = color2Offset;
    pass.SetBindGroup(2, storageBindGroup, 1, &dynamicOffset);

    pass.Draw(3);

    // Set the pipeline to (uniform, storage, storage)
    //  - The first bind group should persist (inherited on some backends)
    //  - The second bind group needs to be set again to pass validation.
    //    It changed from uniform to storage.
    //  - The third bind group should persist. It should be set again by the backend internally.
    pass.SetPipeline(pipeline1);

    // Set the second bind group to color3 in the dynamic storage buffer.
    dynamicOffset = color3Offset;
    pass.SetBindGroup(1, storageBindGroup, 1, &dynamicOffset);

    pass.Draw(3);
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    RGBA8 filled(255, 255, 255, 255);
    RGBA8 notFilled(0, 0, 0, 0);
    uint32_t min = 1, max = kRTSize - 3;
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, max, min);
    EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, min, max);
    EXPECT_PIXEL_RGBA8_EQ(notFilled, renderPass.color, max, max);
}

// Regression test for crbug.com/dawn/408 where dynamic offsets were applied in the wrong order.
// Dynamic offsets should be applied in increasing order of binding number.
TEST_P(BindGroupTests, DynamicOffsetOrder) {
    // We will put the following values and the respective offsets into a buffer.
    // The test will ensure that the correct dynamic offset is applied to each buffer by reading the
    // value from an offset binding.
    std::array<uint32_t, 3> offsets = {3 * kMinDynamicBufferOffsetAlignment,
                                       1 * kMinDynamicBufferOffsetAlignment,
                                       2 * kMinDynamicBufferOffsetAlignment};
    std::array<uint32_t, 3> values = {21, 67, 32};

    // Create three buffers large enough to by offset by the largest offset.
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 3 * kMinDynamicBufferOffsetAlignment + sizeof(uint32_t);
    bufferDescriptor.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer0 = device.CreateBuffer(&bufferDescriptor);
    wgpu::Buffer buffer3 = device.CreateBuffer(&bufferDescriptor);

    // This test uses both storage and uniform buffers to ensure buffer bindings are sorted first by
    // binding number before type.
    bufferDescriptor.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer2 = device.CreateBuffer(&bufferDescriptor);

    // Populate the values
    queue.WriteBuffer(buffer0, offsets[0], &values[0], sizeof(uint32_t));
    queue.WriteBuffer(buffer2, offsets[1], &values[1], sizeof(uint32_t));
    queue.WriteBuffer(buffer3, offsets[2], &values[2], sizeof(uint32_t));

    wgpu::Buffer outputBuffer = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage, {0, 0, 0});

    // Create the bind group and bind group layout.
    // Note: The order of the binding numbers are intentionally different and not in increasing
    // order.
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {
                    {3, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage, true},
                    {0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage, true},
                    {2, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform, true},
                    {4, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
                });
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl,
                                                     {
                                                         {0, buffer0, 0, sizeof(uint32_t)},
                                                         {3, buffer3, 0, sizeof(uint32_t)},
                                                         {2, buffer2, 0, sizeof(uint32_t)},
                                                         {4, outputBuffer, 0, 3 * sizeof(uint32_t)},
                                                     });

    wgpu::ComputePipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.computeStage.module = utils::CreateShaderModuleFromWGSL(device, R"(
        // TODO(crbug.com/tint/386): Use the same struct
        [[block]] struct Buffer0 {
            [[offset(0)]] value : u32;
        };

        [[block]] struct Buffer2 {
            [[offset(0)]] value : u32;
        };

        [[block]] struct Buffer3 {
            [[offset(0)]] value : u32;
        };

        [[block]] struct OutputBuffer {
            [[offset(0)]] value : vec3<u32>;
        };

        [[set(0), binding(2)]] var<uniform> buffer2 : Buffer2;
        [[set(0), binding(3)]] var<storage_buffer> buffer3 : [[access(read)]] Buffer3;
        [[set(0), binding(0)]] var<storage_buffer> buffer0 : [[access(read)]] Buffer0;
        [[set(0), binding(4)]] var<storage_buffer> outputBuffer : [[access(read_write)]] OutputBuffer;

        [[stage(compute)]] fn main() -> void {
            outputBuffer.value = vec3<u32>(buffer0.value, buffer2.value, buffer3.value);
        })");
    pipelineDescriptor.computeStage.entryPoint = "main";
    pipelineDescriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDescriptor);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.SetBindGroup(0, bindGroup, offsets.size(), offsets.data());
    computePassEncoder.Dispatch(1);
    computePassEncoder.EndPass();

    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(values.data(), outputBuffer, 0, values.size());
}

// Test that visibility of bindings in BindGroupLayout can be none
// This test passes by not asserting or crashing.
TEST_P(BindGroupTests, BindGroupLayoutVisibilityCanBeNone) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::BindGroupLayoutEntry entry;
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::None;
    entry.buffer.type = wgpu::BufferBindingType::Uniform;
    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = 1;
    descriptor.entries = &entry;
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&descriptor);

    wgpu::RenderPipeline pipeline = MakeTestPipeline(renderPass, {}, {layout});

    std::array<float, 4> color = {1, 0, 0, 1};
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, &color, sizeof(color), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, layout, {{0, uniformBuffer, 0, sizeof(color)}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(3);
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

// Regression test for crbug.com/dawn/448 that dynamic buffer bindings can have None visibility.
TEST_P(BindGroupTests, DynamicBindingNoneVisibility) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::BindGroupLayoutEntry entry;
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::None;
    entry.buffer.type = wgpu::BufferBindingType::Uniform;
    entry.buffer.hasDynamicOffset = true;
    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = 1;
    descriptor.entries = &entry;
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&descriptor);

    wgpu::RenderPipeline pipeline = MakeTestPipeline(renderPass, {}, {layout});

    std::array<float, 4> color = {1, 0, 0, 1};
    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData(device, &color, sizeof(color), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, layout, {{0, uniformBuffer, 0, sizeof(color)}});

    uint32_t dynamicOffset = 0;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup, 1, &dynamicOffset);
    pass.Draw(3);
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

// Test that bind group bindings may have unbounded and arbitrary binding numbers
TEST_P(BindGroupTests, ArbitraryBindingNumbers) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[builtin(vertex_idx)]] var<in> VertexIndex : u32;
        [[builtin(position)]] var<out> Position : vec4<f32>;

        [[stage(vertex)]] fn main() -> void {
            const pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
                vec2<f32>(-1.0, 1.0),
                vec2<f32>( 1.0, 1.0),
                vec2<f32>(-1.0, -1.0));

            Position = vec4<f32>(pos[VertexIndex], 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        // TODO(crbug.com/tint/386): Use the same struct
        [[block]] struct Ubo1 {
            [[offset(0)]] color : vec4<f32>;
        };

        [[block]] struct Ubo2 {
            [[offset(0)]] color : vec4<f32>;
        };

        [[block]] struct Ubo3 {
            [[offset(0)]] color : vec4<f32>;
        };

        // TODO(crbug.com/tint/386): Use the same struct definition.
        [[set(0), binding(953)]] var <uniform> ubo1 : Ubo1;
        [[set(0), binding(47)]] var <uniform> ubo2 : Ubo2;
        [[set(0), binding(111)]] var <uniform> ubo3 : Ubo3;

        [[location(0)]] var<out> fragColor : vec4<f32>;

        [[stage(fragment)]] fn main() -> void {
            fragColor = ubo1.color + 2.0 * ubo2.color + 4.0 * ubo3.color;
        })");

    utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);
    pipelineDescriptor.vertexStage.module = vsModule;
    pipelineDescriptor.cFragmentStage.module = fsModule;
    pipelineDescriptor.cColorStates[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    wgpu::Buffer black =
        utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {0.f, 0.f, 0.f, 0.f});
    wgpu::Buffer red =
        utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {0.251f, 0.0f, 0.0f, 0.0f});
    wgpu::Buffer green =
        utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {0.0f, 0.251f, 0.0f, 0.0f});
    wgpu::Buffer blue =
        utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {0.0f, 0.0f, 0.251f, 0.0f});

    auto DoTest = [&](wgpu::Buffer color1, wgpu::Buffer color2, wgpu::Buffer color3, RGBA8 filled) {
        auto DoTestInner = [&](wgpu::BindGroup bindGroup) {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(3);
            pass.EndPass();

            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);

            EXPECT_PIXEL_RGBA8_EQ(filled, renderPass.color, 1, 1);
        };

        utils::BindingInitializationHelper bindings[] = {
            {953, color1, 0, 4 * sizeof(float)},  //
            {47, color2, 0, 4 * sizeof(float)},   //
            {111, color3, 0, 4 * sizeof(float)},  //
        };

        // Should work regardless of what order the bindings are specified in.
        DoTestInner(utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                         {bindings[0], bindings[1], bindings[2]}));
        DoTestInner(utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                         {bindings[1], bindings[0], bindings[2]}));
        DoTestInner(utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                         {bindings[2], bindings[0], bindings[1]}));
    };

    // first color is normal, second is 2x, third is 3x.
    DoTest(black, black, black, RGBA8(0, 0, 0, 0));

    // Check the first binding maps to the first slot. We know this because the colors are
    // multiplied 1x.
    DoTest(red, black, black, RGBA8(64, 0, 0, 0));
    DoTest(green, black, black, RGBA8(0, 64, 0, 0));
    DoTest(blue, black, black, RGBA8(0, 0, 64, 0));

    // Use multiple bindings and check the second color maps to the second slot.
    // We know this because the second slot is multiplied 2x.
    DoTest(green, blue, black, RGBA8(0, 64, 128, 0));
    DoTest(blue, green, black, RGBA8(0, 128, 64, 0));
    DoTest(red, green, black, RGBA8(64, 128, 0, 0));

    // Use multiple bindings and check the third color maps to the third slot.
    // We know this because the third slot is multiplied 4x.
    DoTest(black, blue, red, RGBA8(255, 0, 128, 0));
    DoTest(blue, black, green, RGBA8(0, 255, 64, 0));
    DoTest(red, black, blue, RGBA8(64, 0, 255, 0));
}

// This is a regression test for crbug.com/dawn/355 which tests that destruction of a bind group
// that holds the last reference to its bind group layout does not result in a use-after-free. In
// the bug, the destructor of BindGroupBase, when destroying member mLayout,
// Ref<BindGroupLayoutBase> assigns to Ref::mPointee, AFTER calling Release(). After the BGL is
// destroyed, the storage for |mPointee| has been freed.
TEST_P(BindGroupTests, LastReferenceToBindGroupLayout) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(float);
    bufferDesc.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    wgpu::BindGroup bg;
    {
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}});
        bg = utils::MakeBindGroup(device, bgl, {{0, buffer, 0, sizeof(float)}});
    }
}

// Test that bind groups with an empty bind group layout may be created and used.
TEST_P(BindGroupTests, EmptyLayout) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(device, {});
    wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {});

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = utils::MakeBasicPipelineLayout(device, &bgl);
    pipelineDesc.computeStage.entryPoint = "main";
    pipelineDesc.computeStage.module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[stage(compute)]] fn main() -> void {
        })");

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bg);
    pass.Dispatch(1);
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

// Test creating a BGL with a storage buffer binding but declared readonly in the shader works.
// This is a regression test for crbug.com/dawn/410 which tests that it can successfully compile and
// execute the shader.
TEST_P(BindGroupTests, ReadonlyStorage) {
    utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);

    pipelineDescriptor.vertexStage.module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[builtin(vertex_idx)]] var<in> VertexIndex : u32;
        [[builtin(position)]] var<out> Position : vec4<f32>;

        [[stage(vertex)]] fn main() -> void {
            const pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
                vec2<f32>(-1.0, 1.0),
                vec2<f32>( 1.0, 1.0),
                vec2<f32>(-1.0, -1.0));

            Position = vec4<f32>(pos[VertexIndex], 0.0, 1.0);
        })");

    pipelineDescriptor.cFragmentStage.module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct Buffer0 {
            [[offset(0)]] color : vec4<f32>;
        };
        [[set(0), binding(0)]] var<storage_buffer> buffer0 : [[access(read)]] Buffer0;

        [[location(0)]] var<out> fragColor : vec4<f32>;
        [[stage(fragment)]] fn main() -> void {
            fragColor = buffer0.color;
        })");

    constexpr uint32_t kRTSize = 4;
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    pipelineDescriptor.cColorStates[0].format = renderPass.colorFormat;

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});

    pipelineDescriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

    std::array<float, 4> greenColor = {0, 1, 0, 1};
    wgpu::Buffer storageBuffer = utils::CreateBufferFromData(
        device, &greenColor, sizeof(greenColor), wgpu::BufferUsage::Storage);

    pass.SetPipeline(renderPipeline);
    pass.SetBindGroup(0, utils::MakeBindGroup(device, bgl, {{0, storageBuffer}}));
    pass.Draw(3);
    pass.EndPass();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, renderPass.color, 0, 0);
}

// Test that creating a large bind group, with each binding type at the max count, works and can be
// used correctly. The test loads a different value from each binding, and writes 1 to a storage
// buffer if all values are correct.
TEST_P(BindGroupTests, ReallyLargeBindGroup) {
    DAWN_SKIP_TEST_IF(IsOpenGLES());
    std::ostringstream interface;
    std::ostringstream body;
    uint32_t binding = 0;
    uint32_t expectedValue = 42;

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

    auto CreateTextureWithRedData = [&](wgpu::TextureFormat format, uint32_t value,
                                        wgpu::TextureUsage usage) {
        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.usage = wgpu::TextureUsage::CopyDst | usage;
        textureDesc.size = {1, 1, 1};
        textureDesc.format = format;
        wgpu::Texture texture = device.CreateTexture(&textureDesc);

        if (format == wgpu::TextureFormat::R8Unorm) {
            ASSERT(expectedValue < 255u);
        }
        wgpu::Buffer textureData =
            utils::CreateBufferFromData(device, wgpu::BufferUsage::CopySrc, {value});

        wgpu::BufferCopyView bufferCopyView = {};
        bufferCopyView.buffer = textureData;
        bufferCopyView.layout.bytesPerRow = 256;

        wgpu::TextureCopyView textureCopyView = {};
        textureCopyView.texture = texture;

        wgpu::Extent3D copySize = {1, 1, 1};

        commandEncoder.CopyBufferToTexture(&bufferCopyView, &textureCopyView, &copySize);
        return texture;
    };

    std::vector<wgpu::BindGroupEntry> bgEntries;
    static_assert(kMaxSampledTexturesPerShaderStage == kMaxSamplersPerShaderStage,
                  "Please update this test");
    for (uint32_t i = 0; i < kMaxSampledTexturesPerShaderStage; ++i) {
        wgpu::Texture texture = CreateTextureWithRedData(
            wgpu::TextureFormat::R8Unorm, expectedValue, wgpu::TextureUsage::Sampled);
        bgEntries.push_back({binding, nullptr, 0, 0, nullptr, texture.CreateView()});

        interface << "[[set(0), binding(" << binding++ << ")]] "
                  << "var<uniform_constant> tex" << i << " : texture_2d<f32>;\n";

        wgpu::SamplerDescriptor samplerDesc = {};
        bgEntries.push_back({binding, nullptr, 0, 0, device.CreateSampler(&samplerDesc), nullptr});

        interface << "[[set(0), binding(" << binding++ << ")]]"
                  << "var<uniform_constant> samp" << i << " : sampler;\n";

        body << "if (abs(textureSampleLevel(tex" << i << ", samp" << i
             << ", vec2<f32>(0.5, 0.5), 0.0).r - " << expectedValue++
             << ".0 / 255.0) > 0.0001) {\n";
        body << "    return;\n";
        body << "}\n";
    }
    for (uint32_t i = 0; i < kMaxStorageTexturesPerShaderStage; ++i) {
        wgpu::Texture texture = CreateTextureWithRedData(
            wgpu::TextureFormat::R32Uint, expectedValue, wgpu::TextureUsage::Storage);
        bgEntries.push_back({binding, nullptr, 0, 0, nullptr, texture.CreateView()});

        interface << "[[set(0), binding(" << binding++ << ")]] "
                  << "var<uniform_constant> image" << i << " : texture_storage_ro_2d<r32uint>;\n";

        body << "if (textureLoad(image" << i << ", vec2<i32>(0, 0)).r != " << expectedValue++
             << "u) {\n";
        body << "    return;\n";
        body << "}\n";
    }

    for (uint32_t i = 0; i < kMaxUniformBuffersPerShaderStage; ++i) {
        wgpu::Buffer buffer = utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Uniform, {expectedValue, 0, 0, 0});
        bgEntries.push_back({binding, buffer, 0, 4 * sizeof(uint32_t), nullptr, nullptr});

        interface << "[[block]] struct UniformBuffer" << i << R"({
                [[offset(0)]] value : u32;
            };
        )";
        interface << "[[set(0), binding(" << binding++ << ")]] "
                  << "var<uniform> ubuf" << i << " : UniformBuffer" << i << ";\n";

        body << "if (ubuf" << i << ".value != " << expectedValue++ << "u) {\n";
        body << "    return;\n";
        body << "}\n";
    }
    // Save one storage buffer for writing the result
    for (uint32_t i = 0; i < kMaxStorageBuffersPerShaderStage - 1; ++i) {
        wgpu::Buffer buffer = utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Storage, {expectedValue});
        bgEntries.push_back({binding, buffer, 0, sizeof(uint32_t), nullptr, nullptr});

        interface << "[[block]] struct ReadOnlyStorageBuffer" << i << R"({
                [[offset(0)]] value : u32;
            };
        )";
        interface << "[[set(0), binding(" << binding++ << ")]] "
                  << "var<storage_buffer> sbuf" << i << " : [[access(read)]] ReadOnlyStorageBuffer"
                  << i << ";\n";

        body << "if (sbuf" << i << ".value != " << expectedValue++ << "u) {\n";
        body << "    return;\n";
        body << "}\n";
    }

    wgpu::Buffer result = utils::CreateBufferFromData<uint32_t>(
        device, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc, {0});
    bgEntries.push_back({binding, result, 0, sizeof(uint32_t), nullptr, nullptr});

    interface << R"([[block]] struct ReadWriteStorageBuffer{
            [[offset(0)]] value : u32;
        };
    )";
    interface << "[[set(0), binding(" << binding++ << ")]] "
              << "var<storage_buffer> result : [[access(read_write)]] ReadWriteStorageBuffer;\n";

    body << "result.value = 1u;\n";

    std::string shader =
        interface.str() + "[[stage(compute)]] fn main() -> void {\n" + body.str() + "}\n";
    wgpu::ComputePipelineDescriptor cpDesc;
    cpDesc.computeStage.module = utils::CreateShaderModuleFromWGSL(device, shader.c_str());
    cpDesc.computeStage.entryPoint = "main";
    wgpu::ComputePipeline cp = device.CreateComputePipeline(&cpDesc);

    wgpu::BindGroupDescriptor bgDesc = {};
    bgDesc.layout = cp.GetBindGroupLayout(0);
    bgDesc.entryCount = static_cast<uint32_t>(bgEntries.size());
    bgDesc.entries = bgEntries.data();

    wgpu::BindGroup bg = device.CreateBindGroup(&bgDesc);

    wgpu::ComputePassEncoder pass = commandEncoder.BeginComputePass();
    pass.SetPipeline(cp);
    pass.SetBindGroup(0, bg);
    pass.Dispatch(1, 1, 1);
    pass.EndPass();

    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(1, result, 0);
}

DAWN_INSTANTIATE_TEST(BindGroupTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
