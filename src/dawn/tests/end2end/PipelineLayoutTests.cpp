// Copyright 2020 The Dawn Authors
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

#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

class PipelineLayoutTests : public DawnTest {};

// Test creating a PipelineLayout with multiple BGLs where the first BGL uses the max number of
// dynamic buffers. This is a regression test for crbug.com/dawn/449 which would overflow when
// dynamic offset bindings were at max. Test is successful if the pipeline layout is created
// without error.
TEST_P(PipelineLayoutTests, DynamicBuffersOverflow) {
    // Create the first bind group layout which uses max number of dynamic buffers bindings.
    wgpu::BindGroupLayout bglA;
    {
        std::vector<wgpu::BindGroupLayoutEntry> entries;
        for (uint32_t i = 0;
             i < GetSupportedLimits().limits.maxDynamicStorageBuffersPerPipelineLayout; i++) {
            wgpu::BindGroupLayoutEntry entry;
            entry.binding = i;
            entry.visibility = wgpu::ShaderStage::Compute;
            entry.buffer.type = wgpu::BufferBindingType::Storage;
            entry.buffer.hasDynamicOffset = true;

            entries.push_back(entry);
        }

        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.entryCount = static_cast<uint32_t>(entries.size());
        descriptor.entries = entries.data();
        bglA = device.CreateBindGroupLayout(&descriptor);
    }

    // Create the second bind group layout that has one non-dynamic buffer binding.
    // It is in the fragment stage to avoid the max per-stage storage buffer limit.
    wgpu::BindGroupLayout bglB;
    {
        wgpu::BindGroupLayoutDescriptor descriptor;
        wgpu::BindGroupLayoutEntry entry;
        entry.binding = 0;
        entry.visibility = wgpu::ShaderStage::Fragment;
        entry.buffer.type = wgpu::BufferBindingType::Storage;

        descriptor.entryCount = 1;
        descriptor.entries = &entry;
        bglB = device.CreateBindGroupLayout(&descriptor);
    }

    // Create a pipeline layout using both bind group layouts.
    wgpu::PipelineLayoutDescriptor descriptor;
    std::vector<wgpu::BindGroupLayout> bindgroupLayouts = {bglA, bglB};
    descriptor.bindGroupLayoutCount = bindgroupLayouts.size();
    descriptor.bindGroupLayouts = bindgroupLayouts.data();
    device.CreatePipelineLayout(&descriptor);
}

// Regression test for crbug.com/dawn/1689. Test using a compute pass and a render pass,
// where the two pipelines have the same pipeline layout.
TEST_P(PipelineLayoutTests, ComputeAndRenderSamePipelineLayout) {
    wgpu::TextureFormat format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(8, 8)
        fn computeMain() {}

        @vertex fn vertexMain() -> @builtin(position) vec4f {
            return vec4f(0.0);
        }

        @fragment fn fragmentMain() -> @location(0) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
    )");

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform}});

    wgpu::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);
    wgpu::ComputePipeline computePipeline;
    {
        wgpu::ComputePipelineDescriptor desc = {};
        desc.layout = pl;
        desc.compute.module = shaderModule;
        desc.compute.entryPoint = "computeMain";
        computePipeline = device.CreateComputePipeline(&desc);
    }
    wgpu::RenderPipeline renderPipeline;
    {
        wgpu::RenderPipelineDescriptor desc = {};
        desc.layout = pl;
        desc.vertex.module = shaderModule;
        desc.vertex.entryPoint = "vertexMain";

        wgpu::FragmentState fragment = {};
        desc.fragment = &fragment;
        fragment.module = shaderModule;
        fragment.entryPoint = "fragmentMain";
        fragment.targetCount = 1;

        wgpu::ColorTargetState colorTargetState = {};
        colorTargetState.format = format;
        fragment.targets = &colorTargetState;

        renderPipeline = device.CreateRenderPipeline(&desc);
    }

    wgpu::Buffer buffer = utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {1});
    wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl, {{0, buffer}});
    wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl, {{0, buffer}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(computePipeline);
        pass.SetBindGroup(0, bg0);
        pass.DispatchWorkgroups(1);
        pass.End();
    }
    {
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 4, 4, format);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(0, bg1);
        pass.Draw(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

DAWN_INSTANTIATE_TEST(PipelineLayoutTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
