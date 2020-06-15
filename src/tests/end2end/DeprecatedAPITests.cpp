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

#include "common/Constants.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class DeprecationTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        // Skip when validation is off because warnings might be emitted during validation calls
        DAWN_SKIP_TEST_IF(IsDawnValidationSkipped());
    }

    void TearDown() override {
        if (!UsesWire()) {
            EXPECT_EQ(mLastWarningCount,
                      dawn_native::GetDeprecationWarningCountForTesting(device.Get()));
        }
        DawnTest::TearDown();
    }

    size_t mLastWarningCount = 0;
};

#define EXPECT_DEPRECATION_WARNING(statement)                                    \
    do {                                                                         \
        if (UsesWire()) {                                                        \
            statement;                                                           \
        } else {                                                                 \
            size_t warningsBefore =                                              \
                dawn_native::GetDeprecationWarningCountForTesting(device.Get()); \
            statement;                                                           \
            size_t warningsAfter =                                               \
                dawn_native::GetDeprecationWarningCountForTesting(device.Get()); \
            EXPECT_EQ(mLastWarningCount, warningsBefore);                        \
            EXPECT_EQ(warningsAfter, warningsBefore + 1);                        \
            mLastWarningCount = warningsAfter;                                   \
        }                                                                        \
    } while (0)

// Test that using SetSubData emits a deprecation warning.
TEST_P(DeprecationTests, SetSubDataDeprecated) {
    wgpu::BufferDescriptor descriptor;
    descriptor.usage = wgpu::BufferUsage::CopyDst;
    descriptor.size = 4;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    EXPECT_DEPRECATION_WARNING(buffer.SetSubData(0, 0, nullptr));
}

// Test that using SetSubData works
TEST_P(DeprecationTests, SetSubDataStillWorks) {
    DAWN_SKIP_TEST_IF(IsNull());

    wgpu::BufferDescriptor descriptor;
    descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    descriptor.size = 4;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t data = 2020;
    EXPECT_DEPRECATION_WARNING(buffer.SetSubData(0, 4, &data));
    EXPECT_BUFFER_U32_EQ(data, buffer, 0);
}

// Test that using TextureDescriptor::arrayLayerCount emits a warning.
TEST_P(DeprecationTests, TextureDescriptorArrayLayerCountDeprecated) {
    wgpu::TextureDescriptor desc;
    desc.usage = wgpu::TextureUsage::Sampled;
    desc.dimension = wgpu::TextureDimension::e2D;
    desc.size = {1, 1, 1};
    desc.arrayLayerCount = 2;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.mipLevelCount = 1;
    desc.sampleCount = 1;

    EXPECT_DEPRECATION_WARNING(device.CreateTexture(&desc));
}

// Test that using both TextureDescriptor::arrayLayerCount and size.depth triggers an error.
TEST_P(DeprecationTests, TextureDescriptorArrayLayerCountAndDepthSizeIsError) {
    wgpu::TextureDescriptor desc;
    desc.usage = wgpu::TextureUsage::Sampled;
    desc.dimension = wgpu::TextureDimension::e2D;
    desc.size = {1, 1, 2};
    desc.arrayLayerCount = 2;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.mipLevelCount = 1;
    desc.sampleCount = 1;

    ASSERT_DEVICE_ERROR(device.CreateTexture(&desc));
}

// Test that TextureDescriptor::arrayLayerCount does correct state tracking.
TEST_P(DeprecationTests, TextureDescriptorArrayLayerCountStateTracking) {
    wgpu::TextureDescriptor desc;
    desc.usage = wgpu::TextureUsage::Sampled;
    desc.dimension = wgpu::TextureDimension::e2D;
    desc.size = {1, 1, 1};
    desc.arrayLayerCount = 2;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.mipLevelCount = 1;
    desc.sampleCount = 1;

    wgpu::Texture texture;
    EXPECT_DEPRECATION_WARNING(texture = device.CreateTexture(&desc));

    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.dimension = wgpu::TextureViewDimension::e2DArray;
    viewDesc.arrayLayerCount = 2;
    texture.CreateView(&viewDesc);
    viewDesc.arrayLayerCount = 3;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));
}

DAWN_INSTANTIATE_TEST(DeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      VulkanBackend());
