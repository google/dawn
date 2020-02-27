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

#include "tests/DawnTest.h"

#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

constexpr uint32_t kRTSize = 4;

using namespace dawn_native::d3d12;

class D3D12DescriptorHeapTests : public DawnTest {
  private:
    void TestSetUp() override {
        DAWN_SKIP_TEST_IF(UsesWire());
    }
};

// Verify the shader visible heaps switch over within a single submit.
TEST_P(D3D12DescriptorHeapTests, SwitchOverHeaps) {
    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor(device);

    // Fill in a sampler heap with "sampler only" bindgroups (1x sampler per group) by creating a
    // sampler bindgroup each draw. After HEAP_SIZE + 1 draws, the heaps must switch over.
    renderPipelineDescriptor.vertexStage.module =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, R"(
            #version 450
            void main() {
                gl_Position = vec4(0.f, 0.f, 0.f, 1.f);
            })");

    renderPipelineDescriptor.cFragmentStage.module =
        utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, R"(#version 450
            layout(set = 0, binding = 0) uniform sampler sampler0;
            layout(location = 0) out vec4 fragColor;
            void main() {
               fragColor = vec4(0.0, 0.0, 0.0, 0.0);
            })");

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::SamplerDescriptor samplerDesc = utils::GetDefaultSamplerDescriptor();
    wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

    Device* d3dDevice = reinterpret_cast<Device*>(device.Get());
    ShaderVisibleDescriptorAllocator* allocator = d3dDevice->GetShaderVisibleDescriptorAllocator();
    const uint64_t samplerHeapSize =
        allocator->GetShaderVisibleHeapSizeForTesting(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    const Serial heapSerial = allocator->GetShaderVisibleHeapsSerial();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);

        pass.SetPipeline(renderPipeline);

        for (uint32_t i = 0; i < samplerHeapSize + 1; ++i) {
            pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                      {{0, sampler}}));
            pass.Draw(3, 1, 0, 0);
        }

        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_EQ(allocator->GetShaderVisibleHeapsSerial(), heapSerial + 1);
}

DAWN_INSTANTIATE_TEST(D3D12DescriptorHeapTests, D3D12Backend());
