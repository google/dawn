// Copyright 2019 The Dawn Authors
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

#include "tests/DawnTest.h"

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/DawnHelpers.h"

class ObjectCachingTest : public DawnTest {};

// Test that BindGroupLayouts are correctly deduplicated.
TEST_P(ObjectCachingTest, BindGroupLayoutDeduplication) {
    dawn::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{1, dawn::ShaderStageBit::Fragment, dawn::BindingType::UniformBuffer}});
    dawn::BindGroupLayout sameBgl = utils::MakeBindGroupLayout(
        device, {{1, dawn::ShaderStageBit::Fragment, dawn::BindingType::UniformBuffer}});
    dawn::BindGroupLayout otherBgl = utils::MakeBindGroupLayout(
        device, {{1, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer}});

    EXPECT_NE(bgl.Get(), otherBgl.Get());
    EXPECT_EQ(bgl.Get() == sameBgl.Get(), !UsesWire());
}

// Test that an error object doesn't try to uncache itself
TEST_P(ObjectCachingTest, ErrorObjectDoesntUncache) {
    ASSERT_DEVICE_ERROR(
        dawn::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, dawn::ShaderStageBit::Fragment, dawn::BindingType::UniformBuffer},
                     {0, dawn::ShaderStageBit::Fragment, dawn::BindingType::UniformBuffer}}));
}

// Test that PipelineLayouts are correctly deduplicated.
TEST_P(ObjectCachingTest, PipelineLayoutDeduplication) {
    dawn::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{1, dawn::ShaderStageBit::Fragment, dawn::BindingType::UniformBuffer}});
    dawn::BindGroupLayout otherBgl = utils::MakeBindGroupLayout(
        device, {{1, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer}});

    dawn::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);
    dawn::PipelineLayout samePl = utils::MakeBasicPipelineLayout(device, &bgl);
    dawn::PipelineLayout otherPl1 = utils::MakeBasicPipelineLayout(device, nullptr);
    dawn::PipelineLayout otherPl2 = utils::MakeBasicPipelineLayout(device, &otherBgl);

    EXPECT_NE(pl.Get(), otherPl1.Get());
    EXPECT_NE(pl.Get(), otherPl2.Get());
    EXPECT_EQ(pl.Get() == samePl.Get(), !UsesWire());
}

// Test that ShaderModules are correctly deduplicated.
TEST_P(ObjectCachingTest, ShaderModuleDeduplication) {
    dawn::ShaderModule module = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
            #version 450
            layout(location = 0) out vec4 fragColor;
            void main() {
                fragColor = vec4(0.0, 1.0, 0.0, 1.0);
            })");
    dawn::ShaderModule sameModule =
        utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
            #version 450
            layout(location = 0) out vec4 fragColor;
            void main() {
                fragColor = vec4(0.0, 1.0, 0.0, 1.0);
            })");
    dawn::ShaderModule otherModule =
        utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
            #version 450
            layout(location = 0) out vec4 fragColor;
            void main() {
                fragColor = vec4(0.0);
            })");

    EXPECT_NE(module.Get(), otherModule.Get());
    EXPECT_EQ(module.Get() == sameModule.Get(), !UsesWire());
}

// Test that ComputePipeline are correctly deduplicated wrt. their ShaderModule
TEST_P(ObjectCachingTest, ComputePipelineDeduplicationOnShaderModule) {
    dawn::ShaderModule module = utils::CreateShaderModule(device, dawn::ShaderStage::Compute, R"(
            #version 450
            void main() {
                int i = 0;
            })");
    dawn::ShaderModule sameModule =
        utils::CreateShaderModule(device, dawn::ShaderStage::Compute, R"(
            #version 450
            void main() {
                int i = 0;
            })");
    dawn::ShaderModule otherModule =
        utils::CreateShaderModule(device, dawn::ShaderStage::Compute, R"(
            #version 450
            void main() {
            })");

    EXPECT_NE(module.Get(), otherModule.Get());
    EXPECT_EQ(module.Get() == sameModule.Get(), !UsesWire());

    dawn::PipelineLayout layout = utils::MakeBasicPipelineLayout(device, nullptr);

    dawn::PipelineStageDescriptor stageDesc;
    stageDesc.entryPoint = "main";
    stageDesc.module = module;

    dawn::ComputePipelineDescriptor desc;
    desc.computeStage = &stageDesc;
    desc.layout = layout;

    dawn::ComputePipeline pipeline = device.CreateComputePipeline(&desc);

    stageDesc.module = sameModule;
    dawn::ComputePipeline samePipeline = device.CreateComputePipeline(&desc);

    stageDesc.module = otherModule;
    dawn::ComputePipeline otherPipeline = device.CreateComputePipeline(&desc);

    EXPECT_NE(pipeline.Get(), otherPipeline.Get());
    EXPECT_EQ(pipeline.Get() == samePipeline.Get(), !UsesWire());
}

// Test that ComputePipeline are correctly deduplicated wrt. their layout
TEST_P(ObjectCachingTest, ComputePipelineDeduplicationOnLayout) {
    dawn::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{1, dawn::ShaderStageBit::Fragment, dawn::BindingType::UniformBuffer}});
    dawn::BindGroupLayout otherBgl = utils::MakeBindGroupLayout(
        device, {{1, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer}});

    dawn::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);
    dawn::PipelineLayout samePl = utils::MakeBasicPipelineLayout(device, &bgl);
    dawn::PipelineLayout otherPl = utils::MakeBasicPipelineLayout(device, nullptr);

    EXPECT_NE(pl.Get(), otherPl.Get());
    EXPECT_EQ(pl.Get() == samePl.Get(), !UsesWire());

    dawn::PipelineStageDescriptor stageDesc;
    stageDesc.entryPoint = "main";
    stageDesc.module = utils::CreateShaderModule(device, dawn::ShaderStage::Compute, R"(
            #version 450
            void main() {
                int i = 0;
            })");

    dawn::ComputePipelineDescriptor desc;
    desc.computeStage = &stageDesc;

    desc.layout = pl;
    dawn::ComputePipeline pipeline = device.CreateComputePipeline(&desc);

    desc.layout = samePl;
    dawn::ComputePipeline samePipeline = device.CreateComputePipeline(&desc);

    desc.layout = otherPl;
    dawn::ComputePipeline otherPipeline = device.CreateComputePipeline(&desc);

    EXPECT_NE(pipeline.Get(), otherPipeline.Get());
    EXPECT_EQ(pipeline.Get() == samePipeline.Get(), !UsesWire());
}

// Test that RenderPipelines are correctly deduplicated wrt. their layout
TEST_P(ObjectCachingTest, RenderPipelineDeduplicationOnLayout) {
    dawn::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{1, dawn::ShaderStageBit::Fragment, dawn::BindingType::UniformBuffer}});
    dawn::BindGroupLayout otherBgl = utils::MakeBindGroupLayout(
        device, {{1, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer}});

    dawn::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);
    dawn::PipelineLayout samePl = utils::MakeBasicPipelineLayout(device, &bgl);
    dawn::PipelineLayout otherPl = utils::MakeBasicPipelineLayout(device, nullptr);

    EXPECT_NE(pl.Get(), otherPl.Get());
    EXPECT_EQ(pl.Get() == samePl.Get(), !UsesWire());

    utils::ComboRenderPipelineDescriptor desc(device);
    desc.cVertexStage.module = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
            #version 450
            void main() {
                gl_Position = vec4(0.0);
            })");
    desc.cFragmentStage.module = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
            #version 450
            void main() {
            })");

    desc.layout = pl;
    dawn::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    desc.layout = samePl;
    dawn::RenderPipeline samePipeline = device.CreateRenderPipeline(&desc);

    desc.layout = otherPl;
    dawn::RenderPipeline otherPipeline = device.CreateRenderPipeline(&desc);

    EXPECT_NE(pipeline.Get(), otherPipeline.Get());
    EXPECT_EQ(pipeline.Get() == samePipeline.Get(), !UsesWire());
}

// Test that RenderPipelines are correctly deduplicated wrt. their vertex module
TEST_P(ObjectCachingTest, RenderPipelineDeduplicationOnVertexModule) {
    dawn::ShaderModule module = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
            #version 450
            void main() {
                gl_Position = vec4(0.0);
            })");
    dawn::ShaderModule sameModule = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
            #version 450
            void main() {
                gl_Position = vec4(0.0);
            })");
    dawn::ShaderModule otherModule =
        utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
            #version 450
            void main() {
                gl_Position = vec4(1.0);
            })");

    EXPECT_NE(module.Get(), otherModule.Get());
    EXPECT_EQ(module.Get() == sameModule.Get(), !UsesWire());

    utils::ComboRenderPipelineDescriptor desc(device);
    desc.cFragmentStage.module = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
            #version 450
            void main() {
            })");

    desc.cVertexStage.module = module;
    dawn::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    desc.cVertexStage.module = sameModule;
    dawn::RenderPipeline samePipeline = device.CreateRenderPipeline(&desc);

    desc.cVertexStage.module = otherModule;
    dawn::RenderPipeline otherPipeline = device.CreateRenderPipeline(&desc);

    EXPECT_NE(pipeline.Get(), otherPipeline.Get());
    EXPECT_EQ(pipeline.Get() == samePipeline.Get(), !UsesWire());
}

// Test that RenderPipelines are correctly deduplicated wrt. their fragment module
TEST_P(ObjectCachingTest, RenderPipelineDeduplicationOnFragmentModule) {
    dawn::ShaderModule module = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
            #version 450
            void main() {
            })");
    dawn::ShaderModule sameModule =
        utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
            #version 450
            void main() {
            })");
    dawn::ShaderModule otherModule =
        utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
            #version 450
            void main() {
                int i = 0;
            })");

    EXPECT_NE(module.Get(), otherModule.Get());
    EXPECT_EQ(module.Get() == sameModule.Get(), !UsesWire());

    utils::ComboRenderPipelineDescriptor desc(device);
    desc.cVertexStage.module = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
            #version 450
            void main() {
                gl_Position = vec4(0.0);
            })");

    desc.cFragmentStage.module = module;
    dawn::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    desc.cFragmentStage.module = sameModule;
    dawn::RenderPipeline samePipeline = device.CreateRenderPipeline(&desc);

    desc.cFragmentStage.module = otherModule;
    dawn::RenderPipeline otherPipeline = device.CreateRenderPipeline(&desc);

    EXPECT_NE(pipeline.Get(), otherPipeline.Get());
    EXPECT_EQ(pipeline.Get() == samePipeline.Get(), !UsesWire());
}

DAWN_INSTANTIATE_TEST(ObjectCachingTest, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend);
