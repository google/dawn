// Copyright 2019 The Dawn Authors
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
#include "utils/DawnHelpers.h"

class ViewportTest : public DawnTest {
  protected:
    dawn::RenderPipeline CreatePipelineForTest(dawn::CompareFunction depthCompare) {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);

        // Draw two triangles:
        // 1. The top-left triangle is red. Its depth values are >= 0.5. After viewport is applied,
        // the depth might be >= 0.25 if minDepth is 0 and maxDepth is 0.5.
        // 2. The bottom-right triangle is green. Its depth values are <= 0.5. After viewport is
        // applied, the depth might be <= 0.25 if minDepth is 0 and maxDepth is 0.5.
        const char* vs =
            R"(#version 450
            layout(location = 0) out vec4 color;
            const vec3 pos[6] = vec3[6](vec3(-1.0f, -1.0f, 1.0f),
                                        vec3(-1.0f,  1.0f, 0.5f),
                                        vec3( 1.0f, -1.0f, 0.5f),
                                        vec3( 1.0f, -1.0f, 0.5f),
                                        vec3(-1.0f,  1.0f, 0.5f),
                                        vec3( 1.0f,  1.0f, 0.0f));
            void main() {
                gl_Position = vec4(pos[gl_VertexIndex], 1.0);
                if (gl_VertexIndex < 3) {
                    color = vec4(1.0, 0.0, 0.0, 1.0);
                } else {
                    color = vec4(0.0, 1.0, 0.0, 1.0);
                }
            })";
        pipelineDescriptor.cVertexStage.module =
            utils::CreateShaderModule(device, utils::ShaderStage::Vertex, vs);

        const char* fs =
            R"(#version 450
            layout(location = 0) in vec4 color;
            layout(location = 0) out vec4 fragColor;
            void main() {
               fragColor = color;
            })";
        pipelineDescriptor.cFragmentStage.module =
            utils::CreateShaderModule(device, utils::ShaderStage::Fragment, fs);

        pipelineDescriptor.cDepthStencilState.depthCompare = depthCompare;
        pipelineDescriptor.depthStencilState = &pipelineDescriptor.cDepthStencilState;

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    dawn::Texture Create2DTextureForTest(dawn::TextureFormat format) {
        dawn::TextureDescriptor textureDescriptor;
        textureDescriptor.dimension = dawn::TextureDimension::e2D;
        textureDescriptor.format = format;
        textureDescriptor.usage =
            dawn::TextureUsageBit::OutputAttachment | dawn::TextureUsageBit::CopySrc;
        textureDescriptor.arrayLayerCount = 1;
        textureDescriptor.mipLevelCount = 1;
        textureDescriptor.sampleCount = 1;
        textureDescriptor.size = {kSize, kSize, 1};
        return device.CreateTexture(&textureDescriptor);
    }

    enum ColorType {
        TopLeftTriangleColor,
        BottomRightTriangleColor,
        BackgroundColor,

        ColorTypeCount,
    };

    struct ViewportParams {
        float x, y, width, height, minDepth, maxDepth;
    };

    struct TestInfo {
        ViewportParams viewport;
        ColorType topLeftPoint;
        ColorType bottomRightPoint;
        float clearDepth = 1.0f;
        bool setViewport = true;
    };

    void DoTest(const TestInfo& info) {
        dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();

        // Create render targets for 2 render passes.
        dawn::Texture colorTexture1 = Create2DTextureForTest(dawn::TextureFormat::RGBA8Unorm);
        dawn::Texture depthStencilTexture1 =
            Create2DTextureForTest(dawn::TextureFormat::Depth24PlusStencil8);

        dawn::Texture colorTexture2 = Create2DTextureForTest(dawn::TextureFormat::RGBA8Unorm);
        dawn::Texture depthStencilTexture2 =
            Create2DTextureForTest(dawn::TextureFormat::Depth24PlusStencil8);

        // Create render pass 1
        // Note that we may explicitly call SetViewport() in this pass
        {
            utils::ComboRenderPassDescriptor renderPassDescriptor1(
                {colorTexture1.CreateDefaultView()}, depthStencilTexture1.CreateDefaultView());
            renderPassDescriptor1.cColorAttachmentsInfoPtr[0]->clearColor = {0.0, 0.0, 1.0, 1.0};
            renderPassDescriptor1.cColorAttachmentsInfoPtr[0]->loadOp = dawn::LoadOp::Clear;

            renderPassDescriptor1.cDepthStencilAttachmentInfo.clearDepth = info.clearDepth;
            renderPassDescriptor1.cDepthStencilAttachmentInfo.depthLoadOp = dawn::LoadOp::Clear;

            dawn::RenderPassEncoder renderPass1 =
                commandEncoder.BeginRenderPass(&renderPassDescriptor1);
            renderPass1.SetPipeline(CreatePipelineForTest(dawn::CompareFunction::Less));
            if (info.setViewport) {
                ViewportParams viewport = info.viewport;
                renderPass1.SetViewport(viewport.x, viewport.y, viewport.width, viewport.height,
                                        viewport.minDepth, viewport.maxDepth);
            }
            renderPass1.Draw(6, 1, 0, 0);
            renderPass1.EndPass();
        }

        // Create render pass 2
        // Note that we never explicitly call SetViewport() in this pass.
        // Its viewport(x, y, width, height, minDepth, maxDepth) should be
        // (0, 0, rendertarget's width, rendertarget's height, 0.0, 1.0) by default.
        {
            utils::ComboRenderPassDescriptor renderPassDescriptor2(
                {colorTexture2.CreateDefaultView()}, depthStencilTexture2.CreateDefaultView());
            renderPassDescriptor2.cColorAttachmentsInfoPtr[0]->clearColor = {0.0, 0.0, 1.0, 1.0};
            renderPassDescriptor2.cColorAttachmentsInfoPtr[0]->loadOp = dawn::LoadOp::Clear;

            renderPassDescriptor2.cDepthStencilAttachmentInfo.clearDepth = 0.5;
            renderPassDescriptor2.cDepthStencilAttachmentInfo.depthLoadOp = dawn::LoadOp::Clear;

            dawn::RenderPassEncoder renderPass2 =
                commandEncoder.BeginRenderPass(&renderPassDescriptor2);
            renderPass2.SetPipeline(CreatePipelineForTest(dawn::CompareFunction::Greater));
            renderPass2.Draw(6, 1, 0, 0);
            renderPass2.EndPass();
        }

        dawn::CommandBuffer commandBuffer = commandEncoder.Finish();
        dawn::Queue queue = device.CreateQueue();
        queue.Submit(1, &commandBuffer);

        constexpr RGBA8 kColor[ColorTypeCount] = {
            RGBA8(255, 0, 0, 255),  // top-left triangle is red
            RGBA8(0, 255, 0, 255),  // bottom-right triangle is green
            RGBA8(0, 0, 255, 255),  // background is blue
        };

        EXPECT_PIXEL_RGBA8_EQ(kColor[info.topLeftPoint], colorTexture1, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(kColor[info.bottomRightPoint], colorTexture1, kSize - 1, kSize - 1);

        // In render pass 2. Point(0, 0) is tend to be covered by the top-left triangle. Point(3, 3)
        // is tend to be covered by the bottom-right triangle. However, the bottom-right triangle's
        // depth values are <= 0.5. And the depthCompare is Greater. As a result, point(0, 0) will
        // be drawn as usual, its color is the top-left triangle's color. But point(3, 3) will not
        // be drawn by any triangles. Its color is the backgroud color.
        EXPECT_PIXEL_RGBA8_EQ(kColor[TopLeftTriangleColor], colorTexture2, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(kColor[BackgroundColor], colorTexture2, kSize - 1, kSize - 1);
    }

    static constexpr uint32_t kSize = 4;
};

// The viewport is the same size as the backbuffer if it is not explicitly specified. And minDepth
// and maxDepth are 0.0 and 1.0 respectively. The viewport parameters below are not really used.
// Point(0, 0) is covered by the top-left triangle. Likewise, point(3, 3) is covered by the
// bottom-right triangle.
TEST_P(ViewportTest, Default) {
    ViewportParams viewport = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    TestInfo info = {viewport, TopLeftTriangleColor, BottomRightTriangleColor, 1.0, false};
    DoTest(info);
}

// Explicitly specify the viewport as its default value. The result is the same as it is in the test
// above.
TEST_P(ViewportTest, Basic) {
    ViewportParams viewport = {0.0, 0.0, 4.0, 4.0, 0.0, 1.0};
    TestInfo info = {viewport, TopLeftTriangleColor, BottomRightTriangleColor};
    DoTest(info);
}

// Shift the viewport toward top-left by (2, 2). So the top-left triangle is outside of the back
// buffer. We can't see it. And point(0, 0) is covered by the bottom-right triangle now. Point(3, 3)
// is not covered by any triangles.
TEST_P(ViewportTest, ShiftToTopLeft) {
    ViewportParams viewport = {-2.0, -2.0, 4.0, 4.0, 0.0, 1.0};
    TestInfo info = {viewport, BottomRightTriangleColor, BackgroundColor};
    DoTest(info);
}

// Shift the viewport toward bottom-right by (2, 2). So Point(0, 0) is not covered by any triangles.
// The top-left triangle is moved to the bottom-right of back buffer. Point(3, 3) is covered by it.
// While the bottom-right triangle is moved outside of back buffer now.
TEST_P(ViewportTest, ShiftToBottomRight) {
    ViewportParams viewport = {2.0, 2.0, 4.0, 4.0, 0.0, 1.0};
    TestInfo info = {viewport, BackgroundColor, TopLeftTriangleColor};
    DoTest(info);
}

// After applying the minDepth/maxDepth value in viewport and projecting to framebuffer coordinate,
// depth values of the top-left triangle are >= 0.25. They are greater than the depth values in
// depth buffer, so it is not drawn at all. As a result, point(0, 0) is not covered by any
// triangles. But the bottom-right triangle is drawn as usual.
TEST_P(ViewportTest, ApplyDepth) {
    ViewportParams viewport = {0.0, 0.0, 4.0, 4.0, 0.0, 0.5};
    TestInfo info = {viewport, BackgroundColor, BottomRightTriangleColor, 0.25};
    DoTest(info);
}

// Shift the viewport toward top-left by (2, 2). So the top-left triangle is outside of the back
// buffer. We can't see it. And point(0, 0) is covered by the bottom-right triangle now. Its depth
// value is < 0.25. So it is drawn as usual. Point(3, 3) is not covered by any triangles.
TEST_P(ViewportTest, ShiftToTopLeftAndApplyDepth) {
    // Test failing on Linux Vulkan Intel.
    // See https://bugs.chromium.org/p/dawn/issues/detail?id=187
    DAWN_SKIP_TEST_IF(IsLinux() && IsVulkan() && IsIntel());

    ViewportParams viewport = {-2.0, -2.0, 4.0, 4.0, 0.0, 0.5};
    TestInfo info = {viewport, BottomRightTriangleColor, BackgroundColor, 0.25};
    DoTest(info);
}

// Shift the viewport toward bottom-right by (2, 2). So point(0, 0) is not covered by any triangles.
// The top-left triangle is moved to the bottom-right of back buffer. However, depth values of the
// top-left triangle are >= 0.25. They are greater than the depth values in depth buffer, so it is
// not drawn at all. So point(3, 3) is not covered by any triangle, either.
TEST_P(ViewportTest, ShiftToBottomRightAndApplyDepth) {
    ViewportParams viewport = {2.0, 2.0, 4.0, 4.0, 0.0, 0.5};
    TestInfo info = {viewport, BackgroundColor, BackgroundColor, 0.25};
    DoTest(info);
}

// Enlarge the viewport by 2 times. So the entire back buffer is covered by the top-left triangle.
TEST_P(ViewportTest, EnlargeViewport) {
    ViewportParams viewport = {0.0, 0.0, 8.0, 8.0, 0.0, 1.0};
    TestInfo info = {viewport, TopLeftTriangleColor, TopLeftTriangleColor};
    DoTest(info);
}

// Enlarge the viewport by 2 times and shift toward top-left by (2, 2). back buffer sits exactly
// at the center of the whole viewport. So, point(0, 0) is covered by the top-left triangle, and
// point(3, 3) is covered by the bottom-right triangle.
TEST_P(ViewportTest, EnlargeViewportAndShiftToTopLeft) {
    ViewportParams viewport = {-2.0, -2.0, 8.0, 8.0, 0.0, 1.0};
    TestInfo info = {viewport, TopLeftTriangleColor, BottomRightTriangleColor};
    DoTest(info);
}

// Enlarge the viewport by 2 times and shift toward bottom-right by (2, 2). Point(0, 0) is not
// covered by any triangle. Point(3, 3) is covered by the top-left triangle.
TEST_P(ViewportTest, EnlargeViewportAndShiftToBottomRight) {
    ViewportParams viewport = {2.0, 2.0, 8.0, 8.0, 0.0, 1.0};
    TestInfo info = {viewport, BackgroundColor, TopLeftTriangleColor};
    DoTest(info);
}

// Enlarge the viewport by 2 times. So the entire back buffer tend to be covered by the top-left
// triangle. However, depth values of the top-left triangle are >= 0.25. They are greater than the
// depth values in depth buffer, so the top-left triangle is not drawn at all. As a result, neither
// point(0, 0) nor point(3, 3) is covered by any triangles.
TEST_P(ViewportTest, EnlargeViewportAndApplyDepth) {
    ViewportParams viewport = {0.0, 0.0, 8.0, 8.0, 0.0, 0.5};
    TestInfo info = {viewport, BackgroundColor, BackgroundColor, 0.25};
    DoTest(info);
}

// Enlarge the viewport by 2 times and shift toward top-left by (2, 2). The back buffer sits exactly
// at the center of the whole viewport. However, depth values of the top-left triangle are >= 0.25.
// They are greater than the depth values in depth buffer, so the top-left triangle is not drawn at
// all. As a result, point(0, 0) is not covered by it. The bottom-right triangle is drawn because
// its depth values are < 0.25. So point(3, 3) is covered by it as usual.
TEST_P(ViewportTest, EnlargeViewportAndShiftToTopLeftAndApplyDepth) {
    // Test failing on Linux Vulkan Intel.
    // See https://bugs.chromium.org/p/dawn/issues/detail?id=187
    DAWN_SKIP_TEST_IF(IsLinux() && IsVulkan() && IsIntel());

    ViewportParams viewport = {-2.0, -2.0, 8.0, 8.0, 0.0, 0.5};
    TestInfo info = {viewport, BackgroundColor, BottomRightTriangleColor, 0.25};
    DoTest(info);
}

// Enlarge the viewport by 2 times and shift toward bottom-right by (2, 2). Point(0, 0) is not
// covered by any triangle. The point(3, 3) tend to be covered by the top-left triangle. However,
// depth values of the top-left triangle are >= 0.25. They are greater than the depth values in
// depth buffer, so the top-left triangle is not drawn at all. As a result, point(3, 3) is not
// covered by any triangle, either.
TEST_P(ViewportTest, EnlargeViewportAndShiftToBottomRightAndApplyDepth) {
    ViewportParams viewport = {2.0, 2.0, 8.0, 8.0, 0.0, 0.5};
    TestInfo info = {viewport, BackgroundColor, BackgroundColor, 0.25};
    DoTest(info);
}

// Shrink the viewport to its half. So point(0, 0) is covered by the top-left triangle, while
// point(3, 3) is not covered by any triangles because the drawing area is too small to cover the
// entire back buffer.
TEST_P(ViewportTest, ShrinkViewport) {
    ViewportParams viewport = {0.0, 0.0, 2.0, 2.0, 0.0, 1.0};
    TestInfo info = {viewport, TopLeftTriangleColor, BackgroundColor};
    DoTest(info);
}

// Shrink the viewport to its half and move toward top-left by (1, 1), So point(0, 0) is covered by
// bottom-right triangle, while point(3, 3) is not covered by any triangles.
TEST_P(ViewportTest, ShrinkViewportAndShiftToTopLeft) {
    ViewportParams viewport = {-1.0, -1.0, 2.0, 2.0, 0.0, 1.0};
    TestInfo info = {viewport, BottomRightTriangleColor, BackgroundColor};
    DoTest(info);
}

// Shrink the viewport to its half and move toward bottom-right by (3, 3), So point(0, 0) is not
// covered by any triangles, and point(3, 3) is covered by the bottom-right triangle.
TEST_P(ViewportTest, ShrinkViewportAndShiftToBottomRight) {
    ViewportParams viewport = {3.0, 3.0, 2.0, 2.0, 0.0, 1.0};
    TestInfo info = {viewport, BackgroundColor, TopLeftTriangleColor};
    DoTest(info);
}

// Shrink the viewport to its half. So point(0, 0) is tend to be covered by top-left triangle.
// However, depth values of the top-left triangle are >= 0.25. They are greater than the depth
// values in depth buffer, so the top-left triangle is not drawn at all. As a result, point(0, 0)
// is not covered by any triangle. Point(3, 3) is not covered by any triangles, either. Because the
// drawing area is too small to cover the entire back buffer.
TEST_P(ViewportTest, ShrinkViewportAndApplyDepth) {
    ViewportParams viewport = {0.0, 0.0, 2.0, 2.0, 0.0, 0.5};
    TestInfo info = {viewport, BackgroundColor, BackgroundColor, 0.25};
    DoTest(info);
}

// Shrink the viewport to its half and move toward top-left by (1, 1), So point(0, 0) is covered by
// the bottom-right triangle, while point(3, 3) is not covered by any triangles.
TEST_P(ViewportTest, ShrinkViewportAndShiftToTopLeftAndApplyDepth) {
    // Test failing on Linux Vulkan Intel.
    // See https://bugs.chromium.org/p/dawn/issues/detail?id=187
    DAWN_SKIP_TEST_IF(IsLinux() && IsVulkan() && IsIntel());

    ViewportParams viewport = {-1.0, -1.0, 2.0, 2.0, 0.0, 0.5};
    TestInfo info = {viewport, BottomRightTriangleColor, BackgroundColor, 0.25};
    DoTest(info);
}

// Shrink the viewport to its half and move toward bottom-right by (3, 3), So point(0, 0) is not
// covered by any triangle. Point(3, 3) is tend to be covered by the top-left triangle. However,
// depth values of the top-left triangle are >= 0.25. They are greater than the depth values in
// depth buffer, so the top-left triangle is not drawn at all. As a result, point(3, 3) is not
// covered by any triangle, either.
TEST_P(ViewportTest, ShrinkViewportAndShiftToBottomRightAndApplyDepth) {
    ViewportParams viewport = {3.0, 3.0, 2.0, 2.0, 0.0, 0.5};
    TestInfo info = {viewport, BackgroundColor, BackgroundColor, 0.25};
    DoTest(info);
}

DAWN_INSTANTIATE_TEST(ViewportTest, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend);
