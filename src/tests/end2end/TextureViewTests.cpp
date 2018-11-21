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

#include "tests/DawnTest.h"

#include "common/Assert.h"
#include "common/Constants.h"
#include "utils/DawnHelpers.h"

#include <array>

constexpr static unsigned int kRTSize = 64;

class TextureViewTest : public DawnTest {
protected:
    // Generates an arbitrary pixel value per-layer-per-level, used for the "actual" uploaded
    // textures and the "expected" results.
    static int GenerateTestPixelValue(uint32_t layer, uint32_t level) {
        return static_cast<int>(level * 10) + static_cast<int>(layer + 1);
    }

    void SetUp() override {
        DawnTest::SetUp();

        mRenderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        mBindGroupLayout = utils::MakeBindGroupLayout(
            device, {
                        {0, dawn::ShaderStageBit::Fragment, dawn::BindingType::Sampler},
                        {1, dawn::ShaderStageBit::Fragment, dawn::BindingType::SampledTexture},
            });

        dawn::FilterMode kFilterMode = dawn::FilterMode::Nearest;
        dawn::AddressMode kAddressMode = dawn::AddressMode::ClampToEdge;

        dawn::SamplerDescriptor samplerDescriptor;
        samplerDescriptor.minFilter = kFilterMode;
        samplerDescriptor.magFilter = kFilterMode;
        samplerDescriptor.mipmapFilter = kFilterMode;
        samplerDescriptor.addressModeU = kAddressMode;
        samplerDescriptor.addressModeV = kAddressMode;
        samplerDescriptor.addressModeW = kAddressMode;
        mSampler = device.CreateSampler(&samplerDescriptor);

        mPipelineLayout = utils::MakeBasicPipelineLayout(device, &mBindGroupLayout);

        mVSModule = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
            #version 450
            layout (location = 0) out vec2 o_texCoord;
            void main() {
                const vec2 pos[6] = vec2[6](vec2(-2.f, -2.f),
                                            vec2(-2.f,  2.f),
                                            vec2( 2.f, -2.f),
                                            vec2(-2.f,  2.f),
                                            vec2( 2.f, -2.f),
                                            vec2( 2.f,  2.f));
                const vec2 texCoord[6] = vec2[6](vec2(0.f, 0.f),
                                                 vec2(0.f, 1.f),
                                                 vec2(1.f, 0.f),
                                                 vec2(0.f, 1.f),
                                                 vec2(1.f, 0.f),
                                                 vec2(1.f, 1.f));
                gl_Position = vec4(pos[gl_VertexIndex], 0.f, 1.f);
                o_texCoord = texCoord[gl_VertexIndex];
            }
        )");
    }

    void initTexture(uint32_t layerCount, uint32_t levelCount) {
        ASSERT(layerCount > 0 && levelCount > 0);

        constexpr dawn::TextureFormat kFormat = dawn::TextureFormat::R8G8B8A8Unorm;

        const uint32_t textureWidthLevel0 = 1 << levelCount;
        const uint32_t textureHeightLevel0 = 1 << levelCount;

        dawn::TextureDescriptor descriptor;
        descriptor.dimension = dawn::TextureDimension::e2D;
        descriptor.size.width = textureWidthLevel0;
        descriptor.size.height = textureHeightLevel0;
        descriptor.size.depth = 1;
        descriptor.arrayLayer = layerCount;
        descriptor.format = kFormat;
        descriptor.levelCount = levelCount;
        descriptor.usage = dawn::TextureUsageBit::TransferDst | dawn::TextureUsageBit::Sampled;
        mTexture = device.CreateTexture(&descriptor);

        mDefaultTextureViewDescriptor.nextInChain = nullptr;
        mDefaultTextureViewDescriptor.dimension = dawn::TextureViewDimension::e2DArray;
        mDefaultTextureViewDescriptor.format = kFormat;
        mDefaultTextureViewDescriptor.baseMipLevel = 0;
        mDefaultTextureViewDescriptor.levelCount = levelCount;
        mDefaultTextureViewDescriptor.baseArrayLayer = 0;
        mDefaultTextureViewDescriptor.layerCount = layerCount;

        // Create a texture with pixel = (0, 0, 0, level * 10 + layer + 1) at level `level` and
        // layer `layer`.
        static_assert((kTextureRowPitchAlignment % sizeof(RGBA8)) == 0,
            "Texture row pitch alignment must be a multiple of sizeof(RGBA8).");
        constexpr uint32_t kPixelsPerRowPitch = kTextureRowPitchAlignment / sizeof(RGBA8);
        ASSERT_LE(textureWidthLevel0, kPixelsPerRowPitch);

        dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
        for (uint32_t layer = 0; layer < layerCount; ++layer) {
            for (uint32_t level = 0; level < levelCount; ++level) {
                const uint32_t texWidth = textureWidthLevel0 >> level;
                const uint32_t texHeight = textureHeightLevel0 >> level;

                const int pixelValue = GenerateTestPixelValue(layer, level);

                constexpr uint32_t kPaddedTexWidth = kPixelsPerRowPitch;
                std::vector<RGBA8> data(kPaddedTexWidth * texHeight, RGBA8(0, 0, 0, pixelValue));
                dawn::Buffer stagingBuffer = utils::CreateBufferFromData(
                    device, data.data(), data.size() * sizeof(RGBA8),
                    dawn::BufferUsageBit::TransferSrc);
                builder.CopyBufferToTexture(
                    stagingBuffer, 0, kTextureRowPitchAlignment, mTexture, 0, 0, 0, texWidth,
                    texHeight, 1, level, layer);
            }
        }
        dawn::CommandBuffer copy = builder.GetResult();
        queue.Submit(1, &copy);
    }

    void Verify(const dawn::TextureView &textureView, const char* fragmentShader, int expected) {
        dawn::BindGroup bindGroup = device.CreateBindGroupBuilder()
            .SetLayout(mBindGroupLayout)
            .SetSamplers(0, 1, &mSampler)
            .SetTextureViews(1, 1, &textureView)
            .GetResult();

        dawn::ShaderModule fsModule =
            utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, fragmentShader);

        dawn::RenderPipeline pipeline = device.CreateRenderPipelineBuilder()
            .SetColorAttachmentFormat(0, mRenderPass.colorFormat)
            .SetLayout(mPipelineLayout)
            .SetStage(dawn::ShaderStage::Vertex, mVSModule, "main")
            .SetStage(dawn::ShaderStage::Fragment, fsModule, "main")
            .GetResult();

        dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
        {
            dawn::RenderPassEncoder pass = builder.BeginRenderPass(mRenderPass.renderPassInfo);
            pass.SetRenderPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.DrawArrays(6, 1, 0, 0);
            pass.EndPass();
        }

        dawn::CommandBuffer commands = builder.GetResult();
        queue.Submit(1, &commands);

        RGBA8 expectedPixel(0, 0, 0, expected);
        EXPECT_PIXEL_RGBA8_EQ(expectedPixel, mRenderPass.color, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(
            expectedPixel, mRenderPass.color, mRenderPass.width - 1, mRenderPass.height - 1);
        // TODO(jiawei.shao@intel.com): add tests for 3D textures once Dawn supports 3D textures
    }

    void Texture2DViewTest(uint32_t textureArrayLayers,
                           uint32_t textureMipLevels,
                           uint32_t textureViewBaseLayer,
                           uint32_t textureViewBaseMipLevel) {
        ASSERT(textureViewBaseLayer < textureArrayLayers);
        ASSERT(textureViewBaseMipLevel < textureMipLevels);

        initTexture(textureArrayLayers, textureMipLevels);

        dawn::TextureViewDescriptor descriptor = mDefaultTextureViewDescriptor;
        descriptor.dimension = dawn::TextureViewDimension::e2D;
        descriptor.baseArrayLayer = textureViewBaseLayer;
        descriptor.layerCount = 1;
        descriptor.baseMipLevel = textureViewBaseMipLevel;
        descriptor.levelCount = 1;
        dawn::TextureView textureView = mTexture.CreateTextureView(&descriptor);

        const char* fragmentShader = R"(
            #version 450
            layout(set = 0, binding = 0) uniform sampler sampler0;
            layout(set = 0, binding = 1) uniform texture2D texture0;
            layout(location = 0) in vec2 texCoord;
            layout(location = 0) out vec4 fragColor;

            void main() {
                fragColor =
                    texture(sampler2D(texture0, sampler0), texCoord);
            }
        )";

        const int expected = GenerateTestPixelValue(textureViewBaseLayer, textureViewBaseMipLevel);
        Verify(textureView, fragmentShader, expected);
    }

    void Texture2DArrayViewTest(uint32_t textureArrayLayers,
                                uint32_t textureMipLevels,
                                uint32_t textureViewBaseLayer,
                                uint32_t textureViewBaseMipLevel) {
        ASSERT(textureViewBaseLayer < textureArrayLayers);
        ASSERT(textureViewBaseMipLevel < textureMipLevels);

        // We always set the layer count of the texture view to be 3 to match the fragment shader in
        // this test.
        constexpr uint32_t kTextureViewLayerCount = 3;
        ASSERT(textureArrayLayers >= textureViewBaseLayer + kTextureViewLayerCount);

        initTexture(textureArrayLayers, textureMipLevels);

        dawn::TextureViewDescriptor descriptor = mDefaultTextureViewDescriptor;
        descriptor.dimension = dawn::TextureViewDimension::e2DArray;
        descriptor.baseArrayLayer = textureViewBaseLayer;
        descriptor.layerCount = kTextureViewLayerCount;
        descriptor.baseMipLevel = textureViewBaseMipLevel;
        descriptor.levelCount = 1;
        dawn::TextureView textureView = mTexture.CreateTextureView(&descriptor);

        const char* fragmentShader = R"(
            #version 450
            layout(set = 0, binding = 0) uniform sampler sampler0;
            layout(set = 0, binding = 1) uniform texture2DArray texture0;
            layout(location = 0) in vec2 texCoord;
            layout(location = 0) out vec4 fragColor;

            void main() {
                fragColor =
                    texture(sampler2DArray(texture0, sampler0), vec3(texCoord, 0)) +
                    texture(sampler2DArray(texture0, sampler0), vec3(texCoord, 1)) +
                    texture(sampler2DArray(texture0, sampler0), vec3(texCoord, 2));
            }
        )";

        int expected = 0;
        for (int i = 0; i < static_cast<int>(kTextureViewLayerCount); ++i) {
            expected += GenerateTestPixelValue(textureViewBaseLayer + i, textureViewBaseMipLevel);
        }
        Verify(textureView, fragmentShader, expected);
    }

    std::string CreateFragmentShaderForCubeMapFace(uint32_t layer, bool isCubeMapArray) {
        // Reference: https://en.wikipedia.org/wiki/Cube_mapping
        const std::array<std::string, 6> kCoordsToCubeMapFace = {{
             " 1.f,   tc,  -sc",  // Positive X
             "-1.f,   tc,   sc",  // Negative X
             "  sc,  1.f,  -tc",  // Positive Y
             "  sc, -1.f,   tc",  // Negative Y
             "  sc,   tc,  1.f",  // Positive Z
             " -sc,   tc, -1.f",  // Negative Z
            }};

        const std::string textureType = isCubeMapArray ? "textureCubeArray" : "textureCube";
        const std::string samplerType = isCubeMapArray ? "samplerCubeArray" : "samplerCube";
        const uint32_t cubeMapArrayIndex = layer / 6;
        const std::string coordToCubeMapFace = kCoordsToCubeMapFace[layer % 6];

        std::ostringstream stream;
        stream << R"(
            #version 450
            layout(set = 0, binding = 0) uniform sampler sampler0;
            layout(set = 0, binding = 1) uniform )" << textureType << R"( texture0;
            layout(location = 0) in vec2 texCoord;
            layout(location = 0) out vec4 fragColor;
            void main() {
                float sc = 2.f * texCoord.x - 1.f;
                float tc = 2.f * texCoord.y - 1.f;
                fragColor = texture()" << samplerType << "(texture0, sampler0), ";

        if (isCubeMapArray) {
            stream << "vec4(" << coordToCubeMapFace << ", " << cubeMapArrayIndex;
        } else {
            stream << "vec3(" << coordToCubeMapFace;
        }

        stream << R"());
            })";

        return stream.str();
    }

    void TextureCubeMapTest(uint32_t textureArrayLayers,
                            uint32_t textureViewBaseLayer,
                            uint32_t textureViewLayerCount,
                            bool isCubeMapArray) {
        constexpr uint32_t kMipLevels = 1u;
        initTexture(textureArrayLayers, kMipLevels);

        ASSERT_TRUE((textureViewLayerCount == 6) ||
                    (isCubeMapArray && textureViewLayerCount % 6 == 0));

        dawn::TextureViewDescriptor descriptor = mDefaultTextureViewDescriptor;
        descriptor.dimension = (isCubeMapArray) ?
            dawn::TextureViewDimension::CubeArray : dawn::TextureViewDimension::Cube;
        descriptor.baseArrayLayer = textureViewBaseLayer;
        descriptor.layerCount = textureViewLayerCount;

        dawn::TextureView cubeMapTextureView = mTexture.CreateTextureView(&descriptor);

        // Check the data in the every face of the cube map (array) texture view.
        for (uint32_t layer = 0; layer < textureViewLayerCount; ++layer) {
            const std::string &fragmentShader =
                CreateFragmentShaderForCubeMapFace(layer, isCubeMapArray);

            int expected = GenerateTestPixelValue(textureViewBaseLayer + layer, 0);
            Verify(cubeMapTextureView, fragmentShader.c_str(), expected);
        }
    }

    dawn::BindGroupLayout mBindGroupLayout;
    dawn::PipelineLayout mPipelineLayout;
    dawn::Sampler mSampler;
    dawn::Texture mTexture;
    dawn::TextureViewDescriptor mDefaultTextureViewDescriptor;
    dawn::ShaderModule mVSModule;
    utils::BasicRenderPass mRenderPass;
};

// Test drawing a rect with a 2D array texture.
TEST_P(TextureViewTest, Default2DArrayTexture) {
    // TODO(cwallez@chromium.org) understand what the issue is
    DAWN_SKIP_TEST_IF(IsVulkan() && IsNvidia());

    constexpr uint32_t kLayers = 3;
    constexpr uint32_t kMipLevels = 1;
    initTexture(kLayers, kMipLevels);

    dawn::TextureView textureView = mTexture.CreateDefaultTextureView();

    const char* fragmentShader = R"(
            #version 450
            layout(set = 0, binding = 0) uniform sampler sampler0;
            layout(set = 0, binding = 1) uniform texture2DArray texture0;
            layout(location = 0) in vec2 texCoord;
            layout(location = 0) out vec4 fragColor;

            void main() {
                fragColor =
                    texture(sampler2DArray(texture0, sampler0), vec3(texCoord, 0)) +
                    texture(sampler2DArray(texture0, sampler0), vec3(texCoord, 1)) +
                    texture(sampler2DArray(texture0, sampler0), vec3(texCoord, 2));
            }
        )";

    const int expected = GenerateTestPixelValue(0, 0) + GenerateTestPixelValue(1, 0) +
                         GenerateTestPixelValue(2, 0);
    Verify(textureView, fragmentShader, expected);
}

// Test sampling from a 2D texture view created on a 2D array texture.
TEST_P(TextureViewTest, Texture2DViewOn2DArrayTexture) {
    Texture2DViewTest(6, 1, 4, 0);
}

// Test sampling from a 2D array texture view created on a 2D array texture.
TEST_P(TextureViewTest, Texture2DArrayViewOn2DArrayTexture) {
    DAWN_SKIP_TEST_IF(IsMetal());
    Texture2DArrayViewTest(6, 1, 2, 0);
}

// Test sampling from a 2D texture view created on a mipmap level of a 2D texture.
TEST_P(TextureViewTest, Texture2DViewOnOneLevelOf2DTexture) {
    Texture2DViewTest(1, 6, 0, 4);
}

// Test sampling from a 2D texture view created on a mipmap level of a 2D array texture layer.
TEST_P(TextureViewTest, Texture2DViewOnOneLevelOf2DArrayTexture) {
    Texture2DViewTest(6, 6, 3, 4);
}

// Test sampling from a 2D array texture view created on a mipmap level of a 2D array texture.
TEST_P(TextureViewTest, Texture2DArrayViewOnOneLevelOf2DArrayTexture) {
    DAWN_SKIP_TEST_IF(IsMetal());
    Texture2DArrayViewTest(6, 6, 2, 4);
}

// Test sampling from a cube map texture view that covers a whole 2D array texture.
TEST_P(TextureViewTest, TextureCubeMapOnWholeTexture) {
    constexpr uint32_t kTotalLayers = 6;
    TextureCubeMapTest(kTotalLayers, 0, kTotalLayers, false);
}

// Test sampling from a cube map texture view that covers a sub part of a 2D array texture.
TEST_P(TextureViewTest, TextureCubeMapViewOnPartOfTexture) {
    TextureCubeMapTest(10, 2, 6, false);
}

// Test sampling from a cube map texture view that covers the last layer of a 2D array texture.
TEST_P(TextureViewTest, TextureCubeMapViewCoveringLastLayer) {
    constexpr uint32_t kTotalLayers = 10;
    constexpr uint32_t kBaseLayer = 4;
    TextureCubeMapTest(kTotalLayers, kBaseLayer, kTotalLayers - kBaseLayer, false);
}

// Test sampling from a cube map texture array view that covers a whole 2D array texture.
TEST_P(TextureViewTest, TextureCubeMapArrayOnWholeTexture) {
    constexpr uint32_t kTotalLayers = 12;
    TextureCubeMapTest(kTotalLayers, 0, kTotalLayers, true);
}

// Test sampling from a cube map texture array view that covers a sub part of a 2D array texture.
TEST_P(TextureViewTest, TextureCubeMapArrayViewOnPartOfTexture) {
    TextureCubeMapTest(20, 3, 12, true);
}

// Test sampling from a cube map texture array view that covers the last layer of a 2D array texture.
TEST_P(TextureViewTest, TextureCubeMapArrayViewCoveringLastLayer) {
    constexpr uint32_t kTotalLayers = 20;
    constexpr uint32_t kBaseLayer = 8;
    TextureCubeMapTest(kTotalLayers, kBaseLayer, kTotalLayers - kBaseLayer, true);
}

// Test sampling from a cube map array texture view that only has a single cube map.
TEST_P(TextureViewTest, TextureCubeMapArrayViewSingleCubeMap) {
    TextureCubeMapTest(20, 7, 6, true);
}

DAWN_INSTANTIATE_TEST(TextureViewTest, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend)
