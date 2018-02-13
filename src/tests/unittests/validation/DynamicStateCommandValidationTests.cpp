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

class SetScissorRectTest : public ValidationTest {
};

// Test to check that SetScissor can only be used inside render subpasses
TEST_F(SetScissorRectTest, AllowedOnlyInRenderSubpass) {
    DummyRenderPass renderPass = CreateDummyRenderPass();

    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderPass.renderPass, renderPass.framebuffer)
        .BeginRenderSubpass()
        .SetScissorRect(0, 0, 1, 1)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    AssertWillBeError(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderPass.renderPass, renderPass.framebuffer)
        .SetScissorRect(0, 0, 1, 1)
        .BeginRenderSubpass()
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    AssertWillBeError(device.CreateCommandBufferBuilder())
        .SetScissorRect(0, 0, 1, 1)
        .GetResult();

    AssertWillBeError(device.CreateCommandBufferBuilder())
        .BeginComputePass()
        .SetScissorRect(0, 0, 1, 1)
        .EndComputePass()
        .GetResult();
}

// Test to check that an empty scissor is allowed
TEST_F(SetScissorRectTest, EmptyScissor) {
    DummyRenderPass renderPass = CreateDummyRenderPass();

    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderPass.renderPass, renderPass.framebuffer)
        .BeginRenderSubpass()
        .SetScissorRect(0, 0, 0, 0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();
}

// Test to check that a scissor larger than the framebuffer is allowed
// TODO(cwallez@chromium.org): scissor values seem to be integers in all APIs do the same
// and test negative values?
TEST_F(SetScissorRectTest, ScissorLargerThanFramebuffer) {
    DummyRenderPass renderPass = CreateDummyRenderPass();

    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderPass.renderPass, renderPass.framebuffer)
        .BeginRenderSubpass()
        .SetScissorRect(0, 0, renderPass.width + 1, renderPass.height + 1)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();
}

class SetBlendColorTest : public ValidationTest {
};

// Test to check that SetBlendColor can only be used inside render subpasses
TEST_F(SetBlendColorTest, AllowedOnlyInRenderSubpass) {
    DummyRenderPass renderPass = CreateDummyRenderPass();

    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderPass.renderPass, renderPass.framebuffer)
        .BeginRenderSubpass()
        .SetBlendColor(0.0f, 0.0f, 0.0f, 0.0f)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    AssertWillBeError(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderPass.renderPass, renderPass.framebuffer)
        .SetBlendColor(0.0f, 0.0f, 0.0f, 0.0f)
        .BeginRenderSubpass()
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    AssertWillBeError(device.CreateCommandBufferBuilder())
        .SetBlendColor(0.0f, 0.0f, 0.0f, 0.0f)
        .GetResult();

    AssertWillBeError(device.CreateCommandBufferBuilder())
        .BeginComputePass()
        .SetBlendColor(0.0f, 0.0f, 0.0f, 0.0f)
        .EndComputePass()
        .GetResult();
}

// Test that SetBlendColor allows any value, large, small or negative
TEST_F(SetBlendColorTest, AnyValueAllowed) {
    DummyRenderPass renderPass = CreateDummyRenderPass();

    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderPass.renderPass, renderPass.framebuffer)
        .BeginRenderSubpass()
        .SetBlendColor(-1.0f, 42.0f, -0.0f, 0.0f)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();
}

class SetStencilReferenceTest : public ValidationTest {
};

// Test to check that SetStencilReference can only be used inside render subpasses
TEST_F(SetStencilReferenceTest, AllowedOnlyInRenderSubpass) {
    DummyRenderPass renderPass = CreateDummyRenderPass();

    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderPass.renderPass, renderPass.framebuffer)
        .BeginRenderSubpass()
        .SetStencilReference(0)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    AssertWillBeError(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderPass.renderPass, renderPass.framebuffer)
        .SetStencilReference(0)
        .BeginRenderSubpass()
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();

    AssertWillBeError(device.CreateCommandBufferBuilder())
        .SetStencilReference(0)
        .GetResult();

    AssertWillBeError(device.CreateCommandBufferBuilder())
        .BeginComputePass()
        .SetStencilReference(0)
        .EndComputePass()
        .GetResult();
}

// Test that SetStencilReference allows any bit to be set
TEST_F(SetStencilReferenceTest, AllBitsAllowed) {
    DummyRenderPass renderPass = CreateDummyRenderPass();

    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
        .BeginRenderPass(renderPass.renderPass, renderPass.framebuffer)
        .BeginRenderSubpass()
        .SetStencilReference(0xFFFFFFFF)
        .EndRenderSubpass()
        .EndRenderPass()
        .GetResult();
}
