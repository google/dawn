// Copyright 2017 The Dawn Authors
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

// Tests for basic render pass usage
TEST_F(CommandBufferValidationTest, RenderPass) {
    auto renderpass = CreateSimpleRenderPass();

    {
        dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpass);
        pass.EndPass();
        builder.GetResult();
    }

    {
        dawn::CommandBufferBuilder builder = AssertWillBeError(device.CreateCommandBufferBuilder());
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpass);
        builder.GetResult();
    }
}
