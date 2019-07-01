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

#include "common/Assert.h"
#include "common/Math.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/DawnHelpers.h"

#include <type_traits>

// An expectation for float buffer content that can correctly compare different NaN values and
// supports a basic tolerance for comparison of finite values.
class ExpectFloatWithTolerance : public detail::Expectation {
  public:
    ExpectFloatWithTolerance(std::vector<float> expected, float tolerance)
        : mExpected(std::move(expected)), mTolerance(tolerance) {
    }

    testing::AssertionResult Check(const void* data, size_t size) override {
        ASSERT(size == sizeof(float) * mExpected.size());

        const float* actual = static_cast<const float*>(data);

        for (size_t i = 0; i < mExpected.size(); ++i) {
            float expectedValue = mExpected[i];
            float actualValue = actual[i];

            if (!FloatsMatch(expectedValue, actualValue)) {
                testing::AssertionResult result = testing::AssertionFailure()
                                                  << "Expected data[" << i << "] to be close to "
                                                  << expectedValue << ", actual " << actualValue
                                                  << std::endl;
                return result;
            }
        }
        return testing::AssertionSuccess();
    }

  private:
    bool FloatsMatch(float expected, float actual) {
        if (std::isnan(expected)) {
            return std::isnan(actual);
        }

        if (std::isinf(expected)) {
            return std::isinf(actual) && std::signbit(expected) == std::signbit(actual);
        }

        if (mTolerance == 0.0f) {
            return expected == actual;
        }

        float error = std::abs(expected - actual);
        return error < mTolerance;
    }

    std::vector<float> mExpected;
    float mTolerance;
};

class TextureFormatTest : public DawnTest {
  protected:
    void SetUp() {
        DawnTest::SetUp();

        mSampleBGL = utils::MakeBindGroupLayout(
            device, {{0, dawn::ShaderStageBit::Fragment, dawn::BindingType::Sampler},
                     {1, dawn::ShaderStageBit::Fragment, dawn::BindingType::SampledTexture}});
    }

    // Describes what the "decompressed" data type for a texture format is. For example normalized
    // formats are stored as integers but interpreted to produce floating point values.
    enum ComponentType {
        Uint,
        Sint,
        Float,
    };

    // Structure containing all the information that tests need to know about the format.
    struct FormatTestInfo {
        dawn::TextureFormat format;
        uint32_t texelByteSize;
        ComponentType type;
        uint32_t componentCount;
    };

    // Returns a texture format that can be used to contain the interpreted value of the format in
    // formatInfo.
    dawn::TextureFormat GetComponentFormat(FormatTestInfo formatInfo) {
        std::array<dawn::TextureFormat, 4> floatFormats = {
            dawn::TextureFormat::R32Float,
            dawn::TextureFormat::RG32Float,
            dawn::TextureFormat::RGBA32Float,
            dawn::TextureFormat::RGBA32Float,
        };
        std::array<dawn::TextureFormat, 4> sintFormats = {
            dawn::TextureFormat::R32Sint,
            dawn::TextureFormat::RG32Sint,
            dawn::TextureFormat::RGBA32Sint,
            dawn::TextureFormat::RGBA32Sint,
        };
        std::array<dawn::TextureFormat, 4> uintFormats = {
            dawn::TextureFormat::R32Uint,
            dawn::TextureFormat::RG32Uint,
            dawn::TextureFormat::RGBA32Uint,
            dawn::TextureFormat::RGBA32Uint,
        };

        ASSERT(formatInfo.componentCount > 0 && formatInfo.componentCount <= 4);
        switch (formatInfo.type) {
            case Float:
                return floatFormats[formatInfo.componentCount - 1];
            case Sint:
                return sintFormats[formatInfo.componentCount - 1];
            case Uint:
                return uintFormats[formatInfo.componentCount - 1];
            default:
                UNREACHABLE();
                return dawn::TextureFormat::R32Float;
        }
    }

    // Return a pipeline that can be used in a full-texture draw to sample from the texture in the
    // bindgroup and output its decompressed values to the render target.
    dawn::RenderPipeline CreateSamplePipeline(FormatTestInfo formatInfo) {
        utils::ComboRenderPipelineDescriptor desc(device);

        dawn::ShaderModule vsModule =
            utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
            #version 450
            layout(location=0) out vec2 texCoord;
            void main() {
                const vec2 pos[3] = vec2[3](
                    vec2(-3.0f, -1.0f),
                    vec2( 3.0f, -1.0f),
                    vec2( 0.0f,  2.0f)
                );
                gl_Position = vec4(pos[gl_VertexIndex], 0.0f, 1.0f);
                texCoord = gl_Position.xy / 2.0f + vec2(0.5f);
            })");

        // Compute the prefix needed for GLSL types that handle our texture's data.
        const char* prefix = nullptr;
        switch (formatInfo.type) {
            case Float:
                prefix = "";
                break;
            case Sint:
                prefix = "i";
                break;
            case Uint:
                prefix = "u";
                break;
            default:
                UNREACHABLE();
                break;
        }

        std::ostringstream fsSource;
        fsSource << "#version 450\n";
        fsSource << "layout(set=0, binding=0) uniform sampler mySampler;\n";
        fsSource << "layout(set=0, binding=1) uniform " << prefix << "texture2D myTexture;\n";

        fsSource << "layout(location=0) in vec2 texCoord;\n";
        fsSource << "layout(location=0) out " << prefix << "vec4 fragColor;\n";

        fsSource << "void main() {\n";
        fsSource << "    fragColor = texture(" << prefix
                 << "sampler2D(myTexture, mySampler), texCoord);\n";
        fsSource << "}";

        dawn::ShaderModule fsModule =
            utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, fsSource.str().c_str());

        desc.cVertexStage.module = vsModule;
        desc.cFragmentStage.module = fsModule;
        desc.layout = utils::MakeBasicPipelineLayout(device, &mSampleBGL);
        desc.cColorStates[0]->format = GetComponentFormat(formatInfo);

        return device.CreateRenderPipeline(&desc);
    }

    // The sampling test uploads the textureData in a texture with the formatInfo.format format and
    // the samples from it, then checks that the sampled values correspond to expectedRenderData.
    void DoSampleTest(FormatTestInfo formatInfo,
                      const void* textureData,
                      size_t textureDataSize,
                      const void* expectedRenderData,
                      size_t expectedRenderDataSize,
                      float floatTolerance) {
        // The input data should contain an exact number of texels
        ASSERT(textureDataSize % formatInfo.texelByteSize == 0);
        uint32_t width = textureDataSize / formatInfo.texelByteSize;

        // The input data must be a multiple of 4 byte in length for setSubData
        ASSERT(textureDataSize % 4 == 0);

        // Create the texture we will sample from
        dawn::TextureDescriptor textureDesc;
        textureDesc.usage = dawn::TextureUsageBit::TransferDst | dawn::TextureUsageBit::Sampled;
        textureDesc.dimension = dawn::TextureDimension::e2D;
        textureDesc.size = {width, 1, 1};
        textureDesc.arrayLayerCount = 1;
        textureDesc.format = formatInfo.format;
        textureDesc.mipLevelCount = 1;
        textureDesc.sampleCount = 1;

        dawn::Texture texture = device.CreateTexture(&textureDesc);

        dawn::Buffer uploadBuffer = utils::CreateBufferFromData(
            device, textureData, textureDataSize, dawn::BufferUsageBit::TransferSrc);

        // Create the texture that we will render results to
        dawn::TextureDescriptor renderTargetDesc;
        renderTargetDesc.usage =
            dawn::TextureUsageBit::TransferSrc | dawn::TextureUsageBit::OutputAttachment;
        renderTargetDesc.dimension = dawn::TextureDimension::e2D;
        renderTargetDesc.size = {width, 1, 1};
        renderTargetDesc.arrayLayerCount = 1;
        renderTargetDesc.format = GetComponentFormat(formatInfo);
        renderTargetDesc.mipLevelCount = 1;
        renderTargetDesc.sampleCount = 1;

        dawn::Texture renderTarget = device.CreateTexture(&renderTargetDesc);

        // Create the readback buffer for the data in renderTarget
        dawn::BufferDescriptor readbackBufferDesc;
        readbackBufferDesc.usage =
            dawn::BufferUsageBit::TransferDst | dawn::BufferUsageBit::TransferSrc;
        readbackBufferDesc.size = 4 * width * formatInfo.componentCount;
        dawn::Buffer readbackBuffer = device.CreateBuffer(&readbackBufferDesc);

        // Prepare objects needed to sample from texture in the renderpass
        dawn::RenderPipeline pipeline = CreateSamplePipeline(formatInfo);
        dawn::SamplerDescriptor samplerDesc = utils::GetDefaultSamplerDescriptor();
        dawn::Sampler sampler = device.CreateSampler(&samplerDesc);
        dawn::BindGroup bindGroup = utils::MakeBindGroup(
            device, mSampleBGL, {{0, sampler}, {1, texture.CreateDefaultView()}});

        // Encode commands for the test that fill texture, sample it to render to renderTarget then
        // copy renderTarget in a buffer so we can read it easily.
        dawn::CommandEncoder encoder = device.CreateCommandEncoder();

        {
            dawn::BufferCopyView bufferView = utils::CreateBufferCopyView(uploadBuffer, 0, 256, 0);
            dawn::TextureCopyView textureView =
                utils::CreateTextureCopyView(texture, 0, 0, {0, 0, 0});
            dawn::Extent3D extent{width, 1, 1};
            encoder.CopyBufferToTexture(&bufferView, &textureView, &extent);
        }

        utils::ComboRenderPassDescriptor renderPassDesc({renderTarget.CreateDefaultView()});
        dawn::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDesc);
        renderPass.SetPipeline(pipeline);
        renderPass.SetBindGroup(0, bindGroup, 0, nullptr);
        renderPass.Draw(3, 1, 0, 0);
        renderPass.EndPass();

        {
            dawn::BufferCopyView bufferView =
                utils::CreateBufferCopyView(readbackBuffer, 0, 256, 0);
            dawn::TextureCopyView textureView =
                utils::CreateTextureCopyView(renderTarget, 0, 0, {0, 0, 0});
            dawn::Extent3D extent{width, 1, 1};
            encoder.CopyTextureToBuffer(&textureView, &bufferView, &extent);
        }

        dawn::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // For floats use a special expectation that understands how to compare NaNs and support a
        // tolerance.
        if (formatInfo.type == Float) {
            const float* expectedFloats = static_cast<const float*>(expectedRenderData);
            std::vector<float> expectedVector(
                expectedFloats, expectedFloats + expectedRenderDataSize / sizeof(float));
            AddBufferExpectation(__FILE__, __LINE__, readbackBuffer, 0, expectedRenderDataSize,
                                 new ExpectFloatWithTolerance(expectedVector, floatTolerance));
        } else {
            EXPECT_BUFFER_U32_RANGE_EQ(static_cast<const uint32_t*>(expectedRenderData),
                                       readbackBuffer, 0,
                                       expectedRenderDataSize / sizeof(uint32_t));
        }
    }

    template <typename TextureData, typename RenderData>
    void DoSampleTest(FormatTestInfo formatInfo,
                      const std::vector<TextureData>& textureData,
                      const std::vector<RenderData>& expectedRenderData,
                      float floatTolerance = 0.0f) {
        DoSampleTest(formatInfo, textureData.data(), textureData.size() * sizeof(TextureData),
                     expectedRenderData.data(), expectedRenderData.size() * sizeof(RenderData),
                     floatTolerance);
    }

    // Below are helper functions for types that are very similar to one another so the logic is
    // shared.

    template <typename T>
    void DoUnormTest(FormatTestInfo formatInfo) {
        static_assert(!std::is_signed<T>::value && std::is_integral<T>::value, "");
        ASSERT(sizeof(T) * formatInfo.componentCount == formatInfo.texelByteSize);
        ASSERT(formatInfo.type == Float);

        T maxValue = std::numeric_limits<T>::max();
        std::vector<T> textureData = {0, 1, maxValue, maxValue};
        std::vector<float> expectedData = {0.0f, 1.0f / maxValue, 1.0f, 1.0f};

        DoSampleTest(formatInfo, textureData, expectedData);
    }

    template <typename T>
    void DoSnormTest(FormatTestInfo formatInfo) {
        static_assert(std::is_signed<T>::value && std::is_integral<T>::value, "");
        ASSERT(sizeof(T) * formatInfo.componentCount == formatInfo.texelByteSize);
        ASSERT(formatInfo.type == Float);

        T maxValue = std::numeric_limits<T>::max();
        T minValue = std::numeric_limits<T>::min();
        std::vector<T> textureData = {0, 1, maxValue, minValue};
        std::vector<float> expectedData = {0.0f, 1.0f / maxValue, 1.0f, -1.0f};

        DoSampleTest(formatInfo, textureData, expectedData, 0.0001f / maxValue);
    }

    template <typename T>
    void DoUintTest(FormatTestInfo formatInfo) {
        static_assert(!std::is_signed<T>::value && std::is_integral<T>::value, "");
        ASSERT(sizeof(T) * formatInfo.componentCount == formatInfo.texelByteSize);
        ASSERT(formatInfo.type == Uint);

        T maxValue = std::numeric_limits<T>::max();
        std::vector<T> textureData = {0, 1, maxValue, maxValue};
        std::vector<uint32_t> expectedData = {0, 1, maxValue, maxValue};

        DoSampleTest(formatInfo, textureData, expectedData);
    }

    template <typename T>
    void DoSintTest(FormatTestInfo formatInfo) {
        static_assert(std::is_signed<T>::value && std::is_integral<T>::value, "");
        ASSERT(sizeof(T) * formatInfo.componentCount == formatInfo.texelByteSize);
        ASSERT(formatInfo.type == Sint);

        T maxValue = std::numeric_limits<T>::max();
        T minValue = std::numeric_limits<T>::min();
        std::vector<T> textureData = {0, 1, maxValue, minValue};
        std::vector<int32_t> expectedData = {0, 1, maxValue, minValue};

        DoSampleTest(formatInfo, textureData, expectedData);
    }

    void DoFloat32Test(FormatTestInfo formatInfo) {
        ASSERT(sizeof(float) * formatInfo.componentCount == formatInfo.texelByteSize);
        ASSERT(formatInfo.type == Float);

        std::vector<float> textureData = {+0.0f,  -0.0f, 1.0f,     1.0e-29,
                                          1.0e29, NAN,   INFINITY, -INFINITY};

        DoSampleTest(formatInfo, textureData, textureData);
    }

    void DoFloat16Test(FormatTestInfo formatInfo) {
        ASSERT(sizeof(int16_t) * formatInfo.componentCount == formatInfo.texelByteSize);
        ASSERT(formatInfo.type == Float);

        std::vector<float> expectedData = {+0.0f, -0.0f, 1.0f,     1.0e-4,
                                           1.0e4, NAN,   INFINITY, -INFINITY};
        std::vector<uint16_t> textureData;
        for (float value : expectedData) {
            textureData.push_back(Float32ToFloat16(value));
        }

        DoSampleTest(formatInfo, textureData, expectedData, 1.0e-5);
    }

  private:
    dawn::BindGroupLayout mSampleBGL;
};

// Test the R8Unorm format
TEST_P(TextureFormatTest, R8Unorm) {
    DoUnormTest<uint8_t>({dawn::TextureFormat::R8Unorm, 1, Float, 1});
}

// Test the RG8Unorm format
TEST_P(TextureFormatTest, RG8Unorm) {
    DoUnormTest<uint8_t>({dawn::TextureFormat::RG8Unorm, 2, Float, 2});
}

// Test the RGBA8Unorm format
TEST_P(TextureFormatTest, RGBA8Unorm) {
    DoUnormTest<uint8_t>({dawn::TextureFormat::RGBA8Unorm, 4, Float, 4});
}

// Test the R16Unorm format
TEST_P(TextureFormatTest, R16Unorm) {
    DoUnormTest<uint16_t>({dawn::TextureFormat::R16Unorm, 2, Float, 1});
}

// Test the RG16Unorm format
TEST_P(TextureFormatTest, RG16Unorm) {
    DoUnormTest<uint16_t>({dawn::TextureFormat::RG16Unorm, 4, Float, 2});
}

// Test the RGBA16Unorm format
TEST_P(TextureFormatTest, RGBA16Unorm) {
    DoUnormTest<uint16_t>({dawn::TextureFormat::RGBA16Unorm, 8, Float, 4});
}

// Test the BGRA8Unorm format
TEST_P(TextureFormatTest, BGRA8Unorm) {
    uint8_t maxValue = std::numeric_limits<uint8_t>::max();
    std::vector<uint8_t> textureData = {maxValue, 1, 0, maxValue};
    std::vector<float> expectedData = {0.0f, 1.0f / maxValue, 1.0f, 1.0f};
    DoSampleTest({dawn::TextureFormat::BGRA8Unorm, 4, Float, 4}, textureData, expectedData);
}

// Test the R8Snorm format
TEST_P(TextureFormatTest, R8Snorm) {
    DoSnormTest<int8_t>({dawn::TextureFormat::R8Snorm, 1, Float, 1});
}

// Test the RG8Snorm format
TEST_P(TextureFormatTest, RG8Snorm) {
    DoSnormTest<int8_t>({dawn::TextureFormat::RG8Snorm, 2, Float, 2});
}

// Test the RGBA8Snorm format
TEST_P(TextureFormatTest, RGBA8Snorm) {
    DoSnormTest<int8_t>({dawn::TextureFormat::RGBA8Snorm, 4, Float, 4});
}

// Test the R16Snorm format
TEST_P(TextureFormatTest, R16Snorm) {
    DoSnormTest<int16_t>({dawn::TextureFormat::R16Snorm, 2, Float, 1});
}

// Test the RG16Snorm format
TEST_P(TextureFormatTest, RG16Snorm) {
    DoSnormTest<int16_t>({dawn::TextureFormat::RG16Snorm, 4, Float, 2});
}

// Test the RGBA16Snorm format
TEST_P(TextureFormatTest, RGBA16Snorm) {
    DoSnormTest<int16_t>({dawn::TextureFormat::RGBA16Snorm, 8, Float, 4});
}

// Test the R8Uint format
TEST_P(TextureFormatTest, R8Uint) {
    DoUintTest<uint8_t>({dawn::TextureFormat::R8Uint, 1, Uint, 1});
}

// Test the RG8Uint format
TEST_P(TextureFormatTest, RG8Uint) {
    DoUintTest<uint8_t>({dawn::TextureFormat::RG8Uint, 2, Uint, 2});
}

// Test the RGBA8Uint format
TEST_P(TextureFormatTest, RGBA8Uint) {
    DoUintTest<uint8_t>({dawn::TextureFormat::RGBA8Uint, 4, Uint, 4});
}

// Test the R16Uint format
TEST_P(TextureFormatTest, R16Uint) {
    DoUintTest<uint16_t>({dawn::TextureFormat::R16Uint, 2, Uint, 1});
}

// Test the RG16Uint format
TEST_P(TextureFormatTest, RG16Uint) {
    DoUintTest<uint16_t>({dawn::TextureFormat::RG16Uint, 4, Uint, 2});
}

// Test the RGBA16Uint format
TEST_P(TextureFormatTest, RGBA16Uint) {
    DoUintTest<uint16_t>({dawn::TextureFormat::RGBA16Uint, 8, Uint, 4});
}

// Test the R32Uint format
TEST_P(TextureFormatTest, R32Uint) {
    DoUintTest<uint32_t>({dawn::TextureFormat::R32Uint, 4, Uint, 1});
}

// Test the RG32Uint format
TEST_P(TextureFormatTest, RG32Uint) {
    DoUintTest<uint32_t>({dawn::TextureFormat::RG32Uint, 8, Uint, 2});
}

// Test the RGBA32Uint format
TEST_P(TextureFormatTest, RGBA32Uint) {
    DoUintTest<uint32_t>({dawn::TextureFormat::RGBA32Uint, 16, Uint, 4});
}

// Test the R8Sint format
TEST_P(TextureFormatTest, R8Sint) {
    DoSintTest<int8_t>({dawn::TextureFormat::R8Sint, 1, Sint, 1});
}

// Test the RG8Sint format
TEST_P(TextureFormatTest, RG8Sint) {
    DoSintTest<int8_t>({dawn::TextureFormat::RG8Sint, 2, Sint, 2});
}

// Test the RGBA8Sint format
TEST_P(TextureFormatTest, RGBA8Sint) {
    DoSintTest<int8_t>({dawn::TextureFormat::RGBA8Sint, 4, Sint, 4});
}

// Test the R16Sint format
TEST_P(TextureFormatTest, R16Sint) {
    DoSintTest<int16_t>({dawn::TextureFormat::R16Sint, 2, Sint, 1});
}

// Test the RG16Sint format
TEST_P(TextureFormatTest, RG16Sint) {
    DoSintTest<int16_t>({dawn::TextureFormat::RG16Sint, 4, Sint, 2});
}

// Test the RGBA16Sint format
TEST_P(TextureFormatTest, RGBA16Sint) {
    DoSintTest<int16_t>({dawn::TextureFormat::RGBA16Sint, 8, Sint, 4});
}

// Test the R32Sint format
TEST_P(TextureFormatTest, R32Sint) {
    DoSintTest<int32_t>({dawn::TextureFormat::R32Sint, 4, Sint, 1});
}

// Test the RG32Sint format
TEST_P(TextureFormatTest, RG32Sint) {
    DoSintTest<int32_t>({dawn::TextureFormat::RG32Sint, 8, Sint, 2});
}

// Test the RGBA32Sint format
TEST_P(TextureFormatTest, RGBA32Sint) {
    DoSintTest<int32_t>({dawn::TextureFormat::RGBA32Sint, 16, Sint, 4});
}

// Test the R32Float format
TEST_P(TextureFormatTest, R32Float) {
    DoFloat32Test({dawn::TextureFormat::R32Float, 4, Float, 1});
}

// Test the RG32Float format
TEST_P(TextureFormatTest, RG32Float) {
    DoFloat32Test({dawn::TextureFormat::RG32Float, 8, Float, 2});
}

// Test the RGBA32Float format
TEST_P(TextureFormatTest, RGBA32Float) {
    DoFloat32Test({dawn::TextureFormat::RGBA32Float, 16, Float, 4});
}

// Test the R16Float format
TEST_P(TextureFormatTest, R16Float) {
    DoFloat16Test({dawn::TextureFormat::R16Float, 2, Float, 1});
}

// Test the RG16Float format
TEST_P(TextureFormatTest, RG16Float) {
    DoFloat16Test({dawn::TextureFormat::RG16Float, 4, Float, 2});
}

// Test the RGBA16Float format
TEST_P(TextureFormatTest, RGBA16Float) {
    DoFloat16Test({dawn::TextureFormat::RGBA16Float, 8, Float, 4});
}

// Test the RGBA8Unorm format
TEST_P(TextureFormatTest, RGBA8UnormSrgb) {
    uint8_t maxValue = std::numeric_limits<uint8_t>::max();
    std::vector<uint8_t> textureData = {0, 1, maxValue, 64, 35, 68, 152, 168};

    std::vector<float> expectedData;
    for (size_t i = 0; i < textureData.size(); i += 4) {
        expectedData.push_back(SRGBToLinear(textureData[i + 0] / float(maxValue)));
        expectedData.push_back(SRGBToLinear(textureData[i + 1] / float(maxValue)));
        expectedData.push_back(SRGBToLinear(textureData[i + 2] / float(maxValue)));
        // Alpha is linear for sRGB formats
        expectedData.push_back(textureData[i + 3] / float(maxValue));
    }

    DoSampleTest({dawn::TextureFormat::RGBA8UnormSrgb, 4, Float, 4}, textureData, expectedData,
                 1.0e-3);
}

// Test the BGRA8Unorm format
TEST_P(TextureFormatTest, BGRA8UnormSrgb) {
    uint8_t maxValue = std::numeric_limits<uint8_t>::max();
    std::vector<uint8_t> textureData = {0, 1, maxValue, 64, 35, 68, 152, 168};

    std::vector<float> expectedData;
    for (size_t i = 0; i < textureData.size(); i += 4) {
        // Note that R and B are swapped
        expectedData.push_back(SRGBToLinear(textureData[i + 2] / float(maxValue)));
        expectedData.push_back(SRGBToLinear(textureData[i + 1] / float(maxValue)));
        expectedData.push_back(SRGBToLinear(textureData[i + 0] / float(maxValue)));
        // Alpha is linear for sRGB formats
        expectedData.push_back(textureData[i + 3] / float(maxValue));
    }

    DoSampleTest({dawn::TextureFormat::BGRA8UnormSrgb, 4, Float, 4}, textureData, expectedData,
                 1.0e-3);
}

// Test the B5G6R5Unorm format
TEST_P(TextureFormatTest, B5G6R5Unorm) {
    auto MakeBGR565 = [](uint32_t r, uint32_t g, uint32_t b) -> uint16_t {
        ASSERT((r & 0x1F) == r);
        ASSERT((g & 0x3F) == g);
        ASSERT((b & 0x1F) == b);
        return b << 11 | g << 5 | r;
    };

    std::vector<uint16_t> textureData = {MakeBGR565(0, 0, 0), MakeBGR565(31, 63, 31),
                                         MakeBGR565(9, 18, 27), MakeBGR565(0, 0, 0)};
    // This is one of the only 3-channel formats, so we don't have specific testing for them. Alpha
    // should slways be sampled as 1
    // clang-format off
    std::vector<float> expectedData = {
        0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        9 / 31.0f, 18 / 63.0f, 27 / 31.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    // clang-format on

    DoSampleTest({dawn::TextureFormat::B5G6R5Unorm, 2, Float, 4}, textureData, expectedData,
                 1.0e-3);
}

// Test the A2RGB10Unorm format
// TODO(cwallez@chromium.org): This is actually RGB10A2 in WebGPU but Vulkan doesn't support that
// format. Do all platforms have A in the high bits?
TEST_P(TextureFormatTest, A2RGB10Unorm) {
    auto MakeA2RGB10 = [](uint32_t r, uint32_t g, uint32_t b, uint32_t a) -> uint32_t {
        ASSERT((r & 0x3FF) == r);
        ASSERT((g & 0x3FF) == g);
        ASSERT((b & 0x3FF) == b);
        ASSERT((a & 0x3) == a);
        return a << 30 | r << 20 | g << 10 | b;
    };

    std::vector<uint32_t> textureData = {MakeA2RGB10(0, 0, 0, 0), MakeA2RGB10(1023, 1023, 1023, 1),
                                         MakeA2RGB10(243, 578, 765, 2), MakeA2RGB10(0, 0, 0, 3)};
    // clang-format off
    std::vector<float> expectedData = {
        0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1 / 3.0f,
        243 / 1023.0f, 576 / 1023.0f, 765 / 1023.0f, 2 / 3.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    // clang-format on

    DoSampleTest({dawn::TextureFormat::A2RGB10Unorm, 4, Float, 4}, textureData, expectedData,
                 2.0e-3);
}

// Test the B10GR11Float format
// TODO(cwallez@chromium.org): This is actually GR11B10 in WebGPU but Vulkan doesn't support that
// format. Do all platforms have it reversed?
TEST_P(TextureFormatTest, B10GR11Float) {
    constexpr uint32_t kFloat11Zero = 0;
    constexpr uint32_t kFloat11Infinity = 0x7C0;
    constexpr uint32_t kFloat11Nan = 0x7C1;
    constexpr uint32_t kFloat11One = 0x3C0;

    constexpr uint32_t kFloat10Zero = 0;
    constexpr uint32_t kFloat10Infinity = 0x3E0;
    constexpr uint32_t kFloat10Nan = 0x3E1;
    constexpr uint32_t kFloat10One = 0x1E0;

    auto MakeB10GR11 = [](uint32_t r, uint32_t g, uint32_t b) {
        ASSERT((r & 0x7FF) == r);
        ASSERT((g & 0x7FF) == g);
        ASSERT((b & 0x3FF) == b);
        return b << 22 | g << 11 | r;
    };

    // Test each of (0, 1, INFINITY, NaN) for each component but never two with the same value at a
    // time.
    std::vector<uint32_t> textureData = {
        MakeB10GR11(kFloat11Zero, kFloat11Infinity, kFloat10Nan),
        MakeB10GR11(kFloat11Infinity, kFloat11Nan, kFloat10One),
        MakeB10GR11(kFloat11Nan, kFloat11One, kFloat10Zero),
        MakeB10GR11(kFloat11One, kFloat11Zero, kFloat10Infinity),
    };

    // This is one of the only 3-channel formats, so we don't have specific testing for them. Alpha
    // should slways be sampled as 1
    // clang-format off
    std::vector<float> expectedData = {
        0.0f,     INFINITY, NAN,      1.0f,
        INFINITY, NAN,      1.0f,     1.0f,
        NAN,      1.0f,     0.0f,     1.0f,
        1.0f,     0.0f,     INFINITY, 1.0f
    };
    // clang-format on

    DoSampleTest({dawn::TextureFormat::B10GR11Float, 4, Float, 4}, textureData, expectedData);
}

// TODO(cwallez@chromium.org): Add tests for depth-stencil formats when we know if they are copyable
// in WebGPU.

DAWN_INSTANTIATE_TEST(TextureFormatTest, VulkanBackend);
