// Copyright 2020 The Dawn Authors
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

#include "common/Constants.h"
#include "common/Math.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/TestUtils.h"
#include "utils/TextureFormatUtils.h"
#include "utils/WGPUHelpers.h"

namespace {
    static constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::RGBA8Unorm;

    // Set default texture size to single line texture for color conversion tests.
    static constexpr uint64_t kDefaultTextureWidth = 10;
    static constexpr uint64_t kDefaultTextureHeight = 1;

    // Dst texture format copyTextureForBrowser accept
    static const wgpu::TextureFormat kDstTextureFormat[] = {
        wgpu::TextureFormat::RGBA8Unorm,  wgpu::TextureFormat::BGRA8Unorm,
        wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::RG8Unorm,
        wgpu::TextureFormat::RGBA16Float, wgpu::TextureFormat::RG16Float,
        wgpu::TextureFormat::RGB10A2Unorm};

    static const wgpu::Origin3D kOrigins[] = {{1, 1}, {1, 2}, {2, 1}};

    static const wgpu::Extent3D kCopySize[] = {{1, 1}, {2, 1}, {1, 2}, {2, 2}};
}  // anonymous namespace

class CopyTextureForBrowserTests : public DawnTest {
  protected:
    struct TextureSpec {
        wgpu::Origin3D copyOrigin = {};
        wgpu::Extent3D textureSize = {kDefaultTextureWidth, kDefaultTextureHeight};
        uint32_t level = 0;
        wgpu::TextureFormat format = kTextureFormat;
    };

    // This fixed source texture data is for color conversion tests.
    // The source data can fill a texture in default width and height.
    static std::vector<RGBA8> GetFixedSourceTextureData() {
        std::vector<RGBA8> sourceTextureData{
            // Take RGBA8Unorm as example:
            // R channel has different values
            RGBA8(0, 255, 255, 255),    // r = 0.0
            RGBA8(102, 255, 255, 255),  // r = 0.4
            RGBA8(153, 255, 255, 255),  // r = 0.6

            // G channel has different values
            RGBA8(255, 0, 255, 255),    // g = 0.0
            RGBA8(255, 102, 255, 255),  // g = 0.4
            RGBA8(255, 153, 255, 255),  // g = 0.6

            // B channel has different values
            RGBA8(255, 255, 0, 255),    // b = 0.0
            RGBA8(255, 255, 102, 255),  // b = 0.4
            RGBA8(255, 255, 153, 255),  // b = 0.6

            // A channel set to 0
            RGBA8(255, 255, 255, 0)  // a = 0
        };

        return sourceTextureData;
    }

    enum class TextureCopyRole {
        SOURCE,
        DEST,
    };

    // Source texture contains red pixels and dst texture contains green pixels at start.
    static std::vector<RGBA8> GetTextureData(const utils::TextureDataCopyLayout& layout,
                                             TextureCopyRole textureRole) {
        std::vector<RGBA8> textureData(layout.texelBlockCount);
        for (uint32_t layer = 0; layer < layout.mipSize.depthOrArrayLayers; ++layer) {
            const uint32_t sliceOffset = layout.texelBlocksPerImage * layer;
            for (uint32_t y = 0; y < layout.mipSize.height; ++y) {
                const uint32_t rowOffset = layout.texelBlocksPerRow * y;
                for (uint32_t x = 0; x < layout.mipSize.width; ++x) {
                    // Source textures will have variable pixel data to cover cases like
                    // flipY.
                    if (textureRole == TextureCopyRole::SOURCE) {
                        textureData[sliceOffset + rowOffset + x] =
                            RGBA8(static_cast<uint8_t>((x + layer * x) % 256),
                                  static_cast<uint8_t>((y + layer * y) % 256),
                                  static_cast<uint8_t>(x % 256), static_cast<uint8_t>(x % 256));
                    } else {  // Dst textures will have be init as `green` to ensure subrect
                              // copy not cross bound.
                        textureData[sliceOffset + rowOffset + x] =
                            RGBA8(static_cast<uint8_t>(0), static_cast<uint8_t>(255),
                                  static_cast<uint8_t>(0), static_cast<uint8_t>(255));
                    }
                }
            }
        }

        return textureData;
    }

    void SetUp() override {
        DawnTest::SetUp();

        // TODO(crbug.com/tint/682): error: runtime array not supported yet
        DAWN_SKIP_TEST_IF(IsD3D12() && HasToggleEnabled("use_tint_generator"));

        testPipeline = MakeTestPipeline();

        uint32_t uniformBufferData[] = {
            0,     // copy have flipY option
            4,     // channelCount
            0, 0,  // uvec2, subrect copy src origin
            0, 0,  // uvec2, subrect copy dst origin
            0, 0,  // uvec2, subrect copy size
        };

        wgpu::BufferDescriptor uniformBufferDesc = {};
        uniformBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
        uniformBufferDesc.size = sizeof(uniformBufferData);
        uniformBuffer = device.CreateBuffer(&uniformBufferDesc);
    }

    // Do the bit-by-bit comparison between the source and destination texture with GPU (compute
    // shader) instead of CPU after executing CopyTextureForBrowser() to avoid the errors caused by
    // comparing a value generated on CPU to the one generated on GPU.
    wgpu::ComputePipeline MakeTestPipeline() {
        wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
            [[block]] struct Uniforms {
                dstTextureFlipY : u32;
                channelCount    : u32;
                srcCopyOrigin   : vec2<u32>;
                dstCopyOrigin   : vec2<u32>;
                copySize        : vec2<u32>;
            };
            [[block]] struct OutputBuf {
                result : array<u32>;
            };
            [[group(0), binding(0)]] var src : texture_2d<f32>;
            [[group(0), binding(1)]] var dst : texture_2d<f32>;
            [[group(0), binding(2)]] var<storage> output : [[access(read_write)]] OutputBuf;
            [[group(0), binding(3)]] var<uniform> uniforms : Uniforms;
            [[builtin(global_invocation_id)]] var<in> GlobalInvocationID : vec3<u32>;
            fn aboutEqual(value : f32, expect : f32) -> bool {
                // The value diff should be smaller than the hard coded tolerance.
                return abs(value - expect) < 0.001;
            }
            [[stage(compute), workgroup_size(1, 1, 1)]] fn main() -> void {
                var srcSize : vec2<i32> = textureDimensions(src);
                var dstSize : vec2<i32> = textureDimensions(dst);
                var dstTexCoord : vec2<u32> = vec2<u32>(GlobalInvocationID.xy);
                var nonCoveredColor : vec4<f32> =
                    vec4<f32>(0.0, 1.0, 0.0, 1.0); // should be green

                var success : bool = true;
                if (dstTexCoord.x < uniforms.dstCopyOrigin.x ||
                    dstTexCoord.y < uniforms.dstCopyOrigin.y ||
                    dstTexCoord.x >= uniforms.dstCopyOrigin.x + uniforms.copySize.x ||
                    dstTexCoord.y >= uniforms.dstCopyOrigin.y + uniforms.copySize.y) {
                    success = success &&
                              all(textureLoad(dst, vec2<i32>(dstTexCoord), 0) == nonCoveredColor);
                } else {
                    // Calculate source texture coord.
                    var srcTexCoord : vec2<u32> = dstTexCoord - uniforms.dstCopyOrigin +
                                                  uniforms.srcCopyOrigin;
                    // Note that |flipY| equals flip src texture firstly and then do copy from src
                    // subrect to dst subrect. This helps on blink part to handle some input texture
                    // which is flipped and need to unpack flip during the copy.
                    // We need to calculate the expect y coord based on this rule.
                    if (uniforms.dstTextureFlipY == 1u) {
                        srcTexCoord.y = u32(srcSize.y) - srcTexCoord.y - 1u;
                    }

                    var srcColor : vec4<f32> = textureLoad(src, vec2<i32>(srcTexCoord), 0);
                    var dstColor : vec4<f32> = textureLoad(dst, vec2<i32>(dstTexCoord), 0);

                    // Not use loop and variable index format to workaround
                    // crbug.com/tint/638.
                    if (uniforms.channelCount == 2u) { // All have rg components.
                        success = success &&
                                  aboutEqual(dstColor.r, srcColor.r) &&
                                  aboutEqual(dstColor.g, srcColor.g);
                    } else {
                        success = success &&
                                  aboutEqual(dstColor.r, srcColor.r) &&
                                  aboutEqual(dstColor.g, srcColor.g) &&
                                  aboutEqual(dstColor.b, srcColor.b) &&
                                  aboutEqual(dstColor.a, srcColor.a);
                    }
                }
                var outputIndex : u32 = GlobalInvocationID.y * u32(dstSize.x) +
                                        GlobalInvocationID.x;
                if (success) {
                    output.result[outputIndex] = 1u;
                } else {
                    output.result[outputIndex] = 0u;
                }
            }
         )");

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.computeStage.module = csModule;
        csDesc.computeStage.entryPoint = "main";

        return device.CreateComputePipeline(&csDesc);
    }
    static uint32_t GetTextureFormatComponentCount(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::BGRA8Unorm:
            case wgpu::TextureFormat::RGB10A2Unorm:
            case wgpu::TextureFormat::RGBA16Float:
            case wgpu::TextureFormat::RGBA32Float:
                return 4;
            case wgpu::TextureFormat::RG8Unorm:
            case wgpu::TextureFormat::RG16Float:
                return 2;
            default:
                UNREACHABLE();
        }
    }

    void DoColorConversionTest(const TextureSpec& srcSpec, const TextureSpec& dstSpec) {
        DoTest(srcSpec, dstSpec, {kDefaultTextureWidth, kDefaultTextureHeight}, {}, true);
    }

    void DoTest(const TextureSpec& srcSpec,
                const TextureSpec& dstSpec,
                const wgpu::Extent3D& copySize = {kDefaultTextureWidth, kDefaultTextureHeight},
                const wgpu::CopyTextureForBrowserOptions options = {},
                bool useFixedTestValue = false) {
        // Create and initialize src texture.
        wgpu::TextureDescriptor srcDescriptor;
        srcDescriptor.size = srcSpec.textureSize;
        srcDescriptor.format = srcSpec.format;
        srcDescriptor.mipLevelCount = srcSpec.level + 1;
        srcDescriptor.usage =
            wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::Sampled;
        wgpu::Texture srcTexture = device.CreateTexture(&srcDescriptor);

        const utils::TextureDataCopyLayout srcCopyLayout =
            utils::GetTextureDataCopyLayoutForTexture2DAtLevel(
                kTextureFormat,
                {srcSpec.textureSize.width, srcSpec.textureSize.height,
                 copySize.depthOrArrayLayers},
                srcSpec.level);

        std::vector<RGBA8> srcTextureArrayCopyData;
        if (useFixedTestValue) {  // Use fixed value for color conversion tests.
            srcTextureArrayCopyData = GetFixedSourceTextureData();
        } else {  // For other tests, the input format is always kTextureFormat.

            srcTextureArrayCopyData = GetTextureData(srcCopyLayout, TextureCopyRole::SOURCE);
        }

        wgpu::ImageCopyTexture srcImageTextureInit =
            utils::CreateImageCopyTexture(srcTexture, srcSpec.level, {0, 0});

        wgpu::TextureDataLayout srcTextureDataLayout;
        srcTextureDataLayout.offset = 0;
        srcTextureDataLayout.bytesPerRow = srcCopyLayout.bytesPerRow;
        srcTextureDataLayout.rowsPerImage = srcCopyLayout.rowsPerImage;

        device.GetQueue().WriteTexture(&srcImageTextureInit, srcTextureArrayCopyData.data(),
                                       srcTextureArrayCopyData.size() * sizeof(RGBA8),
                                       &srcTextureDataLayout, &srcCopyLayout.mipSize);

        bool testSubRectCopy = srcSpec.copyOrigin.x > 0 || srcSpec.copyOrigin.y > 0 ||
                               dstSpec.copyOrigin.x > 0 || dstSpec.copyOrigin.y > 0 ||
                               srcSpec.textureSize.width > copySize.width ||
                               srcSpec.textureSize.height > copySize.height ||
                               dstSpec.textureSize.width > copySize.width ||
                               dstSpec.textureSize.height > copySize.height;

        // Create and init dst texture.
        wgpu::Texture dstTexture;
        wgpu::TextureDescriptor dstDescriptor;
        dstDescriptor.size = dstSpec.textureSize;
        dstDescriptor.format = dstSpec.format;
        dstDescriptor.mipLevelCount = dstSpec.level + 1;
        dstDescriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::Sampled |
                              wgpu::TextureUsage::OutputAttachment | wgpu::TextureUsage::CopySrc;
        dstTexture = device.CreateTexture(&dstDescriptor);

        if (testSubRectCopy) {
            // For subrect copy tests, dst texture use kTextureFormat always.
            const utils::TextureDataCopyLayout dstCopyLayout =
                utils::GetTextureDataCopyLayoutForTexture2DAtLevel(
                    kTextureFormat,
                    {dstSpec.textureSize.width, dstSpec.textureSize.height,
                     copySize.depthOrArrayLayers},
                    dstSpec.level);

            const std::vector<RGBA8> dstTextureArrayCopyData =
                GetTextureData(dstCopyLayout, TextureCopyRole::DEST);

            wgpu::TextureDataLayout dstTextureDataLayout;
            dstTextureDataLayout.offset = 0;
            dstTextureDataLayout.bytesPerRow = dstCopyLayout.bytesPerRow;
            dstTextureDataLayout.rowsPerImage = dstCopyLayout.rowsPerImage;

            wgpu::ImageCopyTexture dstImageTextureInit =
                utils::CreateImageCopyTexture(dstTexture, dstSpec.level, {0, 0});

            device.GetQueue().WriteTexture(&dstImageTextureInit, dstTextureArrayCopyData.data(),
                                           dstTextureArrayCopyData.size() * sizeof(RGBA8),
                                           &dstTextureDataLayout, &dstCopyLayout.mipSize);
        }

        // Perform the texture to texture copy
        wgpu::ImageCopyTexture srcImageCopyTexture =
            utils::CreateImageCopyTexture(srcTexture, srcSpec.level, srcSpec.copyOrigin);
        wgpu::ImageCopyTexture dstImageCopyTexture =
            utils::CreateImageCopyTexture(dstTexture, dstSpec.level, dstSpec.copyOrigin);
        device.GetQueue().CopyTextureForBrowser(&srcImageCopyTexture, &dstImageCopyTexture,
                                                &copySize, &options);

        // Update uniform buffer based on test config
        uint32_t uniformBufferData[] = {
            options.flipY,                                   // copy have flipY option
            GetTextureFormatComponentCount(dstSpec.format),  // channelCount
            srcSpec.copyOrigin.x,
            srcSpec.copyOrigin.y,  // src texture copy origin
            dstSpec.copyOrigin.x,
            dstSpec.copyOrigin.y,  // dst texture copy origin
            copySize.width,
            copySize.height  // copy size
        };

        device.GetQueue().WriteBuffer(uniformBuffer, 0, uniformBufferData,
                                      sizeof(uniformBufferData));

        // Create output buffer to store result
        wgpu::BufferDescriptor outputDesc;
        outputDesc.size = dstSpec.textureSize.width * dstSpec.textureSize.height * sizeof(uint32_t);
        outputDesc.usage =
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer outputBuffer = device.CreateBuffer(&outputDesc);

        // Create texture views for test.
        wgpu::TextureViewDescriptor srcTextureViewDesc = {};
        srcTextureViewDesc.baseMipLevel = srcSpec.level;
        wgpu::TextureView srcTextureView = srcTexture.CreateView(&srcTextureViewDesc);

        wgpu::TextureViewDescriptor dstTextureViewDesc = {};
        dstTextureViewDesc.baseMipLevel = dstSpec.level;
        wgpu::TextureView dstTextureView = dstTexture.CreateView(&dstTextureViewDesc);

        // Create bind group based on the config.
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, testPipeline.GetBindGroupLayout(0),
            {{0, srcTextureView}, {1, dstTextureView}, {2, outputBuffer}, {3, uniformBuffer}});

        // Start a pipeline to check pixel value in bit form.
        wgpu::CommandEncoder testEncoder = device.CreateCommandEncoder();

        wgpu::CommandBuffer testCommands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(testPipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Dispatch(dstSpec.textureSize.width,
                          dstSpec.textureSize.height);  // Verify dst texture content
            pass.EndPass();

            testCommands = encoder.Finish();
        }
        queue.Submit(1, &testCommands);

        std::vector<uint32_t> expectResult(dstSpec.textureSize.width * dstSpec.textureSize.height,
                                           1);
        EXPECT_BUFFER_U32_RANGE_EQ(expectResult.data(), outputBuffer, 0,
                                   dstSpec.textureSize.width * dstSpec.textureSize.height);
    }

    wgpu::Buffer uniformBuffer;  // Uniform buffer to store dst texture meta info.
    wgpu::ComputePipeline testPipeline;
};

// Verify CopyTextureForBrowserTests works with internal pipeline.
// The case do copy without any transform.
TEST_P(CopyTextureForBrowserTests, PassthroughCopy) {
    // Tests skip due to crbug.com/dawn/592.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    constexpr uint32_t kWidth = 10;
    constexpr uint32_t kHeight = 1;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight};

    DoTest(textureSpec, textureSpec, {kWidth, kHeight});
}

TEST_P(CopyTextureForBrowserTests, VerifyCopyOnXDirection) {
    // Tests skip due to crbug.com/dawn/592.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    constexpr uint32_t kWidth = 1000;
    constexpr uint32_t kHeight = 1;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight};

    DoTest(textureSpec, textureSpec, {kWidth, kHeight});
}

TEST_P(CopyTextureForBrowserTests, VerifyCopyOnYDirection) {
    // Tests skip due to crbug.com/dawn/592.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    constexpr uint32_t kWidth = 1;
    constexpr uint32_t kHeight = 1000;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight};

    DoTest(textureSpec, textureSpec, {kWidth, kHeight});
}

TEST_P(CopyTextureForBrowserTests, VerifyCopyFromLargeTexture) {
    // Tests skip due to crbug.com/dawn/592.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    constexpr uint32_t kWidth = 899;
    constexpr uint32_t kHeight = 999;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight};

    DoTest(textureSpec, textureSpec, {kWidth, kHeight});
}

TEST_P(CopyTextureForBrowserTests, VerifyFlipY) {
    // Tests skip due to crbug.com/dawn/592.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    constexpr uint32_t kWidth = 901;
    constexpr uint32_t kHeight = 1001;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight};

    wgpu::CopyTextureForBrowserOptions options = {};
    options.flipY = true;
    DoTest(textureSpec, textureSpec, {kWidth, kHeight}, options);
}

TEST_P(CopyTextureForBrowserTests, VerifyFlipYInSlimTexture) {
    // Tests skip due to crbug.com/dawn/592.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    constexpr uint32_t kWidth = 1;
    constexpr uint32_t kHeight = 1001;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight};

    wgpu::CopyTextureForBrowserOptions options = {};
    options.flipY = true;
    DoTest(textureSpec, textureSpec, {kWidth, kHeight}, options);
}

// Verify |CopyTextureForBrowser| doing color conversion correctly when
// the source texture is RGBA8Unorm format.
TEST_P(CopyTextureForBrowserTests, FromRGBA8UnormCopy) {
    // Tests skip due to crbug.com/dawn/592.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsBackendValidationEnabled());
    // Skip OpenGLES backend because it fails on using RGBA8Unorm as
    // source texture format.
    DAWN_SKIP_TEST_IF(IsOpenGLES());

    for (wgpu::TextureFormat dstFormat : kDstTextureFormat) {
        TextureSpec srcTextureSpec = {};  // default format is RGBA8Unorm

        TextureSpec dstTextureSpec;
        dstTextureSpec.format = dstFormat;

        DoColorConversionTest(srcTextureSpec, dstTextureSpec);
    }
}

// Verify |CopyTextureForBrowser| doing color conversion correctly when
// the source texture is BGRAUnorm format.
TEST_P(CopyTextureForBrowserTests, FromBGRA8UnormCopy) {
    // Tests skip due to crbug.com/dawn/592.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsBackendValidationEnabled());
    // Skip OpenGLES backend because it fails on using BGRA8Unorm as
    // source texture format.
    DAWN_SKIP_TEST_IF(IsOpenGLES());

    for (wgpu::TextureFormat dstFormat : kDstTextureFormat) {
        TextureSpec srcTextureSpec;
        srcTextureSpec.format = wgpu::TextureFormat::BGRA8Unorm;

        TextureSpec dstTextureSpec;
        dstTextureSpec.format = dstFormat;

        DoColorConversionTest(srcTextureSpec, dstTextureSpec);
    }
}

// Verify |CopyTextureForBrowser| doing subrect copy.
// Source texture is a full red texture and dst texture is a full
// green texture originally. After the subrect copy, affected part
// in dst texture should be red and other part should remain green.
TEST_P(CopyTextureForBrowserTests, CopySubRect) {
    // Tests skip due to crbug.com/dawn/592.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    for (wgpu::Origin3D srcOrigin : kOrigins) {
        for (wgpu::Origin3D dstOrigin : kOrigins) {
            for (wgpu::Extent3D copySize : kCopySize) {
                for (bool flipY : {true, false}) {
                    TextureSpec srcTextureSpec;
                    srcTextureSpec.copyOrigin = srcOrigin;
                    srcTextureSpec.textureSize = {6, 7};

                    TextureSpec dstTextureSpec;
                    dstTextureSpec.copyOrigin = dstOrigin;
                    dstTextureSpec.textureSize = {8, 5};
                    wgpu::CopyTextureForBrowserOptions options = {};
                    options.flipY = flipY;

                    DoTest(srcTextureSpec, dstTextureSpec, copySize, options);
                }
            }
        }
    }
}

DAWN_INSTANTIATE_TEST(CopyTextureForBrowserTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
