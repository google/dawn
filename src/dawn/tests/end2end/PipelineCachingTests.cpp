// Copyright 2022 The Dawn Authors
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

#include <memory>
#include <string_view>

#include "dawn/tests/DawnTest.h"
#include "dawn/tests/end2end/mocks/CachingInterfaceMock.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {

using ::testing::NiceMock;

// TODO(dawn:549) Add some sort of pipeline descriptor repository to test more caching.

static constexpr std::string_view kComputeShader = R"(
        @stage(compute) @workgroup_size(1) fn main() {}
    )";

static constexpr std::string_view kVertexShader = R"(
        @stage(vertex) fn main() -> @builtin(position) vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 0.0);
        }
    )";

static constexpr std::string_view kFragmentShader = R"(
        @stage(fragment) fn main() {}
    )";

class PipelineCachingTests : public DawnTest {
  protected:
    std::unique_ptr<dawn::platform::Platform> CreateTestPlatform() override {
        return std::make_unique<DawnCachingMockPlatform>(&mMockCache);
    }

    NiceMock<CachingInterfaceMock> mMockCache;
};

class SinglePipelineCachingTests : public PipelineCachingTests {};

// Tests that pipeline creation works fine even if the cache is disabled.
// Note: This tests needs to use more than 1 device since the frontend cache on each device
//   will prevent going out to the blob cache.
TEST_P(SinglePipelineCachingTests, ComputePipelineNoCache) {
    mMockCache.Disable();

    // First time should create and since cache is disabled, it should not write out to the
    // cache.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShader.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 0u);

    // Second time should create fine with no cache hits since cache is disabled.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShader.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 0u);
}

// Tests that pipeline creation on the same device uses frontend cache when possible.
TEST_P(SinglePipelineCachingTests, ComputePipelineFrontedCache) {
    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, kComputeShader.data());
    desc.compute.entryPoint = "main";

    // First creation should create a cache entry.
    wgpu::ComputePipeline pipeline;
    EXPECT_CACHE_HIT(mMockCache, 0u, pipeline = device.CreateComputePipeline(&desc));
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);

    // Second creation on the same device should just return from frontend cache and should not
    // call out to the blob cache.
    EXPECT_CALL(mMockCache, LoadData).Times(0);
    wgpu::ComputePipeline samePipeline;
    EXPECT_CACHE_HIT(mMockCache, 0u, samePipeline = device.CreateComputePipeline(&desc));
    EXPECT_EQ(pipeline.Get() == samePipeline.Get(), !UsesWire());
}

// Tests that pipeline creation hits the cache when it is enabled.
// Note: This test needs to use more than 1 device since the frontend cache on each device
//   will prevent going out to the blob cache.
TEST_P(SinglePipelineCachingTests, ComputePipelineBlobCache) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShader.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);

    // Second time should create using the cache.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShader.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 1u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);
}

// Tests that pipeline creation does not hits the cache when it is enabled but we use different
// isolation keys.
TEST_P(SinglePipelineCachingTests, ComputePipelineBlobCacheIsolationKey) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice("isolation key 1");
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShader.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);

    // Second time should also create and write out to the cache.
    {
        wgpu::Device device = CreateDevice("isolation key 2");
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShader.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 2u);
}

// Tests that pipeline creation works fine even if the cache is disabled.
// Note: This tests needs to use more than 1 device since the frontend cache on each device
//   will prevent going out to the blob cache.
TEST_P(SinglePipelineCachingTests, RenderPipelineNoCache) {
    mMockCache.Disable();

    // First time should create and since cache is disabled, it should not write out to the
    // cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShader.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 0u);

    // Second time should create fine with no cache hits since cache is disabled.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShader.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 0u);
}

// Tests that pipeline creation on the same device uses frontend cache when possible.
TEST_P(SinglePipelineCachingTests, RenderPipelineFrontedCache) {
    utils::ComboRenderPipelineDescriptor desc;
    desc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
    desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
    desc.vertex.entryPoint = "main";
    desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShader.data());
    desc.cFragment.entryPoint = "main";

    // First creation should create a cache entry.
    wgpu::RenderPipeline pipeline;
    EXPECT_CACHE_HIT(mMockCache, 0u, pipeline = device.CreateRenderPipeline(&desc));
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);

    // Second creation on the same device should just return from frontend cache and should not
    // call out to the blob cache.
    EXPECT_CALL(mMockCache, LoadData).Times(0);
    wgpu::RenderPipeline samePipeline;
    EXPECT_CACHE_HIT(mMockCache, 0u, samePipeline = device.CreateRenderPipeline(&desc));
    EXPECT_EQ(pipeline.Get() == samePipeline.Get(), !UsesWire());
}

// Tests that pipeline creation hits the cache when it is enabled.
// Note: This test needs to use more than 1 device since the frontend cache on each device
//   will prevent going out to the blob cache.
TEST_P(SinglePipelineCachingTests, RenderPipelineBlobCache) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShader.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);

    // Second time should create using the cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShader.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 1u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);
}

// Tests that pipeline creation does not hits the cache when it is enabled but we use different
// isolation keys.
TEST_P(SinglePipelineCachingTests, RenderPipelineBlobCacheIsolationKey) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice("isolation key 1");
        utils::ComboRenderPipelineDescriptor desc;
        desc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShader.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);

    // Second time should also create and write out to the cache.
    {
        wgpu::Device device = CreateDevice("isolation key 2");
        utils::ComboRenderPipelineDescriptor desc;
        desc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShader.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 2u);
}

DAWN_INSTANTIATE_TEST(SinglePipelineCachingTests, VulkanBackend());

}  // namespace
