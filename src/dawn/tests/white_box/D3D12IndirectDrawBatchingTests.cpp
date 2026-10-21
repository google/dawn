// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <vector>

#include "src/dawn/native/d3d12/CommandBufferD3D12.h"
#include "src/dawn/native/d3d12/DeviceD3D12.h"
#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/utils/ComboRenderPipelineDescriptor.h"
#include "src/dawn/utils/WGPUHelpers.h"

namespace dawn::native::d3d12 {
namespace {

class D3D12IndirectDrawBatchingTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        mRenderPass = utils::CreateBasicRenderPass(device, 4, 4);
        auto shader = utils::CreateShaderModule(device, R"(
            @vertex fn vs(@location(0) pos: vec4f) -> @builtin(position) vec4f {
                return pos;
            }
            @fragment fn fs() -> @location(0) vec4f { return vec4f(0, 1, 0, 1); }
        )");
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = shader;
        desc.vertex.entryPoint = "vs";
        desc.cFragment.module = shader;
        desc.cFragment.entryPoint = "fs";
        desc.vertex.bufferCount = 1;
        desc.cBuffers[0].arrayStride = 4 * sizeof(float);
        desc.cBuffers[0].attributeCount = 1;
        desc.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
        desc.cTargets[0].format = mRenderPass.colorFormat;
        mPipeline = device.CreateRenderPipeline(&desc);
        mVertices = utils::CreateBufferFromData<float>(device, wgpu::BufferUsage::Vertex,
                                                       {-1, -1, 0, 1, 1, -1, 0, 1, 0, 1, 0, 1});
    }

    void Check(const wgpu::Buffer& first,
               uint64_t firstOffset,
               const wgpu::Buffer& second,
               uint64_t secondOffset,
               const std::vector<uint32_t>& expected,
               bool firstIndexed,
               bool changePipeline,
               bool secondIndexed) {
        auto encoder = device.CreateCommandEncoder();
        auto pass = encoder.BeginRenderPass(&mRenderPass.renderPassInfo);
        pass.SetPipeline(mPipeline);
        pass.SetVertexBuffer(0, mVertices);
        auto indices =
            utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {0, 1, 2});
        pass.SetIndexBuffer(indices, wgpu::IndexFormat::Uint32);
        if (firstIndexed) {
            pass.DrawIndexedIndirect(first, firstOffset);
        } else {
            pass.DrawIndirect(first, firstOffset);
        }
        if (changePipeline) {
            pass.SetPipeline(mPipeline);
        }
        if (secondIndexed) {
            pass.DrawIndexedIndirect(second, secondOffset);
        } else {
            pass.DrawIndirect(second, secondOffset);
        }
        pass.End();
        auto commands = encoder.Finish();
        auto* backend = reinterpret_cast<CommandBuffer*>(commands.Get());
        backend->EnableIndirectDrawBatchRecordingForTesting();
        queue.Submit(1, &commands);
        WaitForAllOperations();
        // Vector length counts ExecuteIndirect calls; each entry counts draws in that call.
        EXPECT_EQ(backend->GetIndirectDrawBatchCountsForTesting(), expected);
    }

    utils::BasicRenderPass mRenderPass;
    wgpu::RenderPipeline mPipeline;
    wgpu::Buffer mVertices;
};

TEST_P(D3D12IndirectDrawBatchingTests, AdjacentRecords) {
    auto buffer = utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect,
                                                        {3, 1, 0, 0, 3, 1, 0, 0});
    Check(buffer, 0, buffer, 16, {2}, /*firstIndexed=*/false, /*changePipeline=*/false,
          /*secondIndexed=*/false);
}

TEST_P(D3D12IndirectDrawBatchingTests, NonAdjacentRecords) {
    auto buffer = utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect,
                                                        {3, 1, 0, 0, 0, 0, 0, 0, 3, 1, 0, 0});
    // Validation packs these records together; without it their offsets remain separate.
    const bool skipValidation = !reinterpret_cast<Device*>(device.Get())->IsValidationEnabled();
    Check(buffer, 0, buffer, 32,
          skipValidation ? std::vector<uint32_t>{1, 1} : std::vector<uint32_t>{2},
          /*firstIndexed=*/false, /*changePipeline=*/false, /*secondIndexed=*/false);
}

TEST_P(D3D12IndirectDrawBatchingTests, DifferentBuffers) {
    auto first =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, {3, 1, 0, 0});
    auto second =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, {3, 1, 0, 0});
    Check(first, 0, second, 0, {1, 1}, /*firstIndexed=*/false, /*changePipeline=*/false,
          /*secondIndexed=*/false);
}

TEST_P(D3D12IndirectDrawBatchingTests, PipelineCommandBreaksBatch) {
    auto buffer = utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect,
                                                        {3, 1, 0, 0, 3, 1, 0, 0});
    Check(buffer, 0, buffer, 16, {1, 1}, /*firstIndexed=*/false, /*changePipeline=*/true,
          /*secondIndexed=*/false);
}

TEST_P(D3D12IndirectDrawBatchingTests, AdjacentIndexedRecords) {
    auto buffer = utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect,
                                                        {3, 1, 0, 0, 0, 3, 1, 0, 0, 0});
    Check(buffer, 0, buffer, 20, {2}, /*firstIndexed=*/true, /*changePipeline=*/false,
          /*secondIndexed=*/true);
}

TEST_P(D3D12IndirectDrawBatchingTests, NonAdjacentIndexedRecords) {
    auto buffer = utils::CreateBufferFromData<uint32_t>(
        device, wgpu::BufferUsage::Indirect, {3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 0, 0, 0});
    // Validation packs these records together; without it their offsets remain separate.
    const bool skipValidation = !reinterpret_cast<Device*>(device.Get())->IsValidationEnabled();
    Check(buffer, 0, buffer, 40,
          skipValidation ? std::vector<uint32_t>{1, 1} : std::vector<uint32_t>{2},
          /*firstIndexed=*/true, /*changePipeline=*/false, /*secondIndexed=*/true);
}

TEST_P(D3D12IndirectDrawBatchingTests, DifferentIndexedBuffers) {
    auto first =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, {3, 1, 0, 0, 0});
    auto second =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect, {3, 1, 0, 0, 0});
    Check(first, 0, second, 0, {1, 1}, /*firstIndexed=*/true, /*changePipeline=*/false,
          /*secondIndexed=*/true);
}

TEST_P(D3D12IndirectDrawBatchingTests, PipelineCommandBreaksIndexedBatch) {
    auto buffer = utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect,
                                                        {3, 1, 0, 0, 0, 3, 1, 0, 0, 0});
    Check(buffer, 0, buffer, 20, {1, 1}, /*firstIndexed=*/true, /*changePipeline=*/true,
          /*secondIndexed=*/true);
}

TEST_P(D3D12IndirectDrawBatchingTests, IndexedThenNonIndexed) {
    // The non-indexed record is one non-indexed stride after the indexed record. Its
    // vertex count overlaps the firstInstance field, so both draws use zero instances.
    auto buffer = utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect,
                                                        {3, 0, 0, 0, 0, 0, 0, 0});
    Check(buffer, 0, buffer, 16, {1, 1}, /*firstIndexed=*/true, /*changePipeline=*/false,
          /*secondIndexed=*/false);
}

TEST_P(D3D12IndirectDrawBatchingTests, NonIndexedThenIndexed) {
    // The second record is one indexed stride after the first. With validation disabled,
    // only the command signature prevents the draws from being batched together.
    auto buffer = utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Indirect,
                                                        {3, 1, 0, 0, 0, 3, 1, 0, 0, 0});
    Check(buffer, 0, buffer, 20, {1, 1}, /*firstIndexed=*/false, /*changePipeline=*/false,
          /*secondIndexed=*/true);
}

DAWN_INSTANTIATE_TEST(D3D12IndirectDrawBatchingTests,
                      D3D12Backend(),
                      D3D12Backend({"skip_validation"}));

}  // namespace
}  // namespace dawn::native::d3d12
