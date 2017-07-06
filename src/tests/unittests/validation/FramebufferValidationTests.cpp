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
    auto framebuffer = AssertWillBeSuccess(device.CreateFramebufferBuilder())
        .SetRenderPass(renderpass)
        .SetDimensions(100, 100)
        .GetResult();
}
