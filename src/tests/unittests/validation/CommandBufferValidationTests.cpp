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

class CommandBufferValidationTest : public ValidationTest {
};

// Test for an empty command buffer
TEST_F(CommandBufferValidationTest, Empty) {
    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
        .GetResult();
}

// Tests for null arguments to the command buffer builder
TEST_F(CommandBufferValidationTest, NullArguments) {
    auto renderpass = AssertWillBeSuccess(device.CreateRenderPassBuilder())
        .SetSubpassCount(1)
        .SetAttachmentCount(0)
        .GetResult();
    auto framebuffer = AssertWillBeSuccess(device.CreateFramebufferBuilder())
        .SetRenderPass(renderpass)
        .SetDimensions(100, 100)
        .GetResult();

    AssertWillBeError(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderpass, nullptr)
        .EndRenderPass()
        .GetResult();
    AssertWillBeError(device.CreateCommandBufferBuilder())
        .BeginRenderPass(nullptr, framebuffer)
        .EndRenderPass()
        .GetResult();
}

// Tests for basic render pass usage
TEST_F(CommandBufferValidationTest, RenderPass) {
    auto renderpass = AssertWillBeSuccess(device.CreateRenderPassBuilder())
        .SetAttachmentCount(0)
        .SetSubpassCount(1)
        .GetResult();
    auto framebuffer = AssertWillBeSuccess(device.CreateFramebufferBuilder())
        .SetRenderPass(renderpass)
        .SetDimensions(100, 100)
        .GetResult();

    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderpass, framebuffer)
        .EndRenderPass()
        .GetResult();
    AssertWillBeError(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderpass, framebuffer)
        .GetResult();
}
