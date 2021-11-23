// Copyright 2021 The Dawn Authors
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

#include <string>
#include "tests/unittests/validation/ValidationTest.h"
#include "utils/ComboRenderBundleEncoderDescriptor.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class LabelTest : public ValidationTest {};

TEST_F(LabelTest, BindGroup) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {});

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.entryCount = 0;
    descriptor.entries = nullptr;

    // The label should be empty if one was not set.
    {
        wgpu::BindGroup bindGroup = device.CreateBindGroup(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(bindGroup.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::BindGroup bindGroup = device.CreateBindGroup(&descriptor);
        bindGroup.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(bindGroup.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::BindGroup bindGroup = device.CreateBindGroup(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(bindGroup.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, BindGroupLayout) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";

    wgpu::BindGroupLayoutDescriptor descriptor = {};
    descriptor.entryCount = 0;
    descriptor.entries = nullptr;

    // The label should be empty if one was not set.
    {
        wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(bindGroupLayout.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&descriptor);
        bindGroupLayout.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(bindGroupLayout.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(bindGroupLayout.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, Buffer) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::Uniform;

    // The label should be empty if one was not set.
    {
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(buffer.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        buffer.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(buffer.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(buffer.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, CommandBuffer) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::CommandBufferDescriptor descriptor;

    // The label should be empty if one was not set.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::CommandBuffer commandBuffer = encoder.Finish(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(commandBuffer.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::CommandBuffer commandBuffer = encoder.Finish(&descriptor);
        commandBuffer.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(commandBuffer.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::CommandBuffer commandBuffer = encoder.Finish(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(commandBuffer.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, CommandEncoder) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::CommandEncoderDescriptor descriptor;

    // The label should be empty if one was not set.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&descriptor);
        encoder.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, ComputePassEncoder) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

    wgpu::ComputePassDescriptor descriptor;

    // The label should be empty if one was not set.
    {
        wgpu::ComputePassEncoder encoder = commandEncoder.BeginComputePass(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_TRUE(readbackLabel.empty());
        encoder.EndPass();
    }

    // Test setting a label through API
    {
        wgpu::ComputePassEncoder encoder = commandEncoder.BeginComputePass(&descriptor);
        encoder.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
        encoder.EndPass();
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::ComputePassEncoder encoder = commandEncoder.BeginComputePass(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
        encoder.EndPass();
    }
}

TEST_F(LabelTest, ExternalTexture) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size.width = 1;
    textureDescriptor.size.height = 1;
    textureDescriptor.size.depthOrArrayLayers = 1;
    textureDescriptor.mipLevelCount = 1;
    textureDescriptor.sampleCount = 1;
    textureDescriptor.dimension = wgpu::TextureDimension::e2D;
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage =
        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    wgpu::ExternalTextureDescriptor descriptor;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.plane0 = texture.CreateView();

    // The label should be empty if one was not set.
    {
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(externalTexture.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&descriptor);
        externalTexture.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(externalTexture.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(externalTexture.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, PipelineLayout) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(device, {});

    wgpu::PipelineLayoutDescriptor descriptor;
    descriptor.bindGroupLayoutCount = 1;
    descriptor.bindGroupLayouts = &layout;

    // The label should be empty if one was not set.
    {
        wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(pipelineLayout.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&descriptor);
        pipelineLayout.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(pipelineLayout.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(pipelineLayout.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, QuerySet) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::QuerySetDescriptor descriptor;
    descriptor.type = wgpu::QueryType::Occlusion;
    descriptor.count = 1;

    // The label should be empty if one was not set.
    {
        wgpu::QuerySet querySet = device.CreateQuerySet(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(querySet.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::QuerySet querySet = device.CreateQuerySet(&descriptor);
        querySet.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(querySet.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::QuerySet querySet = device.CreateQuerySet(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(querySet.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, RenderBundleEncoder) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";

    utils::ComboRenderBundleEncoderDescriptor descriptor = {};
    descriptor.colorFormatsCount = 1;
    descriptor.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;

    // The label should be empty if one was not set.
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&descriptor);
        encoder.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, RenderPassEncoder) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {1, 1, 1};
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    utils::ComboRenderPassDescriptor descriptor({texture.CreateView()});

    // The label should be empty if one was not set.
    {
        wgpu::RenderPassEncoder encoder = commandEncoder.BeginRenderPass(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_TRUE(readbackLabel.empty());
        encoder.EndPass();
    }

    // Test setting a label through API
    {
        wgpu::RenderPassEncoder encoder = commandEncoder.BeginRenderPass(&descriptor);
        encoder.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
        encoder.EndPass();
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::RenderPassEncoder encoder = commandEncoder.BeginRenderPass(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(encoder.Get());
        ASSERT_EQ(label, readbackLabel);
        encoder.EndPass();
    }
}

TEST_F(LabelTest, Sampler) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::SamplerDescriptor descriptor;

    // The label should be empty if one was not set.
    {
        wgpu::Sampler sampler = device.CreateSampler(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(sampler.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::Sampler sampler = device.CreateSampler(&descriptor);
        sampler.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(sampler.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::Sampler sampler = device.CreateSampler(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(sampler.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, Texture) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::TextureDescriptor descriptor;
    descriptor.size.width = 1;
    descriptor.size.height = 1;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.mipLevelCount = 1;
    descriptor.sampleCount = 1;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.format = wgpu::TextureFormat::RGBA8Uint;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;

    // The label should be empty if one was not set.
    {
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(texture.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        texture.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(texture.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(texture.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, TextureView) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::TextureDescriptor descriptor;
    descriptor.size.width = 1;
    descriptor.size.height = 1;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.mipLevelCount = 1;
    descriptor.sampleCount = 1;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.format = wgpu::TextureFormat::RGBA8Uint;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;

    wgpu::Texture texture = device.CreateTexture(&descriptor);

    // The label should be empty if one was not set.
    {
        wgpu::TextureView textureView = texture.CreateView();
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(textureView.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::TextureView textureView = texture.CreateView();
        textureView.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(textureView.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        wgpu::TextureViewDescriptor viewDescriptor;
        viewDescriptor.label = label.c_str();
        wgpu::TextureView textureView = texture.CreateView(&viewDescriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(textureView.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, RenderPipeline) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
                return vec4<f32>(0.0, 0.0, 0.0, 1.0);
            })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            [[stage(fragment)]] fn main() -> [[location(0)]] vec4<f32> {
                return vec4<f32>(0.0, 1.0, 0.0, 1.0);
            })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;

    // The label should be empty if one was not set.
    {
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(pipeline.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        pipeline.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(pipeline.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(pipeline.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, ComputePipeline) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";

    wgpu::ShaderModule computeModule = utils::CreateShaderModule(device, R"(
    [[stage(compute), workgroup_size(1)]] fn main() {
    })");
    wgpu::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, nullptr);
    wgpu::ComputePipelineDescriptor descriptor;
    descriptor.layout = pl;
    descriptor.compute.module = computeModule;
    descriptor.compute.entryPoint = "main";

    // The label should be empty if one was not set.
    {
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(pipeline.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&descriptor);
        pipeline.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(pipeline.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(pipeline.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, ShaderModule) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";

    const char* source = R"(
    [[stage(compute), workgroup_size(1)]] fn main() {
    })";

    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.source = source;
    wgpu::ShaderModuleDescriptor descriptor;
    descriptor.nextInChain = &wgslDesc;

    // The label should be empty if one was not set.
    {
        wgpu::ShaderModule shaderModule = device.CreateShaderModule(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(shaderModule.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::ShaderModule shaderModule = device.CreateShaderModule(&descriptor);
        shaderModule.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(shaderModule.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::ShaderModule shaderModule = device.CreateShaderModule(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(shaderModule.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}