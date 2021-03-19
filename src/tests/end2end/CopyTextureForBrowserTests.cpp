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
    static constexpr wgpu::TextureFormat kDstTextureFormat[] = {
        wgpu::TextureFormat::RGBA8Unorm,  wgpu::TextureFormat::BGRA8Unorm,
        wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::RG8Unorm,
        wgpu::TextureFormat::RGBA16Float, wgpu::TextureFormat::RG16Float,
        wgpu::TextureFormat::RGB10A2Unorm};
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

    static std::vector<RGBA8> GetSourceTextureData(const utils::TextureDataCopyLayout& layout) {
        std::vector<RGBA8> textureData(layout.texelBlockCount);
        for (uint32_t layer = 0; layer < layout.mipSize.depth; ++layer) {
            const uint32_t sliceOffset = layout.texelBlocksPerImage * layer;
            for (uint32_t y = 0; y < layout.mipSize.height; ++y) {
                const uint32_t rowOffset = layout.texelBlocksPerRow * y;
                for (uint32_t x = 0; x < layout.mipSize.width; ++x) {
                    textureData[sliceOffset + rowOffset + x] =
                        RGBA8(static_cast<uint8_t>((x + layer * x) % 256),
                              static_cast<uint8_t>((y + layer * y) % 256),
                              static_cast<uint8_t>(x % 256), static_cast<uint8_t>(x % 256));
                }
            }
        }

        return textureData;
    }

    void SetUp() override {
        DawnTest::SetUp();

        testPipeline = MakeTestPipeline();

        uint32_t uniformBufferData[] = {
            0,  // copy have flipY option
            4,  // channelCount
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
        wgpu::ShaderModule csModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[block]] struct Uniforms {
                dstTextureFlipY : u32;
                channelCount : u32;
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
                // Current CopyTextureForBrowser only support full copy now.
                // TODO(crbug.com/dawn/465): Refactor this after CopyTextureForBrowser
                // support sub-rect copy.
                var size : vec2<i32> = textureDimensions(src);
                var dstTexCoord : vec2<i32> = vec2<i32>(GlobalInvocationID.xy);
                var srcTexCoord : vec2<i32> = dstTexCoord;
                if (uniforms.dstTextureFlipY == 1u) {
                    srcTexCoord.y = size.y - dstTexCoord.y - 1;
                }

                var srcColor : vec4<f32> = textureLoad(src, srcTexCoord, 0);
                var dstColor : vec4<f32> = textureLoad(dst, dstTexCoord, 0);
                var success : bool = true;

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

                var outputIndex : u32 = GlobalInvocationID.y * u32(size.x) + GlobalInvocationID.x;
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
        wgpu::TextureDescriptor srcDescriptor;
        srcDescriptor.size = srcSpec.textureSize;
        srcDescriptor.format = srcSpec.format;
        srcDescriptor.mipLevelCount = srcSpec.level + 1;
        srcDescriptor.usage =
            wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::Sampled;
        wgpu::Texture srcTexture = device.CreateTexture(&srcDescriptor);

        wgpu::Texture dstTexture;
        wgpu::TextureDescriptor dstDescriptor;
        dstDescriptor.size = dstSpec.textureSize;
        dstDescriptor.format = dstSpec.format;
        dstDescriptor.mipLevelCount = dstSpec.level + 1;
        dstDescriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::Sampled |
                              wgpu::TextureUsage::OutputAttachment | wgpu::TextureUsage::CopySrc;
        dstTexture = device.CreateTexture(&dstDescriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        const utils::TextureDataCopyLayout copyLayout =
            utils::GetTextureDataCopyLayoutForTexture2DAtLevel(
                kTextureFormat,
                {srcSpec.textureSize.width, srcSpec.textureSize.height, copySize.depth},
                srcSpec.level);

        const std::vector<RGBA8> textureArrayCopyData =
            useFixedTestValue ? GetFixedSourceTextureData() : GetSourceTextureData(copyLayout);
        wgpu::ImageCopyTexture imageCopyTexture =
            utils::CreateImageCopyTexture(srcTexture, srcSpec.level, {0, 0, srcSpec.copyOrigin.z});

        wgpu::TextureDataLayout textureDataLayout;
        textureDataLayout.offset = 0;
        textureDataLayout.bytesPerRow = copyLayout.bytesPerRow;
        textureDataLayout.rowsPerImage = copyLayout.rowsPerImage;

        device.GetQueue().WriteTexture(&imageCopyTexture, textureArrayCopyData.data(),
                                       textureArrayCopyData.size() * sizeof(RGBA8),
                                       &textureDataLayout, &copyLayout.mipSize);

        // Perform the texture to texture copy
        wgpu::ImageCopyTexture srcImageCopyTexture =
            utils::CreateImageCopyTexture(srcTexture, srcSpec.level, srcSpec.copyOrigin);
        wgpu::ImageCopyTexture dstImageCopyTexture =
            utils::CreateImageCopyTexture(dstTexture, dstSpec.level, dstSpec.copyOrigin);

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        device.GetQueue().CopyTextureForBrowser(&srcImageCopyTexture, &dstImageCopyTexture,
                                                &copySize, &options);

        // Update uniform buffer based on test config
        uint32_t uniformBufferData[] = {
            options.flipY,                                    // copy have flipY option
            GetTextureFormatComponentCount(dstSpec.format)};  // channelCount

        device.GetQueue().WriteBuffer(uniformBuffer, 0, uniformBufferData,
                                      sizeof(uniformBufferData));

        // Create output buffer to store result
        wgpu::BufferDescriptor outputDesc;
        outputDesc.size = copySize.width * copySize.height * sizeof(uint32_t);
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
            {{0, srcTextureView},
             {1, dstTextureView},
             {2, outputBuffer, 0, copySize.width * copySize.height * sizeof(uint32_t)},
             {3, uniformBuffer, 0, sizeof(uniformBufferData)}});

        // Start a pipeline to check pixel value in bit form.
        wgpu::CommandEncoder testEncoder = device.CreateCommandEncoder();

        wgpu::CommandBuffer testCommands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(testPipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Dispatch(copySize.width, copySize.height);
            pass.EndPass();

            testCommands = encoder.Finish();
        }
        queue.Submit(1, &testCommands);

        std::vector<uint32_t> expectResult(copySize.width * copySize.height, 1);
        EXPECT_BUFFER_U32_RANGE_EQ(expectResult.data(), outputBuffer, 0,
                                   copySize.width * copySize.height);
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

DAWN_INSTANTIATE_TEST(CopyTextureForBrowserTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
