// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <cmath>
#include <string>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

int8_t ConvertFloatToSnorm8(float value) {
    float roundedValue = (value >= 0) ? (value + 0.5f) : (value - 0.5f);
    float clampedValue = std::clamp(roundedValue, -128.0f, 127.0f);
    return static_cast<int8_t>(clampedValue);
}

class TextureFormatsTier1Test : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::TextureFormatsTier1})) {
            requiredFeatures.push_back(wgpu::FeatureName::TextureFormatsTier1);
        }
        return requiredFeatures;
    }

    const char* GetFullScreenQuadVS() {
        return R"(
                @vertex
                fn vs_main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
                    var positions = array<vec2<f32>, 3>(
                        vec2<f32>(-1.0, -1.0),
                        vec2<f32>( 3.0, -1.0),
                        vec2<f32>( -1.0, 3.0));
                    return vec4<f32>(positions[VertexIndex], 0.0, 1.0);
                }
            )";
    }

    std::string GenerateFragmentShader(const std::vector<float>& srcColorFloats) {
        std::ostringstream fsCodeStream;
        fsCodeStream << R"(
    @fragment
    fn fs_main() -> @location(0) vec4<f32> {
        return vec4<f32>()";

        for (size_t i = 0; i < 4; ++i) {
            if (i < srcColorFloats.size()) {
                fsCodeStream << srcColorFloats[i];
            } else {
                fsCodeStream << (i == 3 ? "1.0" : "0.0");
            }
            if (i < 3) {
                fsCodeStream << ", ";
            }
        }

        fsCodeStream << ");\n"
                     << "}\n";

        return fsCodeStream.str();
    }
};

class RenderAttachmentSnormFormatsTest : public TextureFormatsTier1Test {
  protected:
    void RunRenderTest(wgpu::TextureFormat format, const std::vector<float>& originData) {
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::TextureFormatsTier1));
        wgpu::Extent3D textureSize = {16, 16, 1};
        wgpu::TextureDescriptor textureDesc;
        textureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        textureDesc.dimension = wgpu::TextureDimension::e2D;
        textureDesc.size = textureSize;
        textureDesc.format = format;

        wgpu::Texture texture = device.CreateTexture(&textureDesc);

        std::string fsCode = GenerateFragmentShader(originData);

        std::string combinedShaderCode = GetFullScreenQuadVS() + fsCode;
        wgpu::ShaderModule shaderModule =
            utils::CreateShaderModule(device, combinedShaderCode.c_str());

        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.vertex.module = shaderModule;
        pipelineDesc.vertex.entryPoint = "vs_main";
        pipelineDesc.cFragment.module = shaderModule;
        pipelineDesc.cFragment.entryPoint = "fs_main";

        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        pipelineDesc.cFragment.targetCount = 1;
        pipelineDesc.cTargets[0].format = format;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        wgpu::TextureView textureView = texture.CreateView();

        utils::ComboRenderPassDescriptor renderPass;
        renderPass.cColorAttachments[0].view = textureView;
        renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
        renderPass.cColorAttachments[0].clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
        renderPass.colorAttachmentCount = 1;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);

        pass.SetPipeline(pipeline);
        pass.Draw(3);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        std::vector<int8_t> expectedLowerBounds;
        std::vector<int8_t> expectedUpperBounds;

        uint32_t componentCount = utils::GetTextureComponentCount(format);

        for (uint32_t i = 0; i < componentCount; ++i) {
            float floatComponent = originData[i];
            float scaledComponent = floatComponent * 127.f;

            int8_t lowerSnormExpectation = ConvertFloatToSnorm8(scaledComponent - 0.6f);
            int8_t upperSnormExpectation = ConvertFloatToSnorm8(scaledComponent + 0.6f);

            expectedLowerBounds.push_back(lowerSnormExpectation);
            expectedUpperBounds.push_back(upperSnormExpectation);
        }

        EXPECT_TEXTURE_SNORM_BETWEEN(expectedLowerBounds, expectedUpperBounds, texture, {0, 0},
                                     {1, 1}, format);
    }
};

// Test that r8snorm format is valid as renderable texture if
// 'texture-formats-tier1' is enabled.
TEST_P(RenderAttachmentSnormFormatsTest, R8SnormRenderAttachment) {
    std::vector<float> originData = {-0.5f};  // R
    RunRenderTest(wgpu::TextureFormat::R8Snorm, originData);
}

// Test that rg8snorm format is valid as renderable texture if
// 'texture-formats-tier1' is enabled.
TEST_P(RenderAttachmentSnormFormatsTest, RG8SnormRenderAttachment) {
    std::vector<float> originData = {-0.5f, 0.25f};  // RG
    RunRenderTest(wgpu::TextureFormat::RG8Snorm, originData);
}

// Test that r8snorm format is valid as renderable texture if
// 'texture-formats-tier1' is enabled.
TEST_P(RenderAttachmentSnormFormatsTest, RGBA8SnormRenderAttachment) {
    std::vector<float> originData = {-0.5f, 0.25f, -1.0f, 1.0f};  // RGBA
    RunRenderTest(wgpu::TextureFormat::RGBA8Snorm, originData);
}

DAWN_INSTANTIATE_TEST(RenderAttachmentSnormFormatsTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend(),
                      OpenGLBackend());

class BlendableSnormTextureTest : public TextureFormatsTier1Test {
  protected:
    void RunBlendTest(wgpu::TextureFormat format,
                      const std::vector<float>& srcColorFloats,
                      const std::vector<float>& clearColorFloats) {
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::TextureFormatsTier1));

        std::string fsCode = GenerateFragmentShader(srcColorFloats);

        wgpu::Extent3D textureSize = {16, 16, 1};
        wgpu::TextureDescriptor textureDesc;
        textureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        textureDesc.dimension = wgpu::TextureDimension::e2D;
        textureDesc.size = textureSize;
        textureDesc.format = format;
        wgpu::Texture renderTarget = device.CreateTexture(&textureDesc);
        wgpu::TextureView renderTargetView = renderTarget.CreateView();

        std::string combinedShaderCode = GetFullScreenQuadVS() + fsCode;
        wgpu::ShaderModule shaderModule =
            utils::CreateShaderModule(device, combinedShaderCode.c_str());

        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.vertex.module = shaderModule;
        pipelineDesc.vertex.entryPoint = "vs_main";
        pipelineDesc.cFragment.module = shaderModule;
        pipelineDesc.cFragment.entryPoint = "fs_main";
        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        pipelineDesc.cFragment.targetCount = 1;

        pipelineDesc.cTargets[0].format = format;
        pipelineDesc.cTargets[0].blend = &pipelineDesc.cBlends[0];

        pipelineDesc.cBlends[0].color.srcFactor = wgpu::BlendFactor::SrcAlpha;
        pipelineDesc.cBlends[0].color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
        pipelineDesc.cBlends[0].color.operation = wgpu::BlendOperation::Add;
        pipelineDesc.cBlends[0].alpha.srcFactor = wgpu::BlendFactor::One;
        pipelineDesc.cBlends[0].alpha.dstFactor = wgpu::BlendFactor::Zero;
        pipelineDesc.cBlends[0].alpha.operation = wgpu::BlendOperation::Add;
        pipelineDesc.cTargets[0].writeMask = wgpu::ColorWriteMask::All;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass;
        renderPass.cColorAttachments[0].view = renderTargetView;
        renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;

        renderPass.cColorAttachments[0].clearValue = {clearColorFloats[0], clearColorFloats[1],
                                                      clearColorFloats[2], clearColorFloats[3]};
        renderPass.colorAttachmentCount = 1;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);
        pass.Draw(3);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        std::vector<float> expectedBlendedFloats(4);
        float srcR = (srcColorFloats.size() > 0) ? srcColorFloats[0] : 0.0f;
        float srcG = (srcColorFloats.size() > 1) ? srcColorFloats[1] : 0.0f;
        float srcB = (srcColorFloats.size() > 2) ? srcColorFloats[2] : 0.0f;
        float srcA = (srcColorFloats.size() > 3) ? srcColorFloats[3] : 1.0f;

        float dstR = clearColorFloats[0];
        float dstG = clearColorFloats[1];
        float dstB = clearColorFloats[2];

        expectedBlendedFloats[0] = srcR * srcA + dstR * (1.0f - srcA);
        expectedBlendedFloats[1] = srcG * srcA + dstG * (1.0f - srcA);
        expectedBlendedFloats[2] = srcB * srcA + dstB * (1.0f - srcA);
        expectedBlendedFloats[3] = srcA;

        uint32_t componentCount = utils::GetTextureComponentCount(format);
        std::vector<int8_t> expectedLowerBounds;
        std::vector<int8_t> expectedUpperBounds;

        for (uint32_t i = 0; i < componentCount; ++i) {
            float floatComponent = expectedBlendedFloats[i];
            float scaledComponent = floatComponent * 127.f;

            int8_t lowerSnormExpectation = ConvertFloatToSnorm8(scaledComponent - 0.6f);
            int8_t upperSnormExpectation = ConvertFloatToSnorm8(scaledComponent + 0.6f);

            expectedLowerBounds.push_back(lowerSnormExpectation);
            expectedUpperBounds.push_back(upperSnormExpectation);
        }

        EXPECT_TEXTURE_SNORM_BETWEEN(expectedLowerBounds, expectedUpperBounds, renderTarget, {0, 0},
                                     {1, 1}, format);
    }
};

// Test that r8snorm format is blendable when 'texture-formats-tier1' is enabled.
TEST_P(BlendableSnormTextureTest, R8SnormBlendable) {
    std::vector<float> srcColor = {1.0f, 0.0f, 0.0f, 0.5f};
    std::vector<float> clearColor = {0.0f, 0.0f, 1.0f, 1.0f};
    RunBlendTest(wgpu::TextureFormat::R8Snorm, srcColor, clearColor);
}

// Test that rg8snorm format is blendable when 'texture-formats-tier1' is enabled.
TEST_P(BlendableSnormTextureTest, RG8SnormBlendable) {
    std::vector<float> srcColor = {1.0f, 0.0f, 0.0f, 0.5f};
    std::vector<float> clearColor = {0.0f, 0.0f, 1.0f, 1.0f};
    RunBlendTest(wgpu::TextureFormat::RG8Snorm, srcColor, clearColor);
}

// Test that rgba8snorm format is blendable when 'texture-formats-tier1' is enabled.
TEST_P(BlendableSnormTextureTest, RGBA8SnormBlendable) {
    std::vector<float> srcColor = {1.0f, 0.0f, 0.0f, 0.5f};
    std::vector<float> clearColor = {0.0f, 0.0f, 1.0f, 1.0f};
    RunBlendTest(wgpu::TextureFormat::RGBA8Snorm, srcColor, clearColor);
}

DAWN_INSTANTIATE_TEST(BlendableSnormTextureTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend(),
                      OpenGLBackend());

}  // anonymous namespace
}  // namespace dawn
