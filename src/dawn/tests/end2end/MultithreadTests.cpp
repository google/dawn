// Copyright 2023 The Dawn Authors
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

#include <functional>
#include <limits>
#include <memory>
#include <thread>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/common/Mutex.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

#define LOCKED_CMD(CMD)                   \
    do {                                  \
        dawn::Mutex::AutoLock lk(&mutex); \
        CMD;                              \
    } while (0)

namespace {
class MultithreadTests : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> features;
        // TODO(crbug.com/dawn/1678): DawnWire doesn't support thread safe API yet.
        if (!UsesWire()) {
            features.push_back(wgpu::FeatureName::ImplicitDeviceSynchronization);
        }
        return features;
    }

    void SetUp() override {
        DawnTest::SetUp();
        // TODO(crbug.com/dawn/1678): DawnWire doesn't support thread safe API yet.
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());

        // TODO(crbug.com/dawn/1679): OpenGL backend doesn't support thread safe API yet.
        DAWN_TEST_UNSUPPORTED_IF(IsOpenGL() || IsOpenGLES());
    }

    wgpu::Buffer CreateBuffer(uint32_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage;
        return device.CreateBuffer(&descriptor);
    }

    wgpu::Texture CreateTexture(uint32_t width,
                                uint32_t height,
                                wgpu::TextureFormat format,
                                wgpu::TextureUsage usage,
                                uint32_t mipLevelCount = 1,
                                uint32_t sampleCount = 1) {
        wgpu::TextureDescriptor texDescriptor = {};
        texDescriptor.size = {width, height, 1};
        texDescriptor.format = format;
        texDescriptor.usage = usage;
        texDescriptor.mipLevelCount = mipLevelCount;
        texDescriptor.sampleCount = sampleCount;
        return device.CreateTexture(&texDescriptor);
    }

    void RunInParallel(uint32_t numThreads, const std::function<void(uint32_t)>& workerFunc) {
        std::vector<std::unique_ptr<std::thread>> threads(numThreads);

        for (uint32_t i = 0; i < threads.size(); ++i) {
            threads[i] = std::make_unique<std::thread>([i, workerFunc] { workerFunc(i); });
        }

        for (auto& thread : threads) {
            thread->join();
        }
    }

    dawn::Mutex mutex;
};

class MultithreadCachingTests : public MultithreadTests {
  protected:
    wgpu::ShaderModule CreateComputeShaderModule() const {
        return utils::CreateShaderModule(device, R"(
            struct SSBO {
                value : u32
            }
            @group(0) @binding(0) var<storage, read_write> ssbo : SSBO;

            @compute @workgroup_size(1) fn main() {
                ssbo.value = 1;
            })");
    }

    wgpu::BindGroupLayout CreateComputeBindGroupLayout() const {
        return utils::MakeBindGroupLayout(
            device, {
                        {0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
                    });
    }
};

// Test that creating a same shader module (which will return the cached shader module) and release
// it on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedShaderModulesInParallel) {
    RunInParallel(100, [this](uint32_t) {
        wgpu::ShaderModule csModule = CreateComputeShaderModule();
        EXPECT_NE(nullptr, csModule.Get());
    });
}

// Test that creating a same compute pipeline (which will return the cached pipeline) and release it
// on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedComputePipelinesInParallel) {
    wgpu::ShaderModule csModule = CreateComputeShaderModule();
    wgpu::BindGroupLayout bglayout = CreateComputeBindGroupLayout();
    wgpu::PipelineLayout pipelineLayout = utils::MakePipelineLayout(device, {bglayout});

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = csModule;
    csDesc.compute.entryPoint = "main";
    csDesc.layout = pipelineLayout;

    RunInParallel(100, [&, this](uint32_t) {
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);
        EXPECT_NE(nullptr, pipeline.Get());
    });
}

// Test that creating a same bind group layout (which will return the cached layout) and
// release it on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedBindGroupLayoutsInParallel) {
    RunInParallel(100, [&, this](uint32_t) {
        wgpu::BindGroupLayout layout = CreateComputeBindGroupLayout();
        EXPECT_NE(nullptr, layout.Get());
    });
}

// Test that creating a same pipeline layout (which will return the cached layout) and
// release it on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedPipelineLayoutsInParallel) {
    wgpu::BindGroupLayout bglayout = CreateComputeBindGroupLayout();

    RunInParallel(100, [&, this](uint32_t) {
        wgpu::PipelineLayout pipelineLayout = utils::MakePipelineLayout(device, {bglayout});
        EXPECT_NE(nullptr, pipelineLayout.Get());
    });
}

// Test that creating a same render pipeline (which will return the cached pipeline) and release it
// on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedRenderPipelinesInParallel) {
    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");
    renderPipelineDescriptor.vertex.module = vsModule;
    renderPipelineDescriptor.cFragment.module = fsModule;
    renderPipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

    RunInParallel(100, [&, this](uint32_t) {
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);
        EXPECT_NE(nullptr, pipeline.Get());
    });
}

// Test that creating a same sampler pipeline (which will return the cached sampler) and release it
// on multiple threads won't race.
TEST_P(MultithreadCachingTests, RefAndReleaseCachedSamplersInParallel) {
    wgpu::SamplerDescriptor desc = {};
    RunInParallel(100, [&, this](uint32_t) {
        wgpu::Sampler sampler = device.CreateSampler(&desc);
        EXPECT_NE(nullptr, sampler.Get());
    });
}

class MultithreadEncodingTests : public MultithreadTests {};

// Test that encoding render passes in parallel should work
TEST_P(MultithreadEncodingTests, RenderPassEncodersInParallel) {
    constexpr uint32_t kRTSize = 16;
    constexpr uint32_t kNumThreads = 10;

    wgpu::Texture msaaRenderTarget =
        CreateTexture(kRTSize, kRTSize, wgpu::TextureFormat::RGBA8Unorm,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc,
                      /*mipLevelCount=*/1, /*sampleCount=*/4);
    wgpu::TextureView msaaRenderTargetView = msaaRenderTarget.CreateView();

    wgpu::Texture resolveTarget =
        CreateTexture(kRTSize, kRTSize, wgpu::TextureFormat::RGBA8Unorm,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);
    wgpu::TextureView resolveTargetView = resolveTarget.CreateView();

    std::vector<wgpu::CommandBuffer> commandBuffers(kNumThreads);

    RunInParallel(kNumThreads, [=, &commandBuffers](uint32_t index) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // Clear the renderTarget to red.
        utils::ComboRenderPassDescriptor renderPass({msaaRenderTargetView});
        renderPass.cColorAttachments[0].resolveTarget = resolveTargetView;
        renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();

        commandBuffers[index] = encoder.Finish();
    });

    // Verify that the command buffers executed correctly.
    for (auto& commandBuffer : commandBuffers) {
        queue.Submit(1, &commandBuffer);

        EXPECT_TEXTURE_EQ(utils::RGBA8::kRed, resolveTarget, {0, 0});
        EXPECT_TEXTURE_EQ(utils::RGBA8::kRed, resolveTarget, {kRTSize - 1, kRTSize - 1});
    }
}

// Test that encoding compute passes in parallel should work
TEST_P(MultithreadEncodingTests, ComputePassEncodersInParallel) {
    constexpr uint32_t kNumThreads = 10;
    constexpr uint32_t kExpected = 0xFFFFFFFFu;

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var<storage, read_write> output : u32;

            @compute @workgroup_size(1, 1, 1)
            fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
                output = 0xFFFFFFFFu;
            })");
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    csDesc.compute.entryPoint = "main";
    auto pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::Buffer dstBuffer =
        CreateBuffer(sizeof(uint32_t), wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc |
                                           wgpu::BufferUsage::CopyDst);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, dstBuffer, 0, sizeof(uint32_t)},
                                                     });

    std::vector<wgpu::CommandBuffer> commandBuffers(kNumThreads);

    RunInParallel(kNumThreads, [=, &commandBuffers](uint32_t index) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1, 1, 1);
        pass.End();

        commandBuffers[index] = encoder.Finish();
    });

    // Verify that the command buffers executed correctly.
    for (auto& commandBuffer : commandBuffers) {
        constexpr uint32_t kSentinelData = 0;
        queue.WriteBuffer(dstBuffer, 0, &kSentinelData, sizeof(kSentinelData));
        queue.Submit(1, &commandBuffer);

        EXPECT_BUFFER_U32_EQ(kExpected, dstBuffer, 0);
    }
}

class MultithreadDrawIndexedIndirectTests : public MultithreadTests {
  protected:
    void SetUp() override {
        MultithreadTests::SetUp();

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@location(0) pos : vec4f) -> @builtin(position) vec4f {
                return pos;
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
        descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Uint32;
        descriptor.vertex.bufferCount = 1;
        descriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
        descriptor.cBuffers[0].attributeCount = 1;
        descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
        descriptor.cTargets[0].format = utils::BasicRenderPass::kDefaultColorFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);

        vertexBuffer = utils::CreateBufferFromData<float>(
            device, wgpu::BufferUsage::Vertex,
            {// First quad: the first 3 vertices represent the bottom left triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
             0.0f, 1.0f,

             // Second quad: the first 3 vertices represent the top right triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f,
             0.0f, 1.0f});
    }

    void Test(std::initializer_list<uint32_t> bufferList,
              uint64_t indexOffset,
              uint64_t indirectOffset,
              utils::RGBA8 bottomLeftExpected,
              utils::RGBA8 topRightExpected) {
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
            device, kRTSize, kRTSize, utils::BasicRenderPass::kDefaultColorFormat);
        wgpu::Buffer indexBuffer =
            CreateIndexBuffer({0, 1, 2, 0, 3, 1,
                               // The indices below are added to test negatve baseVertex
                               0 + 4, 1 + 4, 2 + 4, 0 + 4, 3 + 4, 1 + 4});
        TestDraw(
            renderPass, bottomLeftExpected, topRightExpected,
            EncodeDrawCommands(bufferList, indexBuffer, indexOffset, indirectOffset, renderPass));
    }

  private:
    wgpu::Buffer CreateIndirectBuffer(std::initializer_list<uint32_t> indirectParamList) {
        return utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Indirect | wgpu::BufferUsage::Storage, indirectParamList);
    }

    wgpu::Buffer CreateIndexBuffer(std::initializer_list<uint32_t> indexList) {
        return utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, indexList);
    }

    wgpu::CommandBuffer EncodeDrawCommands(std::initializer_list<uint32_t> bufferList,
                                           wgpu::Buffer indexBuffer,
                                           uint64_t indexOffset,
                                           uint64_t indirectOffset,
                                           const utils::BasicRenderPass& renderPass) {
        wgpu::Buffer indirectBuffer = CreateIndirectBuffer(bufferList);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, indexOffset);
            pass.DrawIndexedIndirect(indirectBuffer, indirectOffset);
            pass.End();
        }

        return encoder.Finish();
    }

    void TestDraw(const utils::BasicRenderPass& renderPass,
                  utils::RGBA8 bottomLeftExpected,
                  utils::RGBA8 topRightExpected,
                  wgpu::CommandBuffer commands) {
        queue.Submit(1, &commands);

        LOCKED_CMD(EXPECT_PIXEL_RGBA8_EQ(bottomLeftExpected, renderPass.color, 1, 3));
        LOCKED_CMD(EXPECT_PIXEL_RGBA8_EQ(topRightExpected, renderPass.color, 3, 1));
    }

    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;
    static constexpr uint32_t kRTSize = 4;
};

// Test indirect draws with offsets on multiple threads.
TEST_P(MultithreadDrawIndexedIndirectTests, IndirectOffsetInParallel) {
    // TODO(crbug.com/dawn/789): Test is failing after a roll on SwANGLE on Windows only.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    // TODO(crbug.com/dawn/1292): Some Intel OpenGL drivers don't seem to like
    // the offsets that Tint/GLSL produces.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGL() && IsLinux());

    utils::RGBA8 filled(0, 255, 0, 255);
    utils::RGBA8 notFilled(0, 0, 0, 0);

    RunInParallel(10, [=](uint32_t) {
        // Test an offset draw call, with indirect buffer containing 2 calls:
        // 1) first 3 indices of the second quad (top right triangle)
        // 2) last 3 indices of the second quad

        // Test #1 (no offset)
        Test({3, 1, 0, 4, 0, 3, 1, 3, 4, 0}, 0, 0, notFilled, filled);

        // Offset to draw #2
        Test({3, 1, 0, 4, 0, 3, 1, 3, 4, 0}, 0, 5 * sizeof(uint32_t), filled, notFilled);
    });
}

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

class MultithreadTimestampQueryTests : public MultithreadTests {
  protected:
    void SetUp() override {
        MultithreadTests::SetUp();

        // Skip all tests if timestamp feature is not supported
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::TimestampQuery}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = MultithreadTests::GetRequiredFeatures();
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

    wgpu::Buffer CreateResolveBuffer(uint64_t size) {
        return CreateBuffer(size, /*usage=*/wgpu::BufferUsage::QueryResolve |
                                      wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);
    }
};

// Test resolving timestamp queries on multiple threads. ResolveQuerySet() will create temp
// resources internally so we need to make sure they are thread safe.
TEST_P(MultithreadTimestampQueryTests, ResolveQuerySets_InParallel) {
    constexpr uint32_t kQueryCount = 2;
    constexpr uint32_t kNumThreads = 10;

    std::vector<wgpu::QuerySet> querySets(kNumThreads);
    std::vector<wgpu::Buffer> destinations(kNumThreads);

    for (size_t i = 0; i < kNumThreads; ++i) {
        querySets[i] = CreateQuerySetForTimestamp(kQueryCount);
        destinations[i] = CreateResolveBuffer(kQueryCount * sizeof(uint64_t));
    }

    RunInParallel(kNumThreads, [&](uint32_t index) {
        const auto& querySet = querySets[index];
        const auto& destination = destinations[index];
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        encoder.WriteTimestamp(querySet, 1);
        encoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        LOCKED_CMD(EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t),
                                 new TimestampExpectation));
    });
}

}  // namespace

DAWN_INSTANTIATE_TEST(MultithreadCachingTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

DAWN_INSTANTIATE_TEST(MultithreadEncodingTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

DAWN_INSTANTIATE_TEST(MultithreadDrawIndexedIndirectTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

DAWN_INSTANTIATE_TEST(MultithreadTimestampQueryTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
