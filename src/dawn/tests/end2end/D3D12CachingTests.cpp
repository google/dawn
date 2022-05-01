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

#include <memory>
#include <unordered_map>
#include <utility>

#include "dawn/tests/DawnTest.h"
#include "dawn/tests/end2end/mocks/CachingInterfaceMock.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {
using ::testing::NiceMock;
}  // namespace

class D3D12CachingTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        // TODO(dawn:1341) Re-enable tests once shader caching is re-implemented.
        DAWN_SKIP_TEST_IF_BASE(true, "suppressed", "TODO(dawn:1341)");
    }

    std::unique_ptr<dawn::platform::Platform> CreateTestPlatform() override {
        return std::make_unique<DawnCachingMockPlatform>(&mMockCache);
    }

    NiceMock<CachingInterfaceMock> mMockCache;
};

// Test that duplicate WGSL still works (and re-compiles HLSL) when the cache is not enabled.
TEST_P(D3D12CachingTests, SameShaderNoCache) {
    mMockCache.Disable();

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @stage(vertex) fn vertex_main() -> @builtin(position) vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 1.0);
        }

        @stage(fragment) fn fragment_main() -> @location(0) vec4<f32> {
          return vec4<f32>(1.0, 0.0, 0.0, 1.0);
        }
    )");

    // Store the WGSL shader into the cache.
    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = module;
        desc.vertex.entryPoint = "vertex_main";
        desc.cFragment.module = module;
        desc.cFragment.entryPoint = "fragment_main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 0u);

    // Load the same WGSL shader from the cache.
    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = module;
        desc.vertex.entryPoint = "vertex_main";
        desc.cFragment.module = module;
        desc.cFragment.entryPoint = "fragment_main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 0u);
}

// Test creating a pipeline from two entrypoints in multiple stages will cache the correct number
// of HLSL shaders. WGSL shader should result into caching 2 HLSL shaders (stage x
// entrypoints)
TEST_P(D3D12CachingTests, ReuseShaderWithMultipleEntryPointsPerStage) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @stage(vertex) fn vertex_main() -> @builtin(position) vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 1.0);
        }

        @stage(fragment) fn fragment_main() -> @location(0) vec4<f32> {
          return vec4<f32>(1.0, 0.0, 0.0, 1.0);
        }
    )");

    // Store the WGSL shader into the cache.
    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = module;
        desc.vertex.entryPoint = "vertex_main";
        desc.cFragment.module = module;
        desc.cFragment.entryPoint = "fragment_main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 2u);

    // Load the same WGSL shader from the cache.
    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = module;
        desc.vertex.entryPoint = "vertex_main";
        desc.cFragment.module = module;
        desc.cFragment.entryPoint = "fragment_main";
        EXPECT_CACHE_HIT(mMockCache, 2u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 2u);

    // Modify the WGSL shader functions and make sure it doesn't hit.
    wgpu::ShaderModule newModule = utils::CreateShaderModule(device, R"(
      @stage(vertex) fn vertex_main() -> @builtin(position) vec4<f32> {
          return vec4<f32>(1.0, 1.0, 1.0, 1.0);
      }

      @stage(fragment) fn fragment_main() -> @location(0) vec4<f32> {
        return vec4<f32>(1.0, 1.0, 1.0, 1.0);
      }
  )");

    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = newModule;
        desc.vertex.entryPoint = "vertex_main";
        desc.cFragment.module = newModule;
        desc.cFragment.entryPoint = "fragment_main";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateRenderPipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 4u);
}

// Test creating a WGSL shader with two entrypoints in the same stage will cache the correct number
// of HLSL shaders. WGSL shader should result into caching 1 HLSL shader (stage x entrypoints)
TEST_P(D3D12CachingTests, ReuseShaderWithMultipleEntryPoints) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct Data {
            data : u32
        }
        @binding(0) @group(0) var<storage, read_write> data : Data;

        @stage(compute) @workgroup_size(1) fn write1() {
            data.data = 1u;
        }

        @stage(compute) @workgroup_size(1) fn write42() {
            data.data = 42u;
        }
    )");

    // Store the WGSL shader into the cache.
    {
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = module;
        desc.compute.entryPoint = "write1";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));

        desc.compute.module = module;
        desc.compute.entryPoint = "write42";
        EXPECT_CACHE_HIT(mMockCache, 0u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 2u);

    // Load the same WGSL shader from the cache.
    {
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = module;
        desc.compute.entryPoint = "write1";
        EXPECT_CACHE_HIT(mMockCache, 1u, device.CreateComputePipeline(&desc));

        desc.compute.module = module;
        desc.compute.entryPoint = "write42";
        EXPECT_CACHE_HIT(mMockCache, 1u, device.CreateComputePipeline(&desc));
    }
    EXPECT_EQ(mMockCache.GetNumEntries(), 2u);
}

DAWN_INSTANTIATE_TEST(D3D12CachingTests, D3D12Backend());
