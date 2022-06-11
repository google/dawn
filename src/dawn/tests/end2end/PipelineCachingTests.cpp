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
#include "dawn/tests/mocks/platform/CachingInterfaceMock.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {

using ::testing::NiceMock;

// TODO(dawn:549) Add some sort of pipeline descriptor repository to test more caching.

static constexpr std::string_view kComputeShaderDefault = R"(
        @compute @workgroup_size(1) fn main() {}
    )";

static constexpr std::string_view kComputeShaderMultipleEntryPoints = R"(
        @compute @workgroup_size(16) fn main() {}
        @compute @workgroup_size(64) fn main2() {}
    )";

static constexpr std::string_view kVertexShaderDefault = R"(
        @vertex fn main() -> @builtin(position) vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 0.0);
        }
    )";

static constexpr std::string_view kVertexShaderMultipleEntryPoints = R"(
        @vertex fn main() -> @builtin(position) vec4<f32> {
            return vec4<f32>(1.0, 0.0, 0.0, 1.0);
        }

        @vertex fn main2() -> @builtin(position) vec4<f32> {
            return vec4<f32>(0.5, 0.5, 0.5, 1.0);
        }
    )";

static constexpr std::string_view kFragmentShaderDefault = R"(
        @fragment fn main() -> @location(0) vec4<f32> {
            return vec4<f32>(0.1, 0.2, 0.3, 0.4);
        }
    )";

static constexpr std::string_view kFragmentShaderMultipleOutput = R"(
        struct FragmentOut {
            @location(0) fragColor0 : vec4<f32>,
            @location(1) fragColor1 : vec4<f32>,
        }

        @fragment fn main() -> FragmentOut {
            var output : FragmentOut;
            output.fragColor0 = vec4<f32>(0.1, 0.2, 0.3, 0.4);
            output.fragColor1 = vec4<f32>(0.5, 0.6, 0.7, 0.8);
            return output;
        }
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
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 0u);

    // Second time should create fine with no cache hits since cache is disabled.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 0u);
}

// Tests that pipeline creation on the same device uses frontend cache when possible.
TEST_P(SinglePipelineCachingTests, ComputePipelineFrontedCache) {
    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
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
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);

    // Second time should create using the cache.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 1u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);
}

// Tests that pipeline creation hits the cache when using the same pipeline but with explicit
// layout.
TEST_P(SinglePipelineCachingTests, ComputePipelineBlobCacheExplictLayout) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);

    // Cache should hit: use the same pipeline but with explicit pipeline layout.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        desc.layout = utils::MakeBasicPipelineLayout(device, {});
        EXPECT_CACHE_HIT(mMockCache, 1u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);
}

// Tests that pipeline creation wouldn't hit the cache if the pipelines are not exactly the same.
TEST_P(SinglePipelineCachingTests, ComputePipelineBlobCacheShaderNegativeCases) {
    size_t numCacheEntries = 0u;
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), ++numCacheEntries);

    // Cache should not hit: different shader module.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module =
            utils::CreateShaderModule(device, kComputeShaderMultipleEntryPoints.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), ++numCacheEntries);

    // Cache should not hit: same shader module but different shader entry point.
    {
        wgpu::Device device = CreateDevice();
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module =
            utils::CreateShaderModule(device, kComputeShaderMultipleEntryPoints.data());
        desc.compute.entryPoint = "main2";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), ++numCacheEntries);
}

// Tests that pipeline creation does not hits the cache when it is enabled but we use different
// isolation keys.
TEST_P(SinglePipelineCachingTests, ComputePipelineBlobCacheIsolationKey) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice("isolation key 1");
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
        desc.compute.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);

    // Second time should also create and write out to the cache.
    {
        wgpu::Device device = CreateDevice("isolation key 2");
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = utils::CreateShaderModule(device, kComputeShaderDefault.data());
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
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 0u);

    // Second time should create fine with no cache hits since cache is disabled.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 0u);
}

// Tests that pipeline creation on the same device uses frontend cache when possible.
TEST_P(SinglePipelineCachingTests, RenderPipelineFrontedCache) {
    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
    desc.vertex.entryPoint = "main";
    desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
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
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);

    // Second time should create using the cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 1u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);
}

// Tests that pipeline creation hits the cache when using the same pipeline but with explicit
// layout.
TEST_P(SinglePipelineCachingTests, RenderPipelineBlobCacheExplictLayout) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);

    // Cache should hit: use the same pipeline but with explicit pipeline layout.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        desc.layout = utils::MakeBasicPipelineLayout(device, {});
        EXPECT_CACHE_HIT(mMockCache, 1u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);
}

// Tests that pipeline creation wouldn't hit the cache if the pipelines have different state set in
// the descriptor.
TEST_P(SinglePipelineCachingTests, RenderPipelineBlobCacheDescriptorNegativeCases) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);

    // Cache should not hit: different pipeline descriptor state.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 2u);
}

// Tests that pipeline creation wouldn't hit the cache if the pipelines are not exactly the same in
// terms of shader.
TEST_P(SinglePipelineCachingTests, RenderPipelineBlobCacheShaderNegativeCases) {
    size_t numCacheEntries = 0u;
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), ++numCacheEntries);

    // Cache should not hit: different shader module.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module =
            utils::CreateShaderModule(device, kVertexShaderMultipleEntryPoints.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), ++numCacheEntries);

    // Cache should not hit: same shader module but different shader entry point.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module =
            utils::CreateShaderModule(device, kVertexShaderMultipleEntryPoints.data());
        desc.vertex.entryPoint = "main2";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), ++numCacheEntries);
}

// Tests that pipeline creation wouldn't hit the cache if the pipelines are not exactly the same
// (fragment color targets differences).
TEST_P(SinglePipelineCachingTests, RenderPipelineBlobCacheNegativeCasesFragmentColorTargets) {
    size_t numCacheEntries = 0u;
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.cFragment.targetCount = 2;
        desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        desc.cTargets[1].writeMask = wgpu::ColorWriteMask::None;
        desc.cTargets[1].format = wgpu::TextureFormat::RGBA8Unorm;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module =
            utils::CreateShaderModule(device, kFragmentShaderMultipleOutput.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), ++numCacheEntries);

    // Cache should not hit: different fragment color target state (sparse).
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.cFragment.targetCount = 2;
        desc.cTargets[0].format = wgpu::TextureFormat::Undefined;
        desc.cTargets[1].writeMask = wgpu::ColorWriteMask::None;
        desc.cTargets[1].format = wgpu::TextureFormat::RGBA8Unorm;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module =
            utils::CreateShaderModule(device, kFragmentShaderMultipleOutput.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), ++numCacheEntries);

    // Cache should not hit: different fragment color target state (trailing empty).
    {
        wgpu::Device device = CreateDevice();
        utils::ComboRenderPipelineDescriptor desc;
        desc.cFragment.targetCount = 2;
        desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        desc.cTargets[1].writeMask = wgpu::ColorWriteMask::None;
        desc.cTargets[1].format = wgpu::TextureFormat::Undefined;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module =
            utils::CreateShaderModule(device, kFragmentShaderMultipleOutput.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), ++numCacheEntries);
}

// Tests that pipeline creation does not hits the cache when it is enabled but we use different
// isolation keys.
TEST_P(SinglePipelineCachingTests, RenderPipelineBlobCacheIsolationKey) {
    // First time should create and write out to the cache.
    {
        wgpu::Device device = CreateDevice("isolation key 1");
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 1u);

    // Second time should also create and write out to the cache.
    {
        wgpu::Device device = CreateDevice("isolation key 2");
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, kVertexShaderDefault.data());
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, kFragmentShaderDefault.data());
        desc.cFragment.entryPoint = "main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 2u);
}

DAWN_INSTANTIATE_TEST(SinglePipelineCachingTests,
                      VulkanBackend({"enable_blob_cache"}),
                      D3D12Backend({"enable_blob_cache"}));

}  // namespace
