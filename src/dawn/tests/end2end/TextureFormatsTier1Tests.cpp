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
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class RenderAttachmentSnormFormatsTest : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::TextureFormatsTier1})) {
            requiredFeatures.push_back(wgpu::FeatureName::TextureFormatsTier1);
        }
        return requiredFeatures;
    }

    const char* GetSinglePointVS() {
        return R"(
            @vertex
            fn main() -> @builtin(position) vec4<f32> {
                return vec4<f32>(0.0, 0.0, 0.0, 1.0);
            }
        )";
    }

    int8_t ConvertFloatToSnorm8(float value) {
        float roundedValue = (value >= 0) ? (value + 0.5f) : (value - 0.5f);
        float clampedValue = std::clamp(roundedValue, -128.0f, 127.0f);
        return static_cast<int8_t>(clampedValue);
    }

    void RunSingleFormatTest(wgpu::TextureFormat format,
                             const char* fsCode,
                             const std::vector<float>& originData) {
        wgpu::Extent3D textureSize = {1, 1, 1};
        wgpu::TextureDescriptor textureDesc;
        textureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        textureDesc.dimension = wgpu::TextureDimension::e2D;
        textureDesc.size = textureSize;
        textureDesc.format = format;

        wgpu::Texture texture = device.CreateTexture(&textureDesc);

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, GetSinglePointVS());
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fsCode);

        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.vertex.module = vsModule;
        pipelineDesc.cFragment.module = fsModule;
        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
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
        pass.Draw(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        std::vector<int8_t> expectedLowerBounds;
        std::vector<int8_t> expectedUpperBounds;

        for (uint32_t i = 0; i < originData.size(); ++i) {
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
    DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::TextureFormatsTier1));

    const char* fs_r8snorm = R"(
        @fragment
        fn main() -> @location(0) vec4<f32> {
            return vec4<f32>(-0.5, 0.0, 0.0, 1.0);
        }
    )";
    std::vector<float> originData = {-0.5};

    RunSingleFormatTest(wgpu::TextureFormat::R8Snorm, fs_r8snorm, originData);
}

// Test that rg8snorm format is valid as renderable texture if
// 'texture-formats-tier1' is enabled.
TEST_P(RenderAttachmentSnormFormatsTest, RG8SnormRenderAttachment) {
    DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::TextureFormatsTier1));

    const char* fs_rg8snorm = R"(
        @fragment
        fn main() -> @location(0) vec4<f32> {
            return vec4<f32>(-0.5, 0.25, 0.0, 1.0);
        }
    )";
    std::vector<float> originData = {-0.5, 0.25};

    RunSingleFormatTest(wgpu::TextureFormat::RG8Snorm, fs_rg8snorm, originData);
}

// Test that r8snorm format is valid as renderable texture if
// 'texture-formats-tier1' is enabled.
TEST_P(RenderAttachmentSnormFormatsTest, RGBA8SnormRenderAttachment) {
    DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::TextureFormatsTier1));

    const char* fs_rgba8snorm = R"(
        @fragment
        fn main() -> @location(0) vec4<f32> {
            return vec4<f32>(-0.5, 0.25, -1.0, 1.0);
        }
    )";
    std::vector<float> originData = {-0.5, 0.25, -1.0, 1.0};
    RunSingleFormatTest(wgpu::TextureFormat::RGBA8Snorm, fs_rgba8snorm, originData);
}

DAWN_INSTANTIATE_TEST(RenderAttachmentSnormFormatsTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend(),
                      OpenGLBackend());

}  // anonymous namespace
}  // namespace dawn
