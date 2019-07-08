// Copyright 2019 The Dawn Authors
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

#include "tests/DawnTest.h"

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/DawnHelpers.h"

class CullingTest : public DawnTest {
  protected:
    dawn::RenderPipeline CreatePipelineForTest(dawn::FrontFace frontFace, dawn::CullMode cullMode) {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor(device);

        // Draw two triangles with different winding orders:
        // 1. The top-left one is counterclockwise (CCW)
        // 2. The bottom-right one is clockwise (CW)
        const char* vs =
            R"(#version 450
        const vec2 pos[6] = vec2[6](vec2(-1.0f, -1.0f),
                                    vec2(-1.0f,  0.0f),
                                    vec2( 0.0f, -1.0f),
                                    vec2( 0.0f,  1.0f),
                                    vec2( 1.0f,  0.0f),
                                    vec2( 1.0f,  1.0f));
        void main() {
           gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);
       })";
        pipelineDescriptor.cVertexStage.module =
            utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, vs);

        const char* fs =
            "#version 450\n"
            "layout(location = 0) out vec4 fragColor;"
            "void main() {\n"
            "   fragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "}\n";
        pipelineDescriptor.cFragmentStage.module =
            utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, fs);

        // Set culling mode and front face according to the parameters
        pipelineDescriptor.cRasterizationState.frontFace = frontFace;
        pipelineDescriptor.cRasterizationState.cullMode = cullMode;

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    dawn::Texture Create2DTextureForTest(dawn::TextureFormat format) {
        dawn::TextureDescriptor textureDescriptor;
        textureDescriptor.dimension = dawn::TextureDimension::e2D;
        textureDescriptor.format = format;
        textureDescriptor.usage =
            dawn::TextureUsageBit::OutputAttachment | dawn::TextureUsageBit::CopySrc;
        textureDescriptor.arrayLayerCount = 1;
        textureDescriptor.mipLevelCount = 1;
        textureDescriptor.sampleCount = 1;
        textureDescriptor.size = {kSize, kSize, 1};
        return device.CreateTexture(&textureDescriptor);
    }

    void DoTest(dawn::FrontFace frontFace,
                dawn::CullMode cullMode,
                bool isCCWTriangleCulled,
                bool isCWTriangleCulled) {
        dawn::Texture colorTexture = Create2DTextureForTest(dawn::TextureFormat::RGBA8Unorm);

        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateDefaultView()});
        renderPassDescriptor.cColorAttachmentsInfoPtr[0]->clearColor = {0.0, 1.0, 0.0, 1.0};
        renderPassDescriptor.cColorAttachmentsInfoPtr[0]->loadOp = dawn::LoadOp::Clear;

        dawn::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        dawn::RenderPassEncoder renderPass = commandEncoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(CreatePipelineForTest(frontFace, cullMode));
        renderPass.Draw(6, 1, 0, 0);
        renderPass.EndPass();
        dawn::CommandBuffer commandBuffer = commandEncoder.Finish();
        dawn::Queue queue = device.CreateQueue();
        queue.Submit(1, &commandBuffer);

        constexpr RGBA8 kDrawingColor = RGBA8(255, 0, 0, 255);
        constexpr RGBA8 kBackgroundColor = RGBA8(0, 255, 0, 255);

        RGBA8 kCCWTriangleColor = isCCWTriangleCulled ? kBackgroundColor : kDrawingColor;
        EXPECT_PIXEL_RGBA8_EQ(kCCWTriangleColor, colorTexture, 0, 0);

        RGBA8 kCWTriangleColor = isCWTriangleCulled ? kBackgroundColor : kDrawingColor;
        EXPECT_PIXEL_RGBA8_EQ(kCWTriangleColor, colorTexture, kSize - 1, kSize - 1);
    }

    static constexpr uint32_t kSize = 4;
};

TEST_P(CullingTest, CullNoneWhenCCWIsFrontFace) {
    DoTest(dawn::FrontFace::CCW, dawn::CullMode::None, false, false);
}

TEST_P(CullingTest, CullFrontFaceWhenCCWIsFrontFace) {
    DoTest(dawn::FrontFace::CCW, dawn::CullMode::Front, true, false);
}

TEST_P(CullingTest, CullBackFaceWhenCCWIsFrontFace) {
    DoTest(dawn::FrontFace::CCW, dawn::CullMode::Back, false, true);
}

TEST_P(CullingTest, CullNoneWhenCWIsFrontFace) {
    DoTest(dawn::FrontFace::CW, dawn::CullMode::None, false, false);
}

TEST_P(CullingTest, CullFrontFaceWhenCWIsFrontFace) {
    DoTest(dawn::FrontFace::CW, dawn::CullMode::Front, false, true);
}

TEST_P(CullingTest, CullBackFaceWhenCWIsFrontFace) {
    DoTest(dawn::FrontFace::CW, dawn::CullMode::Back, true, false);
}

DAWN_INSTANTIATE_TEST(CullingTest, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend);
