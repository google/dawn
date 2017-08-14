// Copyright 2017 The NXT Authors
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

class FramebufferValidationTest : public ValidationTest {
    protected:
        nxt::TextureView Create2DAttachment(uint32_t width, uint32_t height, nxt::TextureFormat format) {
            nxt::Texture attachment = device.CreateTextureBuilder()
                .SetDimension(nxt::TextureDimension::e2D)
                .SetExtent(width, height, 1)
                .SetFormat(format)
                .SetMipLevels(1)
                .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment)
                .SetInitialUsage(nxt::TextureUsageBit::OutputAttachment)
                .GetResult();

            return attachment.CreateTextureViewBuilder()
                .GetResult();
        }

        nxt::Framebuffer CreateFramebufferWithAttachment(uint32_t width, uint32_t height, nxt::TextureFormat format) {
            auto renderpass = AssertWillBeSuccess(device.CreateRenderPassBuilder())
                .SetAttachmentCount(1)
                .AttachmentSetFormat(0, format)
                .SetSubpassCount(1)
                .SubpassSetColorAttachment(0, 0, 0)
                .GetResult();
            nxt::TextureView attachment = Create2DAttachment(width, height, format);
            return AssertWillBeSuccess(device.CreateFramebufferBuilder())
                .SetRenderPass(renderpass)
                .SetAttachment(0, attachment)
                .SetDimensions(100, 100)
                .GetResult();
        }
};

// Test for an empty framebuffer builder
TEST_F(FramebufferValidationTest, Empty) {
    auto framebuffer = AssertWillBeError(device.CreateFramebufferBuilder())
        .GetResult();
}

// Tests for null arguments to a framebuffer builder
TEST_F(FramebufferValidationTest, NullArguments) {
    AssertWillBeError(device.CreateFramebufferBuilder())
        .SetRenderPass(nullptr)
        .GetResult();
}

// Tests for passing error-valued arguments to a framebuffer builder
TEST_F(FramebufferValidationTest, ErrorValues) {
    auto renderpass = AssertWillBeError(device.CreateRenderPassBuilder())
        .GetResult();
    AssertWillBeError(device.CreateFramebufferBuilder())
        .SetRenderPass(renderpass)
        .GetResult();
}

// Tests for basic framebuffer construction
TEST_F(FramebufferValidationTest, Basic) {
    auto renderpass = AssertWillBeSuccess(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(0)
        .GetResult();
    auto framebuffer = AssertWillBeSuccess(device.CreateFramebufferBuilder())
        .SetRenderPass(renderpass)
        .SetDimensions(100, 100)
        .GetResult();
}

// Tests for framebuffer construction with an (empty) attachment
TEST_F(FramebufferValidationTest, BasicWithEmptyAttachment) {
    auto renderpass = AssertWillBeSuccess(device.CreateRenderPassBuilder())
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .SetSubpassCount(1)
        .SubpassSetColorAttachment(0, 0, 0)
        .GetResult();
    AssertWillBeError(device.CreateFramebufferBuilder())
        .SetRenderPass(renderpass)
        .SetDimensions(100, 100)
        .GetResult();
}

// Test for a basic framebuffer with one attachment
TEST_F(FramebufferValidationTest, BasicWithOneAttachment) {
    CreateFramebufferWithAttachment(100, 100, nxt::TextureFormat::R8G8B8A8Unorm);
}

// Tests for setting clear values
TEST_F(FramebufferValidationTest, ClearValues) {
    auto framebuffer = CreateFramebufferWithAttachment(100, 100, nxt::TextureFormat::R8G8B8A8Unorm);

    // Set clear color value for attachment 0
    framebuffer.AttachmentSetClearColor(0, 0.0f, 0.0f, 0.0f, 0.0f);
    // Set clear depth/stencil values for attachment 0 - ok, but ignored, since it's a color attachment
    framebuffer.AttachmentSetClearDepthStencil(0, 0.0f, 0);
    // Set clear color value for attachment 1 - should fail
    ASSERT_DEVICE_ERROR(framebuffer.AttachmentSetClearColor(1, 0.0f, 0.0f, 0.0f, 0.0f));
}

// Check validation that the attachment size must be the same as the framebuffer size.
// TODO(cwallez@chromium.org): Investigate this constraint more, for example Vulkan requires
// that the attachment sizes are *at least* the framebuffer size.
TEST_F(FramebufferValidationTest, AttachmentSizeMatchFramebufferSize) {
    auto renderpass = AssertWillBeSuccess(device.CreateRenderPassBuilder())
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .SetSubpassCount(1)
        .SubpassSetColorAttachment(0, 0, 0)
        .GetResult();

    nxt::TextureView attachment = Create2DAttachment(100, 100, nxt::TextureFormat::R8G8B8A8Unorm);

    // Control case: two attachments of the same size
    {
        auto framebuffer = AssertWillBeSuccess(device.CreateFramebufferBuilder())
            .SetRenderPass(renderpass)
            .SetAttachment(0, attachment)
            .SetDimensions(100, 100)
            .GetResult();
    }

    // Error: case, size mismatch (framebuffer bigger than attachments)
    {
        auto framebuffer = AssertWillBeError(device.CreateFramebufferBuilder())
            .SetRenderPass(renderpass)
            .SetAttachment(0, attachment)
            .SetDimensions(200, 200)
            .GetResult();
    }

    // Error: case, size mismatch (framebuffer smaller than attachments)
    {
        auto framebuffer = AssertWillBeError(device.CreateFramebufferBuilder())
            .SetRenderPass(renderpass)
            .SetAttachment(0, attachment)
            .SetDimensions(50, 50)
            .GetResult();
    }

    // TODO(cwallez@chromium.org): also test with a mismatches depth / stencil
}

TEST_F(FramebufferValidationTest, AttachmentFormatMatchTextureFormat) {
    // Control case: attach color attachment to color slot
    {
        auto renderpass = AssertWillBeSuccess(device.CreateRenderPassBuilder())
            .SetAttachmentCount(1)
            .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
            .SetSubpassCount(1)
            .SubpassSetColorAttachment(0, 0, 0)
            .GetResult();

        nxt::TextureView attachment = Create2DAttachment(100, 100, nxt::TextureFormat::R8G8B8A8Unorm);
        auto framebuffer = AssertWillBeSuccess(device.CreateFramebufferBuilder())
            .SetRenderPass(renderpass)
            .SetAttachment(0, attachment)
            .SetDimensions(100, 100)
            .GetResult();
    }

    // Error: attach color attachment to depth slot, setting the attachment format first
    {
        auto renderpass = AssertWillBeSuccess(device.CreateRenderPassBuilder())
            .SetAttachmentCount(1)
            .SetSubpassCount(1)
            .AttachmentSetFormat(0, nxt::TextureFormat::D32FloatS8Uint)
            .SubpassSetDepthStencilAttachment(0, 0)
            .GetResult();

        nxt::TextureView attachment = Create2DAttachment(100, 100, nxt::TextureFormat::R8G8B8A8Unorm);
        auto framebuffer = AssertWillBeError(device.CreateFramebufferBuilder())
            .SetRenderPass(renderpass)
            .SetAttachment(0, attachment)
            .SetDimensions(100, 100)
            .GetResult();
    }

    // Error: attach color attachment to depth slot, but setting the attachment format last
    {
        auto renderpass = AssertWillBeSuccess(device.CreateRenderPassBuilder())
            .SetSubpassCount(1)
            .SetAttachmentCount(1)
            .SubpassSetDepthStencilAttachment(0, 0)
            .AttachmentSetFormat(0, nxt::TextureFormat::D32FloatS8Uint)
            .GetResult();

        nxt::TextureView attachment = Create2DAttachment(100, 100, nxt::TextureFormat::R8G8B8A8Unorm);
        auto framebuffer = AssertWillBeError(device.CreateFramebufferBuilder())
            .SetRenderPass(renderpass)
            .SetAttachment(0, attachment)
            .SetDimensions(100, 100)
            .GetResult();
    }

    // Error: attach depth texture to color slot
    {
        auto renderpass = AssertWillBeSuccess(device.CreateRenderPassBuilder())
            .SetAttachmentCount(1)
            .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
            .SetSubpassCount(1)
            .SubpassSetColorAttachment(0, 0, 0)
            .GetResult();

        nxt::TextureView attachment = Create2DAttachment(100, 100, nxt::TextureFormat::D32FloatS8Uint);
        auto framebuffer = AssertWillBeError(device.CreateFramebufferBuilder())
            .SetRenderPass(renderpass)
            .SetAttachment(0, attachment)
            .SetDimensions(100, 100)
            .GetResult();
    }

    // TODO(kainino@chromium.org): also check attachment samples, etc.
}
