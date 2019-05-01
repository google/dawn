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

DAWN_INSTANTIATE_TEST(ObjectCachingTest, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend);
