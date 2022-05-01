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

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

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

// Clear the content of the result buffer into 0xFFFFFFFF.
constexpr static uint64_t kSentinelValue = ~uint64_t(0u);
constexpr static uint64_t kZero = 0u;
constexpr uint64_t kMinDestinationOffset = 256;
constexpr uint64_t kMinCount = kMinDestinationOffset / sizeof(uint64_t);

class OcclusionExpectation : public detail::Expectation {
  public:
    enum class Result { Zero, NonZero };

    ~OcclusionExpectation() override = default;

    explicit OcclusionExpectation(Result expected) { mExpected = expected; }

    testing::AssertionResult Check(const void* data, size_t size) override {
        ASSERT(size % sizeof(uint64_t) == 0);
        const uint64_t* actual = static_cast<const uint64_t*>(data);
        for (size_t i = 0; i < size / sizeof(uint64_t); i++) {
            if (actual[i] == kSentinelValue) {
                return testing::AssertionFailure()
                       << "Data[" << i << "] was not written (it kept the sentinel value of "
                       << kSentinelValue << ")." << std::endl;
            }
            if (mExpected == Result::Zero && actual[i] != 0) {
                return testing::AssertionFailure()
                       << "Expected data[" << i << "] to be zero, actual: " << actual[i] << "."
                       << std::endl;
            }
            if (mExpected == Result::NonZero && actual[i] == 0) {
                return testing::AssertionFailure()
                       << "Expected data[" << i << "] to be non-zero." << std::endl;
            }
        }

        return testing::AssertionSuccess();
    }

  private:
    Result mExpected;
};

class OcclusionQueryTests : public QueryTests {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Create basic render pipeline
        vsModule = utils::CreateShaderModule(device, R"(
            @stage(vertex)
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
                var pos = array<vec2<f32>, 3>(
                    vec2<f32>( 1.0,  1.0),
                    vec2<f32>(-1.0, -1.0),
                    vec2<f32>( 1.0, -1.0));
                return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
            })");

        fsModule = utils::CreateShaderModule(device, R"(
            @stage(fragment) fn main() -> @location(0) vec4<f32> {
                return vec4<f32>(0.0, 1.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;

        pipeline = device.CreateRenderPipeline(&descriptor);
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
                                                OcclusionExpectation::Result expected) {
        constexpr uint32_t kQueryCount = 1;

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;

        // Enable depth and stencil tests and set comparison tests never pass.
        wgpu::DepthStencilState* depthStencil =
            descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24PlusStencil8);
        depthStencil->depthCompare =
            depthTestEnabled ? wgpu::CompareFunction::Never : wgpu::CompareFunction::Always;
        depthStencil->stencilFront.compare =
            stencilTestEnabled ? wgpu::CompareFunction::Never : wgpu::CompareFunction::Always;
        depthStencil->stencilBack.compare =
            stencilTestEnabled ? wgpu::CompareFunction::Never : wgpu::CompareFunction::Always;

        wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&descriptor);

        wgpu::Texture renderTarget = CreateRenderTexture(wgpu::TextureFormat::RGBA8Unorm);
        wgpu::TextureView renderTargetView = renderTarget.CreateView();

        wgpu::Texture depthTexture = CreateRenderTexture(wgpu::TextureFormat::Depth24PlusStencil8);
        wgpu::TextureView depthTextureView = depthTexture.CreateView();

        wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
        // Set all bits in buffer to check 0 is correctly written if there is no sample passed the
        // occlusion testing
        queue.WriteBuffer(destination, 0, &kSentinelValue, sizeof(kSentinelValue));

        utils::ComboRenderPassDescriptor renderPass({renderTargetView}, depthTextureView);
        renderPass.occlusionQuerySet = querySet;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(renderPipeline);
        pass.SetStencilReference(0);
        pass.BeginOcclusionQuery(0);
        pass.Draw(3);
        pass.EndOcclusionQuery();
        pass.End();

        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, sizeof(uint64_t), new OcclusionExpectation(expected));
    }

    void TestOcclusionQueryWithScissorTest(ScissorRect rect,
                                           OcclusionExpectation::Result expected) {
        constexpr uint32_t kQueryCount = 1;

        wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
        // Set all bits in buffer to check 0 is correctly written if there is no sample passed the
        // occlusion testing
        queue.WriteBuffer(destination, 0, &kSentinelValue, sizeof(kSentinelValue));

        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        renderPass.renderPassInfo.occlusionQuerySet = querySet;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetScissorRect(rect.x, rect.y, rect.width, rect.height);
        pass.BeginOcclusionQuery(0);
        pass.Draw(3);
        pass.EndOcclusionQuery();
        pass.End();

        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, sizeof(uint64_t), new OcclusionExpectation(expected));
    }

    wgpu::ShaderModule vsModule;
    wgpu::ShaderModule fsModule;

    wgpu::RenderPipeline pipeline;

    constexpr static unsigned int kRTSize = 4;
};

// Test creating query set with the type of Occlusion
TEST_P(OcclusionQueryTests, QuerySetCreation) {
    // Zero-sized query set is allowed.
    CreateOcclusionQuerySet(0);

    CreateOcclusionQuerySet(1);
}

// Test destroying query set
TEST_P(OcclusionQueryTests, QuerySetDestroy) {
    wgpu::QuerySet querySet = CreateOcclusionQuerySet(1);
    querySet.Destroy();
}

// Draw a bottom right triangle with depth/stencil testing enabled and check whether there is
// sample passed the testing by non-precise occlusion query with the results:
// zero indicates that no sample passed depth/stencil testing,
// non-zero indicates that at least one sample passed depth/stencil testing.
TEST_P(OcclusionQueryTests, QueryWithDepthStencilTest) {
    // Disable depth/stencil testing, the samples always pass the testing, the expected occlusion
    // result is non-zero.
    TestOcclusionQueryWithDepthStencilTest(false, false, OcclusionExpectation::Result::NonZero);

    // Only enable depth testing and set the samples never pass the testing, the expected occlusion
    // result is zero.
    TestOcclusionQueryWithDepthStencilTest(true, false, OcclusionExpectation::Result::Zero);

    // Only enable stencil testing and set the samples never pass the testing, the expected
    // occlusion result is zero.
    TestOcclusionQueryWithDepthStencilTest(false, true, OcclusionExpectation::Result::Zero);
}

// Draw a bottom right triangle with scissor testing enabled and check whether there is
// sample passed the testing by non-precise occlusion query with the results:
// zero indicates that no sample passed scissor testing,
// non-zero indicates that at least one sample passed scissor testing.
TEST_P(OcclusionQueryTests, QueryWithScissorTest) {
    // TODO(hao.x.li@intel.com): It's failed weirdly on Intel TGL（Window Vulkan) which says
    // the destination buffer keep sentinel value in the second case, it cannot be reproduced with
    // any debug actions including Vulkan validation layers enabled, and takes time to find out if
    // the WriteBuffer and ResolveQuerySet are not executed in order or the ResolveQuerySet does not
    // copy the result to the buffer. In order to integrate end2end tests to Intel driver CL without
    // unknown issues, skip it until we find the root cause.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsIntel());

    // Test there are samples passed scissor testing, the expected occlusion result is non-zero.
    TestOcclusionQueryWithScissorTest({2, 1, 2, 1}, OcclusionExpectation::Result::NonZero);

    // Test there is no sample passed scissor testing, the expected occlusion result is zero.
    TestOcclusionQueryWithScissorTest({0, 0, 2, 1}, OcclusionExpectation::Result::Zero);
}

// Test begin occlusion query with same query index on different render pass
TEST_P(OcclusionQueryTests, Rewrite) {
    constexpr uint32_t kQueryCount = 1;

    wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    // Set all bits in buffer to check 0 is correctly written if there is no sample passed the
    // occlusion testing
    queue.WriteBuffer(destination, 0, &kSentinelValue, sizeof(kSentinelValue));

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    renderPass.renderPassInfo.occlusionQuerySet = querySet;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Begin occlusion without draw call
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.BeginOcclusionQuery(0);
    pass.EndOcclusionQuery();
    pass.End();

    // Begin occlusion with same query index with draw call
    wgpu::RenderPassEncoder rewritePass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    rewritePass.SetPipeline(pipeline);
    rewritePass.BeginOcclusionQuery(0);
    rewritePass.Draw(3);
    rewritePass.EndOcclusionQuery();
    rewritePass.End();

    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER(destination, 0, sizeof(uint64_t),
                  new OcclusionExpectation(OcclusionExpectation::Result::NonZero));
}

// Test resolving occlusion query correctly if the queries are written sparsely, which also tests
// the query resetting at the start of render passes on Vulkan backend.
TEST_P(OcclusionQueryTests, ResolveSparseQueries) {
    // TODO(hao.x.li@intel.com): Fails on Intel Windows Vulkan due to a driver issue that
    // vkCmdFillBuffer and vkCmdCopyQueryPoolResults are not executed in order, skip it util
    // the issue is fixed.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsIntel());

    // TODO(hao.x.li@intel.com): Investigate why it's failed on D3D12 on Nvidia when running with
    // the previous occlusion tests. Expect resolve to 0 for these unwritten queries but the
    // occlusion result of the previous tests is got.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());

    constexpr uint32_t kQueryCount = 7;

    wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    // Set sentinel values to check the queries are resolved correctly if the queries are
    // written sparsely.
    std::vector<uint64_t> sentinelValues(kQueryCount, kSentinelValue);
    queue.WriteBuffer(destination, 0, sentinelValues.data(), kQueryCount * sizeof(uint64_t));

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    renderPass.renderPassInfo.occlusionQuerySet = querySet;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);

    // Write queries sparsely for testing the query resetting on Vulkan and resolving unwritten
    // queries to 0.
    // 0 - not written (tests starting with not written).
    // 1 - written (tests combing multiple written, although other tests already do it).
    // 2 - written.
    // 3 - not written (tests skipping over not written in the middle).
    // 4 - not written.
    // 5 - written (tests another written query in the middle).
    // 6 - not written (tests the last query not being written).
    pass.BeginOcclusionQuery(1);
    pass.Draw(3);
    pass.EndOcclusionQuery();
    pass.BeginOcclusionQuery(2);
    pass.Draw(3);
    pass.EndOcclusionQuery();
    pass.BeginOcclusionQuery(5);
    pass.Draw(3);
    pass.EndOcclusionQuery();
    pass.End();

    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The query at index 0 should be resolved to 0.
    EXPECT_BUFFER_U64_RANGE_EQ(&kZero, destination, 0, 1);
    EXPECT_BUFFER(destination, sizeof(uint64_t), 2 * sizeof(uint64_t),
                  new OcclusionExpectation(OcclusionExpectation::Result::NonZero));
    // The queries at index 3 and 4 should be resolved to 0.
    std::vector<uint64_t> zeros(2, kZero);
    EXPECT_BUFFER_U64_RANGE_EQ(zeros.data(), destination, 3 * sizeof(uint64_t), 2);
    EXPECT_BUFFER(destination, 5 * sizeof(uint64_t), sizeof(uint64_t),
                  new OcclusionExpectation(OcclusionExpectation::Result::NonZero));
    // The query at index 6 should be resolved to 0.
    EXPECT_BUFFER_U64_RANGE_EQ(&kZero, destination, 6 * sizeof(uint64_t), 1);
}

// Test resolving occlusion query to 0 if all queries are not written
TEST_P(OcclusionQueryTests, ResolveWithoutWritten) {
    // TODO(hao.x.li@intel.com): Investigate why it's failed on D3D12 on Nvidia when running with
    // the previous occlusion tests. Expect resolve to 0 but the occlusion result of the previous
    // tests is got.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());

    constexpr uint32_t kQueryCount = 1;

    wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    // Set sentinel values to check 0 is correctly written if resolving query set without
    // any written.
    queue.WriteBuffer(destination, 0, &kSentinelValue, sizeof(kSentinelValue));

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U64_RANGE_EQ(&kZero, destination, 0, 1);
}

// Test resolving occlusion query to the destination buffer with offset
TEST_P(OcclusionQueryTests, ResolveToBufferWithOffset) {
    constexpr uint32_t kQueryCount = 2;

    wgpu::QuerySet querySet = CreateOcclusionQuerySet(kQueryCount);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    renderPass.renderPassInfo.occlusionQuerySet = querySet;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.BeginOcclusionQuery(0);
    pass.Draw(3);
    pass.EndOcclusionQuery();
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    constexpr uint64_t kBufferSize = kQueryCount * sizeof(uint64_t) + kMinDestinationOffset;
    constexpr uint64_t kCount = kQueryCount + kMinCount;

    // Resolve the query result to first slot in the buffer, other slots should not be written.
    {
        wgpu::Buffer destination = CreateResolveBuffer(kBufferSize);
        // Set sentinel values to check the query is resolved to the correct slot of the buffer.
        std::vector<uint64_t> sentinelValues(kCount, kSentinelValue);
        queue.WriteBuffer(destination, 0, sentinelValues.data(), kBufferSize);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, 1, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, sizeof(uint64_t),
                      new OcclusionExpectation(OcclusionExpectation::Result::NonZero));
        EXPECT_BUFFER_U64_RANGE_EQ(sentinelValues.data(), destination, sizeof(uint64_t),
                                   kCount - 1);
    }

    // Resolve the query result to second slot in the buffer, the first one should not be written.
    {
        wgpu::Buffer destination = CreateResolveBuffer(kBufferSize);
        // Set sentinel values to check the query is resolved to the correct slot of the buffer.
        std::vector<uint64_t> sentinelValues(kCount, kSentinelValue);
        queue.WriteBuffer(destination, 0, sentinelValues.data(), kBufferSize);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ResolveQuerySet(querySet, 0, 1, destination, kMinDestinationOffset);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U64_RANGE_EQ(sentinelValues.data(), destination, 0, kMinCount);
        EXPECT_BUFFER(destination, kMinDestinationOffset, sizeof(uint64_t),
                      new OcclusionExpectation(OcclusionExpectation::Result::NonZero));
    }
}

DAWN_INSTANTIATE_TEST(OcclusionQueryTests, D3D12Backend(), MetalBackend(), VulkanBackend());

class PipelineStatisticsQueryTests : public QueryTests {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Skip all tests if pipeline statistics feature is not supported
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::PipelineStatisticsQuery}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::PipelineStatisticsQuery})) {
            requiredFeatures.push_back(wgpu::FeatureName::PipelineStatisticsQuery);
        }

        return requiredFeatures;
    }

    wgpu::QuerySet CreateQuerySetForPipelineStatistics(
        uint32_t queryCount,
        std::vector<wgpu::PipelineStatisticName> pipelineStatistics = {}) {
        wgpu::QuerySetDescriptor descriptor;
        descriptor.count = queryCount;
        descriptor.type = wgpu::QueryType::PipelineStatistics;

        if (pipelineStatistics.size() > 0) {
            descriptor.pipelineStatistics = pipelineStatistics.data();
            descriptor.pipelineStatisticsCount = pipelineStatistics.size();
        }
        return device.CreateQuerySet(&descriptor);
    }
};

// Test creating query set with the type of PipelineStatistics
TEST_P(PipelineStatisticsQueryTests, QuerySetCreation) {
    // Zero-sized query set is allowed.
    CreateQuerySetForPipelineStatistics(0, {wgpu::PipelineStatisticName::ClipperInvocations,
                                            wgpu::PipelineStatisticName::VertexShaderInvocations});

    CreateQuerySetForPipelineStatistics(1, {wgpu::PipelineStatisticName::ClipperInvocations,
                                            wgpu::PipelineStatisticName::VertexShaderInvocations});
}

DAWN_INSTANTIATE_TEST(PipelineStatisticsQueryTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
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

        // Skip all tests if timestamp feature is not supported
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::TimestampQuery}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::TimestampQuery})) {
            requiredFeatures.push_back(wgpu::FeatureName::TimestampQuery);
        }
        return requiredFeatures;
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
    // Zero-sized query set is allowed.
    CreateQuerySetForTimestamp(0);

    CreateQuerySetForTimestamp(1);
}

// Test calling timestamp query from command encoder
TEST_P(TimestampQueryTests, TimestampOnCommandEncoder) {
    constexpr uint32_t kQueryCount = 2;

    // Write timestamp with different query indexes
    {
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

    // Write timestamp with same query index outside pass on same encoder
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.WriteTimestamp(querySet, 1);
        encoder.WriteTimestamp(querySet, 0);
        encoder.WriteTimestamp(querySet, 1);
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    }
}

// Test calling timestamp query from render pass encoder
TEST_P(TimestampQueryTests, TimestampOnRenderPass) {
    constexpr uint32_t kQueryCount = 2;

    // Write timestamp with different query indexes
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.WriteTimestamp(querySet, 0);
        pass.WriteTimestamp(querySet, 1);
        pass.End();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    }

    // Write timestamp with same query index, not need test rewrite inside render pass due to it's
    // not allowed
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.WriteTimestamp(querySet, 1);

        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.WriteTimestamp(querySet, 0);
        pass.WriteTimestamp(querySet, 1);
        pass.End();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    }
}

// Test calling timestamp query from compute pass encoder
TEST_P(TimestampQueryTests, TimestampOnComputePass) {
    constexpr uint32_t kQueryCount = 2;

    // Write timestamp with different query indexes
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.WriteTimestamp(querySet, 0);
        pass.WriteTimestamp(querySet, 1);
        pass.End();
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    }

    // Write timestamp with same query index on both the outside and the inside of the compute pass
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.WriteTimestamp(querySet, 1);

        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.WriteTimestamp(querySet, 0);
        pass.WriteTimestamp(querySet, 1);
        pass.End();

        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    }

    // Write timestamp with same query index inside compute pass
    {
        wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
        wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.WriteTimestamp(querySet, 0);
        pass.WriteTimestamp(querySet, 1);
        pass.WriteTimestamp(querySet, 0);
        pass.WriteTimestamp(querySet, 1);
        pass.End();

        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
    }
}

// Test resolving timestamp query from another different encoder
TEST_P(TimestampQueryTests, ResolveFromAnotherEncoder) {
    constexpr uint32_t kQueryCount = 2;

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

    wgpu::CommandEncoder timestampEncoder = device.CreateCommandEncoder();
    timestampEncoder.WriteTimestamp(querySet, 0);
    timestampEncoder.WriteTimestamp(querySet, 1);
    wgpu::CommandBuffer timestampCommands = timestampEncoder.Finish();
    queue.Submit(1, &timestampCommands);

    wgpu::CommandEncoder resolveEncoder = device.CreateCommandEncoder();
    resolveEncoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer resolveCommands = resolveEncoder.Finish();
    queue.Submit(1, &resolveCommands);

    EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
}

// Test resolving timestamp query correctly if the queries are written sparsely
TEST_P(TimestampQueryTests, ResolveSparseQueries) {
    // TODO(hao.x.li@intel.com): Fails on Intel Windows Vulkan due to a driver issue that
    // vkCmdFillBuffer and vkCmdCopyQueryPoolResults are not executed in order, skip it util
    // the issue is fixed.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsIntel());

    constexpr uint32_t kQueryCount = 4;

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    // Set sentinel values to check the queries are resolved correctly if the queries are
    // written sparsely
    std::vector<uint64_t> sentinelValues{0, kSentinelValue, 0, kSentinelValue};
    queue.WriteBuffer(destination, 0, sentinelValues.data(), kQueryCount * sizeof(uint64_t));

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.WriteTimestamp(querySet, 0);
    encoder.WriteTimestamp(querySet, 2);
    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER(destination, 0, sizeof(uint64_t), new TimestampExpectation);
    // The query with no value written should be resolved to 0.
    EXPECT_BUFFER_U64_RANGE_EQ(&kZero, destination, sizeof(uint64_t), 1);
    EXPECT_BUFFER(destination, 2 * sizeof(uint64_t), sizeof(uint64_t), new TimestampExpectation);
    // The query with no value written should be resolved to 0.
    EXPECT_BUFFER_U64_RANGE_EQ(&kZero, destination, 3 * sizeof(uint64_t), 1);
}

// Test resolving timestamp query to 0 if all queries are not written
TEST_P(TimestampQueryTests, ResolveWithoutWritten) {
    constexpr uint32_t kQueryCount = 2;

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    // Set sentinel values to check 0 is correctly written if resolving query set with no
    // query is written
    std::vector<uint64_t> sentinelValues(kQueryCount, kSentinelValue);
    queue.WriteBuffer(destination, 0, sentinelValues.data(), kQueryCount * sizeof(uint64_t));

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint64_t> expectedZeros(kQueryCount);
    EXPECT_BUFFER_U64_RANGE_EQ(expectedZeros.data(), destination, 0, kQueryCount);
}

// Test resolving timestamp query to one slot in the buffer
TEST_P(TimestampQueryTests, ResolveToBufferWithOffset) {
    // TODO(hao.x.li@intel.com): Fails on Intel Windows Vulkan due to a driver issue that
    // vkCmdFillBuffer and vkCmdCopyQueryPoolResults are not executed in order, skip it util
    // the issue is fixed.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsIntel());

    constexpr uint32_t kQueryCount = 2;
    constexpr uint64_t kBufferSize = kQueryCount * sizeof(uint64_t) + kMinDestinationOffset;
    constexpr uint64_t kCount = kQueryCount + kMinCount;

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);

    // Resolve the query result to first slot in the buffer, other slots should not be written
    {
        wgpu::Buffer destination = CreateResolveBuffer(kBufferSize);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.ResolveQuerySet(querySet, 0, 1, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        std::vector<uint64_t> zeros(kCount - 1, kZero);
        EXPECT_BUFFER(destination, 0, sizeof(uint64_t), new TimestampExpectation);
        EXPECT_BUFFER_U64_RANGE_EQ(zeros.data(), destination, sizeof(uint64_t), kCount - 1);
    }

    // Resolve the query result to the buffer with offset, the slots before the offset
    // should not be written
    {
        wgpu::Buffer destination = CreateResolveBuffer(kBufferSize);
        // Set sentinel values to check the query is resolved to the correct slot of the buffer.
        std::vector<uint64_t> sentinelValues(kCount, kZero);
        queue.WriteBuffer(destination, 0, sentinelValues.data(), kBufferSize);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.ResolveQuerySet(querySet, 0, 1, destination, kMinDestinationOffset);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        std::vector<uint64_t> zeros(kMinCount, kZero);
        EXPECT_BUFFER_U64_RANGE_EQ(zeros.data(), destination, 0, kMinCount);
        EXPECT_BUFFER(destination, kMinDestinationOffset, sizeof(uint64_t),
                      new TimestampExpectation);
    }
}

// Test resolving a query set twice into the same destination buffer with potentially overlapping
// ranges
TEST_P(TimestampQueryTests, ResolveTwiceToSameBuffer) {
    // TODO(hao.x.li@intel.com): Fails on Intel Windows Vulkan due to a driver issue that
    // vkCmdFillBuffer and vkCmdCopyQueryPoolResults are not executed in order, skip it util
    // the issue is fixed.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsIntel());

    constexpr uint32_t kQueryCount = kMinCount + 2;

    wgpu::QuerySet querySet = CreateQuerySetForTimestamp(kQueryCount);
    wgpu::Buffer destination = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    for (uint32_t i = 0; i < kQueryCount; i++) {
        encoder.WriteTimestamp(querySet, i);
    }
    encoder.ResolveQuerySet(querySet, 0, kMinCount + 1, destination, 0);
    encoder.ResolveQuerySet(querySet, kMinCount, 2, destination, kMinDestinationOffset);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t), new TimestampExpectation);
}

DAWN_INSTANTIATE_TEST(TimestampQueryTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
