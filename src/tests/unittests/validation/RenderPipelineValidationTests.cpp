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

#include "utils/NXTHelpers.h"

class RenderPipelineValidationTest : public ValidationTest {
    protected:
        void SetUp() override {
            ValidationTest::SetUp();

            CreateSimpleRenderPassAndFramebuffer(device, &renderpass, &framebuffer);

            pipelineLayout = device.CreatePipelineLayoutBuilder().GetResult();

            inputState = device.CreateInputStateBuilder().GetResult();

            blendState = device.CreateBlendStateBuilder().GetResult();

            vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
                #version 450
                void main() {
                    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
                })"
            );

            fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
                #version 450
                out vec4 fragColor;
                void main() {
                    fragColor = vec4(0.0, 1.0, 0.0, 1.0);
                })"
            );
        }

        nxt::RenderPipelineBuilder& AddDefaultStates(nxt::RenderPipelineBuilder&& builder) {
            builder.SetSubpass(renderpass, 0)
                .SetLayout(pipelineLayout)
                .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
                .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
                .SetPrimitiveTopology(nxt::PrimitiveTopology::TriangleList);
            return builder;
        }

        nxt::RenderPass renderpass;
        nxt::Framebuffer framebuffer;
        nxt::ShaderModule vsModule;
        nxt::ShaderModule fsModule;
        nxt::InputState inputState;
        nxt::BlendState blendState;
        nxt::PipelineLayout pipelineLayout;
};

// Test cases where creation should succeed
TEST_F(RenderPipelineValidationTest, CreationSuccess) {
    AddDefaultStates(AssertWillBeSuccess(device.CreateRenderPipelineBuilder()))
        .GetResult();

    AddDefaultStates(AssertWillBeSuccess(device.CreateRenderPipelineBuilder()))
        .SetInputState(inputState)
        .GetResult();

    AddDefaultStates(AssertWillBeSuccess(device.CreateRenderPipelineBuilder()))
        .SetColorAttachmentBlendState(0, blendState)
        .GetResult();
}

// Test creation failure when properties are missing
TEST_F(RenderPipelineValidationTest, CreationMissingProperty) {
    // Vertex stage not set
    {
        AssertWillBeError(device.CreateRenderPipelineBuilder())
            .SetSubpass(renderpass, 0)
            .SetLayout(pipelineLayout)
            .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
            .SetPrimitiveTopology(nxt::PrimitiveTopology::TriangleList)
            .GetResult();
    }

    // Fragment stage not set
    {
        AssertWillBeError(device.CreateRenderPipelineBuilder())
            .SetSubpass(renderpass, 0)
            .SetLayout(pipelineLayout)
            .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
            .SetPrimitiveTopology(nxt::PrimitiveTopology::TriangleList)
            .GetResult();
    }

    // Subpass not set
    {
        AssertWillBeError(device.CreateRenderPipelineBuilder())
            .SetLayout(pipelineLayout)
            .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
            .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
            .SetPrimitiveTopology(nxt::PrimitiveTopology::TriangleList)
            .GetResult();
    }
}

TEST_F(RenderPipelineValidationTest, BlendState) {
    // Fails because blend state is set on a nonexistent color attachment
    {
        auto texture1 = device.CreateTextureBuilder()
            .SetDimension(nxt::TextureDimension::e2D)
            .SetExtent(640, 480, 1)
            .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
            .SetMipLevels(1)
            .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment)
            .GetResult();
        texture1.FreezeUsage(nxt::TextureUsageBit::OutputAttachment);
        auto textureView1 = texture1.CreateTextureViewBuilder()
            .GetResult();

        auto texture2 = device.CreateTextureBuilder()
            .SetDimension(nxt::TextureDimension::e2D)
            .SetExtent(640, 480, 1)
            .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
            .SetMipLevels(1)
            .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment)
            .GetResult();
        texture2.FreezeUsage(nxt::TextureUsageBit::OutputAttachment);
        auto textureView2 = texture2.CreateTextureViewBuilder()
            .GetResult();

        auto renderpass = device.CreateRenderPassBuilder()
            .SetAttachmentCount(2)
            .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
            .AttachmentSetFormat(1, nxt::TextureFormat::R8G8B8A8Unorm)
            .SetSubpassCount(1)
            .SubpassSetColorAttachment(0, 0, 0)
            .GetResult();

        auto framebuffer = device.CreateFramebufferBuilder()
            .SetRenderPass(renderpass)
            .SetDimensions(640, 480)
            .SetAttachment(0, textureView1)
            .SetAttachment(1, textureView2)
            .GetResult();


        // This one succeeds because attachment 0 is the subpass's color attachment
        AssertWillBeSuccess(device.CreateRenderPipelineBuilder())
            .SetSubpass(renderpass, 0)
            .SetLayout(pipelineLayout)
            .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
            .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
            .SetPrimitiveTopology(nxt::PrimitiveTopology::TriangleList)
            .SetColorAttachmentBlendState(0, blendState)
            .GetResult();

        // This fails because attachment 1 is not one of the subpass's color attachments
        AssertWillBeError(device.CreateRenderPipelineBuilder())
            .SetSubpass(renderpass, 0)
            .SetLayout(pipelineLayout)
            .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
            .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
            .SetPrimitiveTopology(nxt::PrimitiveTopology::TriangleList)
            .SetColorAttachmentBlendState(1, blendState)
            .GetResult();
    }

    // Fails because color attachment is out of bounds
    {
        AddDefaultStates(AssertWillBeError(device.CreateRenderPipelineBuilder()))
            .SetColorAttachmentBlendState(1, blendState)
            .GetResult();
    }

    // Fails because color attachment blend state is set twice
    {
        AddDefaultStates(AssertWillBeError(device.CreateRenderPipelineBuilder()))
            .SetColorAttachmentBlendState(0, blendState)
            .SetColorAttachmentBlendState(0, blendState)
            .GetResult();
    }
}

// TODO(enga@google.com): These should be added to the test above when validation is implemented
TEST_F(RenderPipelineValidationTest, DISABLED_TodoCreationMissingProperty) {
    // Fails because pipeline layout is not set
    {
        AssertWillBeError(device.CreateRenderPipelineBuilder())
            .SetSubpass(renderpass, 0)
            .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
            .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
            .SetPrimitiveTopology(nxt::PrimitiveTopology::TriangleList)
            .GetResult();
    }

    // Fails because primitive topology is not set
    {
        AssertWillBeError(device.CreateRenderPipelineBuilder())
            .SetSubpass(renderpass, 0)
            .SetLayout(pipelineLayout)
            .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
            .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
            .GetResult();
    }
}

// Test creation failure when specifying properties multiple times
TEST_F(RenderPipelineValidationTest, DISABLED_CreationDuplicates) {
    // Fails because input state is set twice
    {
        AddDefaultStates(AssertWillBeError(device.CreateRenderPipelineBuilder()))
            .SetInputState(inputState)
            .GetResult();
    }

    // Fails because primitive topology is set twice
    {
        AddDefaultStates(AssertWillBeError(device.CreateRenderPipelineBuilder()))
            .SetPrimitiveTopology(nxt::PrimitiveTopology::TriangleList)
            .GetResult();
    }

    // Fails because vertex stage is set twice
    {
        AddDefaultStates(AssertWillBeError(device.CreateRenderPipelineBuilder()))
            .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
            .GetResult();
    }

    // Fails because fragment stage is set twice
    {
        AddDefaultStates(AssertWillBeError(device.CreateRenderPipelineBuilder()))
            .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
            .GetResult();
    }

    // Fails because subpass is set twice
    {
        AddDefaultStates(AssertWillBeError(device.CreateRenderPipelineBuilder()))
            .SetSubpass(renderpass, 0)
            .GetResult();
    }

    // Fails because the layout is set twice
    {
        AddDefaultStates(AssertWillBeError(device.CreateRenderPipelineBuilder()))
            .SetLayout(pipelineLayout)
            .GetResult();
    }
}
