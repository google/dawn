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

#include <cmath>

class DeprecationTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        // Skip when validation is off because warnings might be emitted during validation calls
        DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));
    }
};

// Test that using size=0 to indicate default size in setVertexBuffer and setIndexBuffer is
// deprecated.
TEST_P(DeprecationTests, SetBufferWithZeroSizeAsDefault) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 128;
    bufferDesc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::Vertex;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass;

    {
        // Control case, use wgpu::kWholeSize to indicate default size.
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 0, wgpu::kWholeSize);
        pass.SetVertexBuffer(0, buffer, 0, wgpu::kWholeSize);
        pass.EndPass();
    }

    {
        // Control case, omitting size parameter to indicate default size.
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 0);
        pass.SetVertexBuffer(0, buffer, 0);
        pass.EndPass();
    }

    {
        // Deprecated case, use 0 to indicate default size will cause deprecated warning.
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        EXPECT_DEPRECATION_WARNING(pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 0, 0));
        EXPECT_DEPRECATION_WARNING(pass.SetVertexBuffer(0, buffer, 0, 0));
        pass.EndPass();
    }
}

// Test that using size=0 to indicate default size in mapAsync of buffer is
// deprecated.
TEST_P(DeprecationTests, BufferMapAsyncWithZeroSizeAsDefault) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 128;
    bufferDesc.usage = wgpu::BufferUsage::MapWrite;

    {
        // Control case, use wgpu::kWholeMapSize to indicate default size.
        wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

        buffer.MapAsync(wgpu::MapMode::Write, 0, wgpu::kWholeMapSize, nullptr, nullptr);

        WaitForAllOperations();
    }

    {
        // Deprecated case, use 0 to indicate default size will cause deprecated warning.
        wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

        EXPECT_DEPRECATION_WARNING(buffer.MapAsync(wgpu::MapMode::Write, 0, 0, nullptr, nullptr));

        WaitForAllOperations();
    }
}

DAWN_INSTANTIATE_TEST(DeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
