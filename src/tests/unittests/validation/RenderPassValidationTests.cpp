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

class RenderPassValidationTest : public ValidationTest {
};

// Test for an empty render pass builder
TEST_F(RenderPassValidationTest, Empty) {
    AssertWillBeError(device.CreateRenderPassBuilder())
        .GetResult();
}

// Test for a render pass with one subpass and no attachments
TEST_F(RenderPassValidationTest, OneSubpass) {
    AssertWillBeSuccess(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(0)
        .GetResult();
}

// Test for a render pass with one subpass and one attachment
TEST_F(RenderPassValidationTest, OneSubpassOneAttachment) {
    AssertWillBeSuccess(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        // without a load op
        .SubpassSetColorAttachment(0, 0, 0)
        .GetResult();
}

// Tests for setting attachment load ops
TEST_F(RenderPassValidationTest, AttachmentLoadOps) {
    AssertWillBeSuccess(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        // with a load op
        .AttachmentSetColorLoadOp(0, nxt::LoadOp::Clear)
        .SubpassSetColorAttachment(0, 0, 0)
        .GetResult();

    AssertWillBeSuccess(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        // with a load op of the wrong type - this is okay, just ignored
        .AttachmentSetDepthStencilLoadOps(0, nxt::LoadOp::Clear, nxt::LoadOp::Clear)
        .SubpassSetColorAttachment(0, 0, 0)
        .GetResult();
}

// Test for attachment slot arguments out of bounds
TEST_F(RenderPassValidationTest, AttachmentOutOfBounds) {
    // Control case
    AssertWillBeSuccess(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .GetResult();

    AssertWillBeError(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        // Test AttachmentSetFormat slot out of bounds
        .AttachmentSetFormat(1, nxt::TextureFormat::R8G8B8A8Unorm)
        .GetResult();

    AssertWillBeError(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        // Test AttachmentSetColorLoadOp slot out of bounds
        .AttachmentSetColorLoadOp(1, nxt::LoadOp::Clear)
        .GetResult();

    AssertWillBeError(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        // Test AttachmentSetDepthStencilLoadOps slot out of bounds
        .AttachmentSetDepthStencilLoadOps(1, nxt::LoadOp::Clear, nxt::LoadOp::Clear)
        .GetResult();

    AssertWillBeError(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        // Test SubpassSetColorAttachment attachment slot out of bounds
        .SubpassSetColorAttachment(0, 0, 1)
        .GetResult();

    AssertWillBeError(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        // Test SubpassSetDepthStencilAttachment attachment slot out of bounds
        .SubpassSetDepthStencilAttachment(0, 1)
        .GetResult();
}

// Test for subpass arguments out of bounds
TEST_F(RenderPassValidationTest, SubpassOutOfBounds) {
    // Control case
    AssertWillBeSuccess(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .SubpassSetColorAttachment(0, 0, 0)
        .GetResult();

    AssertWillBeError(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .SubpassSetColorAttachment(0, 0, 0)
        // Test SubpassSetColorAttachment subpass out of bounds
        .SubpassSetColorAttachment(1, 0, 0)
        .GetResult();

    AssertWillBeError(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::D32FloatS8Uint)
        // Test SubpassSetDepthStencilAttachment subpass out of bounds
        .SubpassSetDepthStencilAttachment(1, 0)
        .GetResult();
}

// Test attaching depth/stencil textures to color attachments and vice versa
TEST_F(RenderPassValidationTest, SubpassAttachmentWrongAspect) {
    AssertWillBeError(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .SubpassSetDepthStencilAttachment(0, 0)
        .GetResult();

    AssertWillBeError(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::D32FloatS8Uint)
        .SubpassSetColorAttachment(0, 0, 0)
        .GetResult();
}
