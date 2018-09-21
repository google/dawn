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

class SetScissorRectTest : public ValidationTest {
};

// Test to check basic use of SetScissor
TEST_F(SetScissorRectTest, Success) {
    DummyRenderPass renderPass = CreateDummyRenderPass();

    dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass.renderPass);
        pass.SetScissorRect(0, 0, 1, 1);
        pass.EndPass();
    }
    builder.GetResult();
}

// Test to check that an empty scissor is allowed
TEST_F(SetScissorRectTest, EmptyScissor) {
    DummyRenderPass renderPass = CreateDummyRenderPass();

    dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass.renderPass);
        pass.SetScissorRect(0, 0, 0, 0);
        pass.EndPass();
    }
    builder.GetResult();
}

// Test to check that a scissor larger than the framebuffer is allowed
// TODO(cwallez@chromium.org): scissor values seem to be integers in all APIs do the same
// and test negative values?
TEST_F(SetScissorRectTest, ScissorLargerThanFramebuffer) {
    DummyRenderPass renderPass = CreateDummyRenderPass();

    dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass.renderPass);
        pass.SetScissorRect(0, 0, renderPass.width + 1, renderPass.height + 1);
        pass.EndPass();
    }
    builder.GetResult();
}

class SetBlendColorTest : public ValidationTest {
};

// Test to check basic use of SetBlendColor
TEST_F(SetBlendColorTest, Success) {
    DummyRenderPass renderPass = CreateDummyRenderPass();

    dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass.renderPass);
        pass.SetBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
        pass.EndPass();
    }
    builder.GetResult();
}

// Test that SetBlendColor allows any value, large, small or negative
TEST_F(SetBlendColorTest, AnyValueAllowed) {
    DummyRenderPass renderPass = CreateDummyRenderPass();

    dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass.renderPass);
        pass.SetBlendColor(-1.0f, 42.0f, -0.0f, 0.0f);
        pass.EndPass();
    }
    builder.GetResult();
}

class SetStencilReferenceTest : public ValidationTest {
};

// Test to check basic use of SetBlendColor
TEST_F(SetStencilReferenceTest, Success) {
    DummyRenderPass renderPass = CreateDummyRenderPass();

    dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass.renderPass);
        pass.SetStencilReference(0);
        pass.EndPass();
    }
    builder.GetResult();
}

// Test that SetStencilReference allows any bit to be set
TEST_F(SetStencilReferenceTest, AllBitsAllowed) {
    DummyRenderPass renderPass = CreateDummyRenderPass();

    dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderPass.renderPass);
        pass.SetStencilReference(0xFFFFFFFF);
        pass.EndPass();
    }
    builder.GetResult();
}
