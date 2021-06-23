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

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

#define EXPECT_CACHE_HIT(N, statement)              \
    do {                                            \
        size_t before = mPersistentCache.mHitCount; \
        statement;                                  \
        FlushWire();                                \
        size_t after = mPersistentCache.mHitCount;  \
        EXPECT_EQ(N, after - before);               \
    } while (0)

// FakePersistentCache implements a in-memory persistent cache.
class FakePersistentCache : public dawn_platform::CachingInterface {
  public:
    // PersistentCache API
    void StoreData(const WGPUDevice device,
                   const void* key,
                   size_t keySize,
                   const void* value,
                   size_t valueSize) override {
        if (mIsDisabled)
            return;
        const std::string keyStr(reinterpret_cast<const char*>(key), keySize);

        const uint8_t* value_start = reinterpret_cast<const uint8_t*>(value);
        std::vector<uint8_t> entry_value(value_start, value_start + valueSize);

        EXPECT_TRUE(mCache.insert({keyStr, std::move(entry_value)}).second);
    }

    size_t LoadData(const WGPUDevice device,
                    const void* key,
                    size_t keySize,
                    void* value,
                    size_t valueSize) override {
        const std::string keyStr(reinterpret_cast<const char*>(key), keySize);
        auto entry = mCache.find(keyStr);
        if (entry == mCache.end()) {
            return 0;
        }
        if (valueSize >= entry->second.size()) {
            memcpy(value, entry->second.data(), entry->second.size());
        }
        mHitCount++;
        return entry->second.size();
    }

    using Blob = std::vector<uint8_t>;
    using FakeCache = std::unordered_map<std::string, Blob>;

    FakeCache mCache;

    size_t mHitCount = 0;
    bool mIsDisabled = false;
};

// Test platform that only supports caching.
class DawnTestPlatform : public dawn_platform::Platform {
  public:
    DawnTestPlatform(dawn_platform::CachingInterface* cachingInterface)
        : mCachingInterface(cachingInterface) {
    }
    ~DawnTestPlatform() override = default;

    dawn_platform::CachingInterface* GetCachingInterface(const void* fingerprint,
                                                         size_t fingerprintSize) override {
        return mCachingInterface;
    }

    dawn_platform::CachingInterface* mCachingInterface = nullptr;
};

class D3D12CachingTests : public DawnTest {
  protected:
    std::unique_ptr<dawn_platform::Platform> CreateTestPlatform() override {
        return std::make_unique<DawnTestPlatform>(&mPersistentCache);
    }

    FakePersistentCache mPersistentCache;
};

// Test that duplicate WGSL still re-compiles HLSL even when the cache is not enabled.
TEST_P(D3D12CachingTests, SameShaderNoCache) {
    mPersistentCache.mIsDisabled = true;

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        [[stage(vertex)]] fn vertex_main() -> [[builtin(position)]] vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 1.0);
        }

        [[stage(fragment)]] fn fragment_main() -> [[location(0)]] vec4<f32> {
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

        EXPECT_CACHE_HIT(0u, device.CreateRenderPipeline(&desc));
    }

    EXPECT_EQ(mPersistentCache.mCache.size(), 0u);

    // Load the same WGSL shader from the cache.
    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = module;
        desc.vertex.entryPoint = "vertex_main";
        desc.cFragment.module = module;
        desc.cFragment.entryPoint = "fragment_main";

        EXPECT_CACHE_HIT(0u, device.CreateRenderPipeline(&desc));
    }

    EXPECT_EQ(mPersistentCache.mCache.size(), 0u);
}

// Test creating a pipeline from two entrypoints in multiple stages will cache the correct number
// of HLSL shaders. WGSL shader should result into caching 2 HLSL shaders (stage x
// entrypoints)
TEST_P(D3D12CachingTests, ReuseShaderWithMultipleEntryPointsPerStage) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        [[stage(vertex)]] fn vertex_main() -> [[builtin(position)]] vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 1.0);
        }

        [[stage(fragment)]] fn fragment_main() -> [[location(0)]] vec4<f32> {
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

        EXPECT_CACHE_HIT(0u, device.CreateRenderPipeline(&desc));
    }

    EXPECT_EQ(mPersistentCache.mCache.size(), 2u);

    // Load the same WGSL shader from the cache.
    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = module;
        desc.vertex.entryPoint = "vertex_main";
        desc.cFragment.module = module;
        desc.cFragment.entryPoint = "fragment_main";

        // Cached HLSL shader calls LoadData twice (once to peek, again to get), so check 2 x
        // kNumOfShaders hits.
        EXPECT_CACHE_HIT(4u, device.CreateRenderPipeline(&desc));
    }

    EXPECT_EQ(mPersistentCache.mCache.size(), 2u);

    // Modify the WGSL shader functions and make sure it doesn't hit.
    wgpu::ShaderModule newModule = utils::CreateShaderModule(device, R"(
      [[stage(vertex)]] fn vertex_main() -> [[builtin(position)]] vec4<f32> {
          return vec4<f32>(1.0, 1.0, 1.0, 1.0);
      }

      [[stage(fragment)]] fn fragment_main() -> [[location(0)]] vec4<f32> {
        return vec4<f32>(1.0, 1.0, 1.0, 1.0);
      }
  )");

    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = newModule;
        desc.vertex.entryPoint = "vertex_main";
        desc.cFragment.module = newModule;
        desc.cFragment.entryPoint = "fragment_main";
        EXPECT_CACHE_HIT(0u, device.CreateRenderPipeline(&desc));
    }

    // Cached HLSL shader calls LoadData twice (once to peek, again to get), so check 2 x
    // kNumOfShaders hits.
    EXPECT_EQ(mPersistentCache.mCache.size(), 4u);
}

// Test creating a WGSL shader with two entrypoints in the same stage will cache the correct number
// of HLSL shaders. WGSL shader should result into caching 1 HLSL shader (stage x entrypoints)
TEST_P(D3D12CachingTests, ReuseShaderWithMultipleEntryPoints) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        [[block]] struct Data {
            data : u32;
        };
        [[binding(0), group(0)]] var<storage, read_write> data : Data;

        [[stage(compute), workgroup_size(1)]] fn write1() {
            data.data = 1u;
        }

        [[stage(compute), workgroup_size(1)]] fn write42() {
            data.data = 42u;
        }
    )");

    // Store the WGSL shader into the cache.
    {
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = module;
        desc.compute.entryPoint = "write1";
        EXPECT_CACHE_HIT(0u, device.CreateComputePipeline(&desc));

        desc.compute.module = module;
        desc.compute.entryPoint = "write42";
        EXPECT_CACHE_HIT(0u, device.CreateComputePipeline(&desc));
    }

    EXPECT_EQ(mPersistentCache.mCache.size(), 2u);

    // Load the same WGSL shader from the cache.
    {
        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = module;
        desc.compute.entryPoint = "write1";

        // Cached HLSL shader calls LoadData twice (once to peek, again to get), so check 2 x
        // kNumOfShaders hits.
        EXPECT_CACHE_HIT(2u, device.CreateComputePipeline(&desc));

        desc.compute.module = module;
        desc.compute.entryPoint = "write42";

        // Cached HLSL shader calls LoadData twice, so check 2 x kNumOfShaders hits.
        EXPECT_CACHE_HIT(2u, device.CreateComputePipeline(&desc));
    }

    EXPECT_EQ(mPersistentCache.mCache.size(), 2u);
}

DAWN_INSTANTIATE_TEST(D3D12CachingTests, D3D12Backend());
