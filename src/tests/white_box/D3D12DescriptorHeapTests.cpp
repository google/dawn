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

// Pooling tests are required to advance the GPU completed serial to reuse heaps.
// This requires Tick() to be called at-least |kFrameDepth| times. This constant
// should be updated if the internals of Tick() change.
constexpr uint32_t kFrameDepth = 2;

using namespace dawn_native::d3d12;

class D3D12DescriptorHeapTests : public DawnTest {
  protected:
    void TestSetUp() override {
        DAWN_SKIP_TEST_IF(UsesWire());
        mD3DDevice = reinterpret_cast<Device*>(device.Get());
    }

    Device* mD3DDevice = nullptr;
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

// Verify shader-visible heaps can be recycled for multiple submits.
TEST_P(D3D12DescriptorHeapTests, PoolHeapsInMultipleSubmits) {
    constexpr D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

    ShaderVisibleDescriptorAllocator* allocator = mD3DDevice->GetShaderVisibleDescriptorAllocator();

    std::list<ComPtr<ID3D12DescriptorHeap>> heaps = {
        allocator->GetShaderVisibleHeapForTesting(heapType)};

    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(heapType), 0u);

    // Allocate + Tick() up to |kFrameDepth| and ensure heaps are always unique.
    for (uint32_t i = 0; i < kFrameDepth; i++) {
        EXPECT_TRUE(allocator->AllocateAndSwitchShaderVisibleHeaps().IsSuccess());
        ComPtr<ID3D12DescriptorHeap> heap = allocator->GetShaderVisibleHeapForTesting(heapType);
        EXPECT_TRUE(std::find(heaps.begin(), heaps.end(), heap) == heaps.end());
        heaps.push_back(heap);
        mD3DDevice->Tick();
    }

    // Repeat up to |kFrameDepth| again but ensure heaps are the same in the expected order
    // (oldest heaps are recycled first). The "+ 1" is so we also include the very first heap in the
    // check.
    for (uint32_t i = 0; i < kFrameDepth + 1; i++) {
        EXPECT_TRUE(allocator->AllocateAndSwitchShaderVisibleHeaps().IsSuccess());
        ComPtr<ID3D12DescriptorHeap> heap = allocator->GetShaderVisibleHeapForTesting(heapType);
        EXPECT_TRUE(heaps.front() == heap);
        heaps.pop_front();
        mD3DDevice->Tick();
    }

    EXPECT_TRUE(heaps.empty());
    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(heapType), kFrameDepth);
}

// Verify shader-visible heaps do not recycle in a pending submit.
TEST_P(D3D12DescriptorHeapTests, PoolHeapsInPendingSubmit) {
    constexpr D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    constexpr uint32_t kNumOfSwitches = 5;

    ShaderVisibleDescriptorAllocator* allocator = mD3DDevice->GetShaderVisibleDescriptorAllocator();

    const Serial heapSerial = allocator->GetShaderVisibleHeapsSerial();

    std::set<ComPtr<ID3D12DescriptorHeap>> heaps = {
        allocator->GetShaderVisibleHeapForTesting(heapType)};

    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(heapType), 0u);

    // Switch-over |kNumOfSwitches| and ensure heaps are always unique.
    for (uint32_t i = 0; i < kNumOfSwitches; i++) {
        EXPECT_TRUE(allocator->AllocateAndSwitchShaderVisibleHeaps().IsSuccess());
        ComPtr<ID3D12DescriptorHeap> heap = allocator->GetShaderVisibleHeapForTesting(heapType);
        EXPECT_TRUE(std::find(heaps.begin(), heaps.end(), heap) == heaps.end());
        heaps.insert(heap);
    }

    // After |kNumOfSwitches|, no heaps are recycled.
    EXPECT_EQ(allocator->GetShaderVisibleHeapsSerial(), heapSerial + kNumOfSwitches);
    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(heapType), kNumOfSwitches);
}

// Verify switching shader-visible heaps do not recycle in a pending submit but do so
// once no longer pending.
TEST_P(D3D12DescriptorHeapTests, PoolHeapsInPendingAndMultipleSubmits) {
    constexpr D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    constexpr uint32_t kNumOfSwitches = 5;

    ShaderVisibleDescriptorAllocator* allocator = mD3DDevice->GetShaderVisibleDescriptorAllocator();
    const Serial heapSerial = allocator->GetShaderVisibleHeapsSerial();

    std::set<ComPtr<ID3D12DescriptorHeap>> heaps = {
        allocator->GetShaderVisibleHeapForTesting(heapType)};

    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(heapType), 0u);

    // Switch-over |kNumOfSwitches| to create a pool of unique heaps.
    for (uint32_t i = 0; i < kNumOfSwitches; i++) {
        EXPECT_TRUE(allocator->AllocateAndSwitchShaderVisibleHeaps().IsSuccess());
        ComPtr<ID3D12DescriptorHeap> heap = allocator->GetShaderVisibleHeapForTesting(heapType);
        EXPECT_TRUE(std::find(heaps.begin(), heaps.end(), heap) == heaps.end());
        heaps.insert(heap);
    }

    EXPECT_EQ(allocator->GetShaderVisibleHeapsSerial(), heapSerial + kNumOfSwitches);
    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(heapType), kNumOfSwitches);

    // Ensure switched-over heaps can be recycled by advancing the GPU by at-least |kFrameDepth|.
    for (uint32_t i = 0; i < kFrameDepth; i++) {
        mD3DDevice->Tick();
    }

    // Switch-over |kNumOfSwitches| again reusing the same heaps.
    for (uint32_t i = 0; i < kNumOfSwitches; i++) {
        EXPECT_TRUE(allocator->AllocateAndSwitchShaderVisibleHeaps().IsSuccess());
        ComPtr<ID3D12DescriptorHeap> heap = allocator->GetShaderVisibleHeapForTesting(heapType);
        EXPECT_TRUE(std::find(heaps.begin(), heaps.end(), heap) != heaps.end());
        heaps.erase(heap);
    }

    // After switching-over |kNumOfSwitches| x 2, ensure no additional heaps exist.
    EXPECT_EQ(allocator->GetShaderVisibleHeapsSerial(), heapSerial + kNumOfSwitches * 2);
    EXPECT_EQ(allocator->GetShaderVisiblePoolSizeForTesting(heapType), kNumOfSwitches);
}

DAWN_INSTANTIATE_TEST(D3D12DescriptorHeapTests, D3D12Backend());
