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

#include "common/Constants.h"

namespace {

class RenderPassDescriptorValidationTest : public ValidationTest {
};

dawn::TextureView Create2DAttachment(dawn::Device& device, uint32_t width, uint32_t height, dawn::TextureFormat format) {
    dawn::TextureDescriptor descriptor;
    descriptor.dimension = dawn::TextureDimension::e2D;
    descriptor.width = width;
    descriptor.height = height;
    descriptor.depth = 1;
    descriptor.arrayLayer = 1;
    descriptor.format = format;
    descriptor.mipLevel = 1;
    descriptor.usage = dawn::TextureUsageBit::OutputAttachment;
    dawn::Texture attachment = device.CreateTexture(&descriptor);

    return attachment.CreateDefaultTextureView();
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
        dawn::TextureView color = Create2DAttachment(device, 1, 1, dawn::TextureFormat::R8G8B8A8Unorm);
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color, dawn::LoadOp::Clear)
            .GetResult();
    }
    // One depth-stencil attachment
    {
        dawn::TextureView depthStencil = Create2DAttachment(device, 1, 1, dawn::TextureFormat::D32FloatS8Uint);
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetDepthStencilAttachment(depthStencil, dawn::LoadOp::Clear, dawn::LoadOp::Clear)
            .GetResult();
    }
}

// Test OOB color attachment indices are handled
TEST_F(RenderPassDescriptorValidationTest, ColorAttachmentOutOfBounds) {
    // For setting the color attachment, control case
    {
        dawn::TextureView color = Create2DAttachment(device, 1, 1, dawn::TextureFormat::R8G8B8A8Unorm);
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(kMaxColorAttachments - 1, color, dawn::LoadOp::Clear)
            .GetResult();
    }
    // For setting the color attachment, OOB
    {
        dawn::TextureView color = Create2DAttachment(device, 1, 1, dawn::TextureFormat::R8G8B8A8Unorm);
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(kMaxColorAttachments, color, dawn::LoadOp::Clear)
            .GetResult();
    }

    dawn::TextureView color = Create2DAttachment(device, 1, 1, dawn::TextureFormat::R8G8B8A8Unorm);
    // For setting the clear color, control case
    {
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color, dawn::LoadOp::Clear)
            .SetColorAttachmentClearColor(kMaxColorAttachments - 1, 0.0f, 0.0f, 0.0f, 0.0f)
            .GetResult();
    }
    // For setting the clear color, OOB
    {
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color, dawn::LoadOp::Clear)
            .SetColorAttachmentClearColor(kMaxColorAttachments, 0.0f, 0.0f, 0.0f, 0.0f)
            .GetResult();
    }
}

// Test setting a clear value without an attachment and vice-versa is ok.
TEST_F(RenderPassDescriptorValidationTest, ClearAndAttachmentMismatchIsOk) {
    dawn::TextureView color = Create2DAttachment(device, 1, 1, dawn::TextureFormat::R8G8B8A8Unorm);

    // For cleared attachment 0 doesn't get a color, clear color for 1 is unused
    {
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color, dawn::LoadOp::Clear)
            .SetColorAttachmentClearColor(1, 0.0f, 0.0f, 0.0f, 0.0f)
            .GetResult();
    }
    // Clear depth stencil doesn't get values
    {
        dawn::TextureView depthStencil = Create2DAttachment(device, 1, 1, dawn::TextureFormat::D32FloatS8Uint);
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetDepthStencilAttachment(depthStencil, dawn::LoadOp::Clear, dawn::LoadOp::Clear)
            .GetResult();
    }
    // Clear values for depth-stencil when it isn't used
    {
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color, dawn::LoadOp::Clear)
            .SetDepthStencilAttachmentClearValue(0.0f, 0)
            .GetResult();
    }
}

// Attachments must have the same size
TEST_F(RenderPassDescriptorValidationTest, SizeMustMatch) {
    dawn::TextureView color1x1A = Create2DAttachment(device, 1, 1, dawn::TextureFormat::R8G8B8A8Unorm);
    dawn::TextureView color1x1B = Create2DAttachment(device, 1, 1, dawn::TextureFormat::R8G8B8A8Unorm);
    dawn::TextureView color2x2 = Create2DAttachment(device, 2, 2, dawn::TextureFormat::R8G8B8A8Unorm);
    dawn::TextureView depthStencil1x1 = Create2DAttachment(device, 1, 1, dawn::TextureFormat::D32FloatS8Uint);
    dawn::TextureView depthStencil2x2 = Create2DAttachment(device, 2, 2, dawn::TextureFormat::D32FloatS8Uint);

    // Control case: all the same size (1x1)
    {
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color1x1A, dawn::LoadOp::Clear)
            .SetColorAttachment(1, color1x1B, dawn::LoadOp::Clear)
            .SetDepthStencilAttachment(depthStencil1x1, dawn::LoadOp::Clear, dawn::LoadOp::Clear)
            .GetResult();
    }

    // One of the color attachments has a different size
    {
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color1x1A, dawn::LoadOp::Clear)
            .SetColorAttachment(1, color2x2, dawn::LoadOp::Clear)
            .SetDepthStencilAttachment(depthStencil1x1, dawn::LoadOp::Clear, dawn::LoadOp::Clear)
            .GetResult();
    }

    // The depth stencil attachment has a different size
    {
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, color1x1A, dawn::LoadOp::Clear)
            .SetColorAttachment(1, color1x1B, dawn::LoadOp::Clear)
            .SetDepthStencilAttachment(depthStencil2x2, dawn::LoadOp::Clear, dawn::LoadOp::Clear)
            .GetResult();
    }
}

// Attachments formats must match whether they are used for color or depth-stencil
TEST_F(RenderPassDescriptorValidationTest, FormatMismatch) {
    dawn::TextureView color = Create2DAttachment(device, 1, 1, dawn::TextureFormat::R8G8B8A8Unorm);
    dawn::TextureView depthStencil = Create2DAttachment(device, 1, 1, dawn::TextureFormat::D32FloatS8Uint);

    // Using depth-stencil for color
    {
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachment(0, depthStencil, dawn::LoadOp::Clear)
            .GetResult();
    }

    // Using color for depth-stencil
    {
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetDepthStencilAttachment(color, dawn::LoadOp::Clear, dawn::LoadOp::Clear)
            .GetResult();
    }
}

// TODO(cwallez@chromium.org): Constraints on attachment aliasing?

} // anonymous namespace
