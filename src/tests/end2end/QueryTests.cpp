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

// This file contains test for deprecated parts of Dawn's API while following WebGPU's evolution.
// It contains test for the "old" behavior that will be deleted once users are migrated, tests that
// a deprecation warning is emitted when the "old" behavior is used, and tests that an error is
// emitted when both the old and the new behavior are used (when applicable).

#include "tests/DawnTest.h"

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class QueryTests : public DawnTest {
  protected:
    wgpu::Buffer CreateResolveBuffer(uint64_t size) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = wgpu::BufferUsage::QueryResolve | wgpu::BufferUsage::CopySrc |
                           wgpu::BufferUsage::CopyDst;
        return device.CreateBuffer(&descriptor);
    }
};

class OcclusionQueryTests : public QueryTests {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[builtin(vertex_idx)]] var<in> VertexIndex : u32;
            [[builtin(position)]] var<out> Position : vec4<f32>;
            [[stage(vertex)]] fn main() -> void {
                const pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
                    vec2<f32>( 1.0,  1.0),
                    vec2<f32>(-1.0, -1.0),
                    vec2<f32>( 1.0, -1.0));
                Position = vec4<f32>(pos[VertexIndex], 0.0, 1.0);
            })");

        fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[location(0)]] var<out> fragColor : vec4<f32>;
            [[stage(fragment)]] fn main() -> void {
                fragColor = vec4<f32>(0.0, 1.0, 0.0, 1.0);
            })");
    }

    struct ScissorRect {
        uint32_t x;
        uint32_t y;
        uint32_t width;
        uint32_t height;
    };

    wgpu::QuerySet CreateOcclusionQuerySet(uint32_t count) {
        wgpu::QuerySetDescriptor descriptor;
        descriptor.count = count;
        descriptor.type = wgpu::QueryType::Occlusion;
        return device.CreateQuerySet(&descriptor);
    }

    wgpu::Texture CreateRenderTexture(wgpu::TextureFormat format) {
        wgpu::TextureDescriptor descriptor;
        descriptor.size = {kRTSize, kRTSize, 1};
        descriptor.format = format;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment;
        return device.CreateTexture(&descriptor);
    }

    void TestOcclusionQueryWithDepthStencilTest(bool depthTestEnabled,
                                                bool stencilTestEnabled,
                                                uint64_t expected) {
        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;

        // Enable depth and stencil tests and set comparison tests never pass.
        wgpu::DepthStencilStateDescriptor depthStencilState;
        depthStencilState.depthCompare =
            depthTestEnabled ? wgpu::CompareFunction::Never : wgpu::CompareFunction::Always;
        depthStencilState.stencilFront.compare =
            stencilTestEnabled ? wgpu::CompareFunction::Never : wgpu::CompareFunction::Always;
        depthStencilState.stencilBack.compare =
            stencilTestEnabled ? wgpu::CompareFunction::Never : wgpu::CompareFunction::Always;

        descriptor.cDepthStencilState = depthStencilState;
        descriptor.cDepthStencilState.format = wgpu::TextureFormat::Depth24PlusStencil8;
        descriptor.depthStencilState = &descriptor.cDepthStencilState;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        wgpu::Texture renderTarget = CreateRenderTexture(wgpu::TextureFormat::RGBA8Unorm);
        wgpu::TextureView renderTargetView = renderTarget.CreateView();

        wgpu::Texture depthTexture = CreateRenderTexture(wgpu::TextureFormat::Depth24PlusStencil8);
        wgpu::TextureView depthTextureView = depthTexture.CreateView();

        wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
        uint64_t myData = ~uint64_t(1);
        // Set all bits in buffer to check 0 is correctly written if there is no sample passed the
        // occlusion testing
        queue.WriteBuffer(destination, 0, &myData, sizeof(myData));

        utils::ComboRenderPassDescriptor renderPass({renderTargetView}, depthTextureView);
        renderPass.occlusionQuerySet = querySet;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);
        pass.SetStencilReference(0);
        pass.BeginOcclusionQuery(0);
        pass.Draw(3);
        pass.EndOcclusionQuery();
        pass.EndPass();

        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U64_EQ(expected, destination, 0);
    }

    void TestOcclusionQueryWithScissorTest(ScissorRect rect, uint64_t expected) {
        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
        uint64_t myData = ~uint64_t(1);
        // Set all bits in buffer to check 0 is correctly written if there is no sample passed the
        // occlusion testing
        queue.WriteBuffer(destination, 0, &myData, sizeof(myData));

        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        renderPass.renderPassInfo.occlusionQuerySet = querySet;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetScissorRect(rect.x, rect.y, rect.width, rect.height);
        pass.BeginOcclusionQuery(0);
        pass.Draw(3);
        pass.EndOcclusionQuery();
        pass.EndPass();

        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U64_EQ(expected, destination, 0);
    }

    wgpu::ShaderModule vsModule;
    wgpu::ShaderModule fsModule;

    constexpr static unsigned int kRTSize = 4;
    constexpr static uint32_t kQueryCount = 1;
};

// Test creating query set with the type of Occlusion
TEST_P(OcclusionQueryTests, QuerySetCreation) {
    CreateOcclusionQuerySet(kQueryCount);
}

// Test destroying query set
TEST_P(OcclusionQueryTests, QuerySetDestroy) {
    wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
    querySet.Destroy();
}

// Draw a bottom right triangle with depth/stencil testing enabled and check whether there is
// sample passed the testing by non-precise occlusion query with the binary results:
// 0 indicates that no sample passed depth/stencil testing,
// 1 indicates that at least one sample passed depth/stencil testing.
TEST_P(OcclusionQueryTests, QueryWithDepthStencilTest) {
    // TODO(hao.x.li@intel.com): Implement non-precise occlusion on Metal and Vulkan
    DAWN_SKIP_TEST_IF(IsMetal() || IsVulkan());

    // Disable depth/stencil testing, the samples always pass the testing, the expected occlusion
    // result is 1.
    TestOcclusionQueryWithDepthStencilTest(false, false, 1);

    // Only enable depth testing and set the samples never pass the testing, the expected occlusion
    // result is 0.
    TestOcclusionQueryWithDepthStencilTest(true, false, 0);

    // Only enable stencil testing and set the samples never pass the testing, the expected
    // occlusion result is 0.
    TestOcclusionQueryWithDepthStencilTest(false, true, 0);
}

// Draw a bottom right triangle with scissor testing enabled and check whether there is
// sample passed the testing by non-precise occlusion query with the binary results:
// 0 indicates that no sample passed scissor testing,
// 1 indicates that at least one sample passed scissor testing.
TEST_P(OcclusionQueryTests, QueryWithScissorTest) {
    // TODO(hao.x.li@intel.com): Implement non-precise occlusion on Metal and Vulkan
    DAWN_SKIP_TEST_IF(IsMetal() || IsVulkan());

    // Test there are samples passed scissor testing, the expected occlusion result is 1.
    TestOcclusionQueryWithScissorTest({2, 1, 2, 1}, 1);

    // Test there is no sample passed scissor testing, the expected occlusion result is 0.
    TestOcclusionQueryWithScissorTest({0, 0, 2, 1}, 0);
}

DAWN_INSTANTIATE_TEST(OcclusionQueryTests, D3D12Backend(), MetalBackend(), VulkanBackend());

class PipelineStatisticsQueryTests : public QueryTests {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Skip all tests if pipeline statistics extension is not supported
        DAWN_SKIP_TEST_IF(!SupportsExtensions({"pipeline_statistics_query"}));
    }

    std::vector<const char*> GetRequiredExtensions() override {
        std::vector<const char*> requiredExtensions = {};
        if (SupportsExtensions({"pipeline_statistics_query"})) {
            requiredExtensions.push_back("pipeline_statistics_query");
        }

        return requiredExtensions;
    }
};

// Test creating query set with the type of PipelineStatistics
TEST_P(PipelineStatisticsQueryTests, QuerySetCreation) {
    wgpu::QuerySetDescriptor descriptor;
    descriptor.count = 1;
    descriptor.type = wgpu::QueryType::PipelineStatistics;
    wgpu::PipelineStatisticName pipelineStatistics[2] = {
        wgpu::PipelineStatisticName::ClipperInvocations,
        wgpu::PipelineStatisticName::VertexShaderInvocations};
    descriptor.pipelineStatistics = pipelineStatistics;
    descriptor.pipelineStatisticsCount = 2;
    device.CreateQuerySet(&descriptor);
}

DAWN_INSTANTIATE_TEST(PipelineStatisticsQueryTests,
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend());

class TimestampExpectation : public detail::Expectation {
  public:
    ~TimestampExpectation() override = default;

    // Expect the timestamp results are greater than 0.
    testing::AssertionResult Check(const void* data, size_t size) override {
        ASSERT(size % sizeof(uint64_t) == 0);
        const uint64_t* timestamps = static_cast<const uint64_t*>(data);
        for (size_t i = 0; i < size / sizeof(uint64_t); i++) {
            if (timestamps[i] == 0) {
                return testing::AssertionFailure()
                       << "Expected data[" << i << "] to be greater than 0." << std::endl;
            }
        }

        return testing::AssertionSuccess();
    }
};

class TimestampQueryTests : public QueryTests {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Skip all tests if timestamp extension is not supported
        DAWN_SKIP_TEST_IF(!SupportsExtensions({"timestamp_query"}));
    }

    std::vector<const char*> GetRequiredExtensions() override {
        std::vector<const char*> requiredExtensions = {};
        if (SupportsExtensions({"timestamp_query"})) {
            requiredExtensions.push_back("timestamp_query");
        }
        return requiredExtensions;
    }

    wgpu::QuerySet CreateQuerySetForTimestamp(uint32_t queryCount) {
        wgpu::QuerySetDescriptor descriptor;
        descriptor.count = queryCount;
        descriptor.type = wgpu::QueryType::Timestamp;
        return device.CreateQuerySet(&descriptor);
    }
};

// Test creating query set with the type of Timestamp
TEST_P(TimestampQueryTests, QuerySetCreation) {
    CreateQuerySetForTimestamp(1);
}

// Test calling timestamp query from command encoder
TEST_P(TimestampQueryTests, TimestampOnCommandEncoder) {
    // TODO(hao.x.li@intel.com): Crash occurs if we only call WriteTimestamp in a command encoder
    // without any copy commands on Metal on AMD GPU. See https://crbug.com/dawn/545.
    DAWN_SKIP_TEST_IF(IsMetal() && IsAMD());

    constexpr uint32_t kQueryCount = 2;

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.WriteTimestamp(querySet, 0);
    encoder.WriteTimestamp(querySet, 1);
    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
}

// Test calling timestamp query from render pass encoder
TEST_P(TimestampQueryTests, TimestampOnRenderPass) {
    constexpr uint32_t kQueryCount = 2;

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.WriteTimestamp(querySet, 0);
    pass.WriteTimestamp(querySet, 1);
    pass.EndPass();
    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
}

// Test calling timestamp query from compute pass encoder
TEST_P(TimestampQueryTests, TimestampOnComputePass) {
    constexpr uint32_t kQueryCount = 2;

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.WriteTimestamp(querySet, 0);
    pass.WriteTimestamp(querySet, 1);
    pass.EndPass();
    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
}

// Test resolving timestamp query to one slot in the buffer
TEST_P(TimestampQueryTests, ResolveToBufferWithOffset) {
    // TODO(hao.x.li@intel.com): Fail to resolve query to buffer with offset on Windows Vulkan and
    // Metal on Intel platforms, need investigation.
    DAWN_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());
    DAWN_SKIP_TEST_IF(IsIntel() && IsMetal());

    // TODO(hao.x.li@intel.com): Crash occurs if we only call WriteTimestamp in a command encoder
    // without any copy commands on Metal on AMD GPU. See https://crbug.com/dawn/545.
    DAWN_SKIP_TEST_IF(IsMetal() && IsAMD());

    constexpr uint32_t kQueryCount = 2;
    constexpr uint64_t kZero = 0;

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);

    // Resolve the query result to first slot in the buffer, other slots should not be written
    {
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.WriteTimestamp(querySet, 1);
        encoder.ResolveQuerySet(querySet, 0, 1, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, sizeof(uint64_t), new TimestampExpectation);
        EXPECT_BUFFER_U64_RANGE_EQ(&kZero, destination, sizeof(uint64_t), 1);
    }

    // Resolve the query result to the buffer with offset, the slots before the offset
    // should not be written
    {
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.WriteTimestamp(querySet, 1);
        encoder.ResolveQuerySet(querySet, 0, 1, destination, sizeof(uint64_t));
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U64_RANGE_EQ(&kZero, destination, 0, 1);
        EXPECT_BUFFER(destination, sizeof(uint64_t), sizeof(uint64_t), new TimestampExpectation);
    }
}

DAWN_INSTANTIATE_TEST(TimestampQueryTests, D3D12Backend(), MetalBackend(), VulkanBackend());
