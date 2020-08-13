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

DAWN_INSTANTIATE_TEST(DeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      VulkanBackend());

class BufferCopyViewDeprecationTests : public DeprecationTests {
  protected:
    wgpu::TextureCopyView MakeTextureCopyView() {
        wgpu::TextureDescriptor desc = {};
        desc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.size = {1, 1, 2};
        desc.format = wgpu::TextureFormat::RGBA8Unorm;

        wgpu::TextureCopyView copy;
        copy.texture = device.CreateTexture(&desc);
        copy.origin = {0, 0, 1};
        return copy;
    }

    wgpu::Extent3D copySize = {1, 1, 1};
};

// Test that using BufferCopyView::{offset,bytesPerRow,rowsPerImage} emits a warning.
TEST_P(BufferCopyViewDeprecationTests, DeprecationWarning) {
    wgpu::BufferDescriptor desc;
    desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    desc.size = 8;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    wgpu::TextureCopyView texCopy = MakeTextureCopyView();

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::BufferCopyView bufCopy = {};
        bufCopy.buffer = buffer;
        bufCopy.offset = 4;
        EXPECT_DEPRECATION_WARNING(encoder.CopyBufferToTexture(&bufCopy, &texCopy, &copySize));
        EXPECT_DEPRECATION_WARNING(encoder.CopyTextureToBuffer(&texCopy, &bufCopy, &copySize));
        wgpu::CommandBuffer command = encoder.Finish();
        queue.Submit(1, &command);
    }
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::BufferCopyView bufCopy = {};
        bufCopy.buffer = buffer;
        bufCopy.bytesPerRow = 128;
        EXPECT_DEPRECATION_WARNING(encoder.CopyBufferToTexture(&bufCopy, &texCopy, &copySize));
        EXPECT_DEPRECATION_WARNING(encoder.CopyTextureToBuffer(&texCopy, &bufCopy, &copySize));
        // Since bytesPerRow is not 256-aligned.
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::BufferCopyView bufCopy = {};
        bufCopy.buffer = buffer;
        bufCopy.rowsPerImage = 1;
        EXPECT_DEPRECATION_WARNING(encoder.CopyBufferToTexture(&bufCopy, &texCopy, &copySize));
        EXPECT_DEPRECATION_WARNING(encoder.CopyTextureToBuffer(&texCopy, &bufCopy, &copySize));
        wgpu::CommandBuffer command = encoder.Finish();
        queue.Submit(1, &command);
    }
}

// Test that using both any old field and any new field is an error
TEST_P(BufferCopyViewDeprecationTests, BothOldAndNew) {
    wgpu::BufferDescriptor desc;
    desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    desc.size = 8;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    wgpu::TextureCopyView texCopy = MakeTextureCopyView();

    auto testOne = [=](const wgpu::BufferCopyView& bufCopy) {
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToTexture(&bufCopy, &texCopy, &copySize);
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyTextureToBuffer(&texCopy, &bufCopy, &copySize);
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    };

    {
        wgpu::BufferCopyView bufCopy = {};
        bufCopy.buffer = buffer;
        bufCopy.layout.bytesPerRow = kTextureBytesPerRowAlignment;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToTexture(&bufCopy, &texCopy, &copySize);
            encoder.CopyTextureToBuffer(&texCopy, &bufCopy, &copySize);
            wgpu::CommandBuffer command = encoder.Finish();
            queue.Submit(1, &command);
        }

        bufCopy.offset = 4;
        testOne(bufCopy);
        bufCopy.offset = 0;
        bufCopy.bytesPerRow = kTextureBytesPerRowAlignment;
        testOne(bufCopy);
        bufCopy.bytesPerRow = 0;
        bufCopy.rowsPerImage = 1;
        testOne(bufCopy);
    }
}

DAWN_INSTANTIATE_TEST(BufferCopyViewDeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      VulkanBackend());
