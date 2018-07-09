// Copyright 2018 The NXT Authors
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

#include "common/Constants.h"

namespace {

class RenderPassDescriptorValidationTest : public ValidationTest {
};

nxt::TextureView Create2DAttachment(nxt::Device& device, uint32_t width, uint32_t height, nxt::TextureFormat format) {
    nxt::Texture attachment = device.CreateTextureBuilder()
        .SetDimension(nxt::TextureDimension::e2D)
        .SetExtent(width, height, 1)
        .SetFormat(format)
        .SetMipLevels(1)
        .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment)
        .GetResult();

    return attachment.CreateTextureViewBuilder()
        .GetResult();
}

// A render pass with no attachments isn't valid
TEST_F(RenderPassDescriptorValidationTest, Empty) {
    AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
        .GetResult();
}

// A render pass with only one color or one depth attachment is ok
TEST_F(RenderPassDescriptorValidationTest, OneAttachment) {
    // One color attachment
    {
        nxt::TextureView color = Create2DAttachment(device, 1, 1, nxt::TextureFormat::R8G8B8A8Unorm);
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color, nxt::LoadOp::Clear)
            .GetResult();
    }
    // One depth-stencil attachment
    {
        nxt::TextureView depthStencil = Create2DAttachment(device, 1, 1, nxt::TextureFormat::D32FloatS8Uint);
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetDepthStencilAttachment(depthStencil, nxt::LoadOp::Clear, nxt::LoadOp::Clear)
            .GetResult();
    }
}

// Test OOB color attachment indices are handled
TEST_F(RenderPassDescriptorValidationTest, ColorAttachmentOutOfBounds) {
    // For setting the color attachment, control case
    {
        nxt::TextureView color = Create2DAttachment(device, 1, 1, nxt::TextureFormat::R8G8B8A8Unorm);
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(kMaxColorAttachments - 1, color, nxt::LoadOp::Clear)
            .GetResult();
    }
    // For setting the color attachment, OOB
    {
        nxt::TextureView color = Create2DAttachment(device, 1, 1, nxt::TextureFormat::R8G8B8A8Unorm);
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(kMaxColorAttachments, color, nxt::LoadOp::Clear)
            .GetResult();
    }

    nxt::TextureView color = Create2DAttachment(device, 1, 1, nxt::TextureFormat::R8G8B8A8Unorm);
    // For setting the clear color, control case
    {
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color, nxt::LoadOp::Clear)
            .SetColorAttachmentClearColor(kMaxColorAttachments - 1, 0.0f, 0.0f, 0.0f, 0.0f)
            .GetResult();
    }
    // For setting the clear color, OOB
    {
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color, nxt::LoadOp::Clear)
            .SetColorAttachmentClearColor(kMaxColorAttachments, 0.0f, 0.0f, 0.0f, 0.0f)
            .GetResult();
    }
}

// Test setting a clear value without an attachment and vice-versa is ok.
TEST_F(RenderPassDescriptorValidationTest, ClearAndAttachmentMismatchIsOk) {
    nxt::TextureView color = Create2DAttachment(device, 1, 1, nxt::TextureFormat::R8G8B8A8Unorm);

    // For cleared attachment 0 doesn't get a color, clear color for 1 is unused
    {
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color, nxt::LoadOp::Clear)
            .SetColorAttachmentClearColor(1, 0.0f, 0.0f, 0.0f, 0.0f)
            .GetResult();
    }
    // Clear depth stencil doesn't get values
    {
        nxt::TextureView depthStencil = Create2DAttachment(device, 1, 1, nxt::TextureFormat::D32FloatS8Uint);
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetDepthStencilAttachment(depthStencil, nxt::LoadOp::Clear, nxt::LoadOp::Clear)
            .GetResult();
    }
    // Clear values for depth-stencil when it isn't used
    {
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color, nxt::LoadOp::Clear)
            .SetDepthStencilAttachmentClearValue(0.0f, 0)
            .GetResult();
    }
}

// Attachments must have the same size
TEST_F(RenderPassDescriptorValidationTest, SizeMustMatch) {
    nxt::TextureView color1x1A = Create2DAttachment(device, 1, 1, nxt::TextureFormat::R8G8B8A8Unorm);
    nxt::TextureView color1x1B = Create2DAttachment(device, 1, 1, nxt::TextureFormat::R8G8B8A8Unorm);
    nxt::TextureView color2x2 = Create2DAttachment(device, 2, 2, nxt::TextureFormat::R8G8B8A8Unorm);
    nxt::TextureView depthStencil1x1 = Create2DAttachment(device, 1, 1, nxt::TextureFormat::D32FloatS8Uint);
    nxt::TextureView depthStencil2x2 = Create2DAttachment(device, 2, 2, nxt::TextureFormat::D32FloatS8Uint);

    // Control case: all the same size (1x1)
    {
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color1x1A, nxt::LoadOp::Clear)
            .SetColorAttachment(1, color1x1B, nxt::LoadOp::Clear)
            .SetDepthStencilAttachment(depthStencil1x1, nxt::LoadOp::Clear, nxt::LoadOp::Clear)
            .GetResult();
    }

    // One of the color attachments has a different size
    {
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color1x1A, nxt::LoadOp::Clear)
            .SetColorAttachment(1, color2x2, nxt::LoadOp::Clear)
            .SetDepthStencilAttachment(depthStencil1x1, nxt::LoadOp::Clear, nxt::LoadOp::Clear)
            .GetResult();
    }

    // The depth stencil attachment has a different size
    {
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color1x1A, nxt::LoadOp::Clear)
            .SetColorAttachment(1, color1x1B, nxt::LoadOp::Clear)
            .SetDepthStencilAttachment(depthStencil2x2, nxt::LoadOp::Clear, nxt::LoadOp::Clear)
            .GetResult();
    }
}

// Attachments formats must match whether they are used for color or depth-stencil
TEST_F(RenderPassDescriptorValidationTest, FormatMismatch) {
    nxt::TextureView color = Create2DAttachment(device, 1, 1, nxt::TextureFormat::R8G8B8A8Unorm);
    nxt::TextureView depthStencil = Create2DAttachment(device, 1, 1, nxt::TextureFormat::D32FloatS8Uint);

    // Using depth-stencil for color
    {
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, depthStencil, nxt::LoadOp::Clear)
            .GetResult();
    }

    // Using color for depth-stencil
    {
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetDepthStencilAttachment(color, nxt::LoadOp::Clear, nxt::LoadOp::Clear)
            .GetResult();
    }
}

// TODO(cwallez@chromium.org): Constraints on attachment aliasing?

} // anonymous namespace
