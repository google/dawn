// Copyright 2018 The Dawn Authors
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

#include "tests/unittests/validation/ValidationTest.h"

#include "utils/WGPUHelpers.h"

#include <cmath>

class SetViewportTest : public ValidationTest {
  protected:
    void TestViewportCall(bool success,
                          float x,
                          float y,
                          float width,
                          float height,
                          float minDepth,
                          float maxDepth) {
        utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, kWidth, kHeight);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetViewport(x, y, width, height, minDepth, maxDepth);
        pass.EndPass();

        if (success) {
            encoder.Finish();
        } else {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }

    static constexpr uint32_t kWidth = 5;
    static constexpr uint32_t kHeight = 3;
};

// Test to check basic use of SetViewport
TEST_F(SetViewportTest, Success) {
    TestViewportCall(true, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0);
}

// Test to check that NaN in viewport parameters is not allowed
TEST_F(SetViewportTest, ViewportParameterNaN) {
    TestViewportCall(false, NAN, 0.0, 1.0, 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, NAN, 1.0, 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, NAN, 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, NAN, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, NAN, 1.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, 0.0, NAN);
}

// Test to check that an empty viewport is allowed.
TEST_F(SetViewportTest, EmptyViewport) {
    // Width of viewport is zero.
    TestViewportCall(true, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0);

    // Height of viewport is zero.
    TestViewportCall(true, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0);

    // Both width and height of viewport are zero.
    TestViewportCall(true, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
}

// Test to check that viewport larger than the framebuffer is disallowed
TEST_F(SetViewportTest, ViewportLargerThanFramebuffer) {
    // Control case: width and height are set to the render target size.
    TestViewportCall(true, 0.0, 0.0, kWidth, kHeight, 0.0, 1.0);

    // Width is larger than the rendertarget's width
    TestViewportCall(false, 0.0, 0.0, kWidth + 1.0, kHeight, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, nextafter(float(kWidth), 1000.0f), kHeight, 0.0, 1.0);

    // Height is larger than the rendertarget's height
    TestViewportCall(false, 0.0, 0.0, kWidth, kHeight + 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, kWidth, nextafter(float(kHeight), 1000.0f), 0.0, 1.0);

    // x + width is larger than the rendertarget's width
    TestViewportCall(false, 2.0, 0.0, kWidth - 1.0, kHeight, 0.0, 1.0);
    TestViewportCall(false, 1.0, 0.0, nextafter(float(kWidth - 1.0), 1000.0f), kHeight, 0.0, 1.0);

    // Height is larger than the rendertarget's height
    TestViewportCall(false, 0.0, 2.0, kWidth, kHeight - 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, 1.0, kWidth, nextafter(float(kHeight - 1.0), 1000.0f), 0.0, 1.0);
}

// Test to check that negative x in viewport is disallowed
TEST_F(SetViewportTest, NegativeXYWidthHeight) {
    // Control case: everything set to 0 is allowed.
    TestViewportCall(true, +0.0, +0.0, +0.0, +0.0, 0.0, 1.0);
    TestViewportCall(true, -0.0, -0.0, -0.0, -0.0, 0.0, 1.0);

    // Nonzero negative values are disallowed
    TestViewportCall(false, -1.0, 0.0, 1.0, 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, -1.0, 1.0, 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, -1.0, 1.0, 0.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, -1.0, 0.0, 1.0);
}

// Test to check that minDepth out of range [0, 1] is disallowed
TEST_F(SetViewportTest, MinDepthOutOfRange) {
    // MinDepth is -1
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, -1.0, 1.0);

    // MinDepth is 2 or 1 + epsilon
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, 2.0, 1.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, nextafter(1.0f, 1000.0f), 1.0);
}

// Test to check that minDepth out of range [0, 1] is disallowed
TEST_F(SetViewportTest, MaxDepthOutOfRange) {
    // MaxDepth is -1
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, 1.0, -1.0);

    // MaxDepth is 2 or 1 + epsilon
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, 1.0, 2.0);
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, 1.0, nextafter(1.0f, 1000.0f));
}

// Test to check that minDepth equal or greater than maxDepth is disallowed
TEST_F(SetViewportTest, MinDepthEqualOrGreaterThanMaxDepth) {
    TestViewportCall(true, 0.0, 0.0, 1.0, 1.0, 0.5, 0.5);
    TestViewportCall(false, 0.0, 0.0, 1.0, 1.0, 0.8, 0.5);
}

class SetScissorRectTest : public ValidationTest {};

// Test to check basic use of SetScissor
TEST_F(SetScissorRectTest, Success) {
    DummyRenderPass renderPass(device);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetScissorRect(0, 0, 1, 1);
        pass.EndPass();
    }
    encoder.Finish();
}

// Test to check that an empty scissor is not allowed
TEST_F(SetScissorRectTest, EmptyScissor) {
    DummyRenderPass renderPass(device);

    // Width of scissor rect is zero.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetScissorRect(0, 0, 0, 1);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Height of scissor rect is zero.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetScissorRect(0, 0, 1, 0);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Both width and height of scissor rect are zero.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetScissorRect(0, 0, 0, 0);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test to check that a scissor larger than the framebuffer is allowed
TEST_F(SetScissorRectTest, ScissorLargerThanFramebuffer) {
    DummyRenderPass renderPass(device);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetScissorRect(0, 0, renderPass.width + 1, renderPass.height + 1);
        pass.EndPass();
    }
    encoder.Finish();
}

class SetBlendColorTest : public ValidationTest {};

// Test to check basic use of SetBlendColor
TEST_F(SetBlendColorTest, Success) {
    DummyRenderPass renderPass(device);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        constexpr wgpu::Color kTransparentBlack{0.0f, 0.0f, 0.0f, 0.0f};
        pass.SetBlendColor(&kTransparentBlack);
        pass.EndPass();
    }
    encoder.Finish();
}

// Test that SetBlendColor allows any value, large, small or negative
TEST_F(SetBlendColorTest, AnyValueAllowed) {
    DummyRenderPass renderPass(device);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        constexpr wgpu::Color kAnyColorValue{-1.0f, 42.0f, -0.0f, 0.0f};
        pass.SetBlendColor(&kAnyColorValue);
        pass.EndPass();
    }
    encoder.Finish();
}

class SetStencilReferenceTest : public ValidationTest {};

// Test to check basic use of SetStencilReferenceTest
TEST_F(SetStencilReferenceTest, Success) {
    DummyRenderPass renderPass(device);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetStencilReference(0);
        pass.EndPass();
    }
    encoder.Finish();
}

// Test that SetStencilReference allows any bit to be set
TEST_F(SetStencilReferenceTest, AllBitsAllowed) {
    DummyRenderPass renderPass(device);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetStencilReference(0xFFFFFFFF);
        pass.EndPass();
    }
    encoder.Finish();
}
