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
#include "utils/TextureUtils.h"
#include "utils/WGPUHelpers.h"

namespace {
    static constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::RGBA8Unorm;

    // Set default texture size to single line texture for color conversion tests.
    static constexpr uint64_t kDefaultTextureWidth = 10;
    static constexpr uint64_t kDefaultTextureHeight = 1;

    using Alpha = wgpu::AlphaOp;
    DAWN_TEST_PARAM_STRUCT(AlphaTestParams, Alpha);

    using SrcFormat = wgpu::TextureFormat;
    using DstFormat = wgpu::TextureFormat;
    using SrcOrigin = wgpu::Origin3D;
    using DstOrigin = wgpu::Origin3D;
    using CopySize = wgpu::Extent3D;
    using FlipY = bool;

    std::ostream& operator<<(std::ostream& o, wgpu::Origin3D origin) {
        o << origin.x << ", " << origin.y << ", " << origin.z;
        return o;
    }

    std::ostream& operator<<(std::ostream& o, wgpu::Extent3D copySize) {
        o << copySize.width << ", " << copySize.height << ", " << copySize.depthOrArrayLayers;
        return o;
    }

    DAWN_TEST_PARAM_STRUCT(FormatTestParams, SrcFormat, DstFormat);
    DAWN_TEST_PARAM_STRUCT(SubRectTestParams, SrcOrigin, DstOrigin, CopySize, FlipY);

}  // anonymous namespace

template <typename Parent>
class CopyTextureForBrowserTests : public Parent {
  protected:
    struct TextureSpec {
        wgpu::Origin3D copyOrigin = {};
        wgpu::Extent3D textureSize = {kDefaultTextureWidth, kDefaultTextureHeight};
        uint32_t level = 0;
        wgpu::TextureFormat format = kTextureFormat;
    };

    enum class TextureCopyRole {
        SOURCE,
        DEST,
    };

    // Source texture contains red pixels and dst texture contains green pixels at start.
    static std::vector<RGBA8> GetTextureData(const utils::TextureDataCopyLayout& layout,
                                             TextureCopyRole textureRole,
                                             wgpu::AlphaOp alphaOp = wgpu::AlphaOp::DontChange) {
        std::array<uint8_t, 4> alpha = {0, 102, 153, 255};  // 0.0, 0.4, 0.6, 1.0
        std::vector<RGBA8> textureData(layout.texelBlockCount);
        for (uint32_t layer = 0; layer < layout.mipSize.depthOrArrayLayers; ++layer) {
            const uint32_t sliceOffset = layout.texelBlocksPerImage * layer;
            for (uint32_t y = 0; y < layout.mipSize.height; ++y) {
                const uint32_t rowOffset = layout.texelBlocksPerRow * y;
                for (uint32_t x = 0; x < layout.mipSize.width; ++x) {
                    // Source textures will have variable pixel data to cover cases like
                    // flipY.
                    if (textureRole == TextureCopyRole::SOURCE) {
                        switch (alphaOp) {
                            case wgpu::AlphaOp::DontChange:
                                textureData[sliceOffset + rowOffset + x] = RGBA8(
                                    static_cast<uint8_t>((x + layer * x) % 256),
                                    static_cast<uint8_t>((y + layer * y) % 256),
                                    static_cast<uint8_t>(x % 256), static_cast<uint8_t>(x % 256));
                                break;
                            case wgpu::AlphaOp::Premultiply:
                                // For premultiply alpha test cases, we expect each channel in dst
                                // texture will equal to the alpha channel value.
                                textureData[sliceOffset + rowOffset + x] = RGBA8(
                                    static_cast<uint8_t>(255), static_cast<uint8_t>(255),
                                    static_cast<uint8_t>(255), static_cast<uint8_t>(alpha[x % 4]));
                                break;
                            case wgpu::AlphaOp::Unpremultiply:
                                // For unpremultiply alpha test cases, we expect each channel in dst
                                // texture will equal to 1.0.
                                textureData[sliceOffset + rowOffset + x] =
                                    RGBA8(static_cast<uint8_t>(alpha[x % 4]),
                                          static_cast<uint8_t>(alpha[x % 4]),
                                          static_cast<uint8_t>(alpha[x % 4]),
                                          static_cast<uint8_t>(alpha[x % 4]));
                                break;
                            default:
                                UNREACHABLE();
                                break;
                        }
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
        Parent::SetUp();
        pipeline = MakeTestPipeline();

        uint32_t uniformBufferData[] = {
            0,  // copy have flipY option
            4,  // channelCount
            0,
            0,  // uvec2, subrect copy src origin
            0,
            0,  // uvec2, subrect copy dst origin
            0,
            0,                                                 // uvec2, subrect copy size
            static_cast<uint32_t>(wgpu::AlphaOp::DontChange),  // AlphaOp: DontChange
        };

        wgpu::BufferDescriptor uniformBufferDesc = {};
        uniformBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
        uniformBufferDesc.size = sizeof(uniformBufferData);
        uniformBuffer = this->device.CreateBuffer(&uniformBufferDesc);
    }

    // Do the bit-by-bit comparison between the source and destination texture with GPU (compute
    // shader) instead of CPU after executing CopyTextureForBrowser() to avoid the errors caused by
    // comparing a value generated on CPU to the one generated on GPU.
    wgpu::ComputePipeline MakeTestPipeline() {
        wgpu::ShaderModule csModule = utils::CreateShaderModule(this->device, R"(
            [[block]] struct Uniforms {
                dstTextureFlipY : u32;
                channelCount    : u32;
                srcCopyOrigin   : vec2<u32>;
                dstCopyOrigin   : vec2<u32>;
                copySize        : vec2<u32>;
                alphaOp         : u32;
            };
            [[block]] struct OutputBuf {
                result : array<u32>;
            };
            [[group(0), binding(0)]] var src : texture_2d<f32>;
            [[group(0), binding(1)]] var dst : texture_2d<f32>;
            [[group(0), binding(2)]] var<storage, read_write> output : OutputBuf;
            [[group(0), binding(3)]] var<uniform> uniforms : Uniforms;
            fn aboutEqual(value : f32, expect : f32) -> bool {
                // The value diff should be smaller than the hard coded tolerance.
                return abs(value - expect) < 0.01;
            }
            [[stage(compute), workgroup_size(1, 1, 1)]]
            fn main([[builtin(global_invocation_id)]] GlobalInvocationID : vec3<u32>) {
                let srcSize = textureDimensions(src);
                let dstSize = textureDimensions(dst);
                let dstTexCoord = vec2<u32>(GlobalInvocationID.xy);
                let nonCoveredColor = vec4<f32>(0.0, 1.0, 0.0, 1.0); // should be green

                var success : bool = true;
                if (dstTexCoord.x < uniforms.dstCopyOrigin.x ||
                    dstTexCoord.y < uniforms.dstCopyOrigin.y ||
                    dstTexCoord.x >= uniforms.dstCopyOrigin.x + uniforms.copySize.x ||
                    dstTexCoord.y >= uniforms.dstCopyOrigin.y + uniforms.copySize.y) {
                    success = success &&
                              all(textureLoad(dst, vec2<i32>(dstTexCoord), 0) == nonCoveredColor);
                } else {
                    // Calculate source texture coord.
                    var srcTexCoord = dstTexCoord - uniforms.dstCopyOrigin +
                                                  uniforms.srcCopyOrigin;
                    // Note that |flipY| equals flip src texture firstly and then do copy from src
                    // subrect to dst subrect. This helps on blink part to handle some input texture
                    // which is flipped and need to unpack flip during the copy.
                    // We need to calculate the expect y coord based on this rule.
                    if (uniforms.dstTextureFlipY == 1u) {
                        srcTexCoord.y = u32(srcSize.y) - srcTexCoord.y - 1u;
                    }

                    var srcColor = textureLoad(src, vec2<i32>(srcTexCoord), 0);
                    var dstColor = textureLoad(dst, vec2<i32>(dstTexCoord), 0);

                    // Expect the dst texture channels should be all equal to alpha value
                    // after premultiply.
                    // TODO(crbug.com/1217153): if wgsl support `constexpr` and allow it
                    // to be case selector, Replace 0u/1u/2u with a constexpr variable with
                    // meaningful name.
                    switch(uniforms.alphaOp) {
                        case 0u: { // AlphaOp: DontChange
                            break;
                        }
                        case 1u: { // AlphaOp: Premultiply
                            srcColor = vec4<f32>(srcColor.rgb * srcColor.a, srcColor.a);
                            break;
                        }
                        case 2u: { // AlphaOp: Unpremultiply
                            if (srcColor.a != 0.0) {
                                srcColor = vec4<f32>(srcColor.rgb / srcColor.a, srcColor.a);
                            }
                            break;
                        }
                        default: {
                            break;
                        }
                    }

                    // Not use loop and variable index format to workaround
                    // crbug.com/tint/638.
                    switch(uniforms.channelCount) {
                        case 1u: {
                            success = success && aboutEqual(dstColor.r, srcColor.r);
                            break;
                        }
                        case 2u: {
                            success = success &&
                                      aboutEqual(dstColor.r, srcColor.r) &&
                                      aboutEqual(dstColor.g, srcColor.g);
                            break;
                        }
                        case 4u: {
                            success = success &&
                                      aboutEqual(dstColor.r, srcColor.r) &&
                                      aboutEqual(dstColor.g, srcColor.g) &&
                                      aboutEqual(dstColor.b, srcColor.b) &&
                                      aboutEqual(dstColor.a, srcColor.a);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }
                let outputIndex = GlobalInvocationID.y * u32(dstSize.x) + GlobalInvocationID.x;
                if (success) {
                    output.result[outputIndex] = 1u;
                } else {
                    output.result[outputIndex] = 0u;
                }
            }
         )");

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = csModule;
        csDesc.compute.entryPoint = "main";

        return this->device.CreateComputePipeline(&csDesc);
    }
    static uint32_t GetTextureFormatComponentCount(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::RGBA8UnormSrgb:
            case wgpu::TextureFormat::BGRA8Unorm:
            case wgpu::TextureFormat::BGRA8UnormSrgb:
            case wgpu::TextureFormat::RGB10A2Unorm:
            case wgpu::TextureFormat::RGBA16Float:
            case wgpu::TextureFormat::RGBA32Float:
                return 4;
            case wgpu::TextureFormat::RG8Unorm:
            case wgpu::TextureFormat::RG16Float:
            case wgpu::TextureFormat::RG32Float:
                return 2;
            case wgpu::TextureFormat::R8Unorm:
            case wgpu::TextureFormat::R16Float:
            case wgpu::TextureFormat::R32Float:
                return 1;
            default:
                UNREACHABLE();
        }
    }

    wgpu::Texture CreateTexture(const TextureSpec& spec, wgpu::TextureUsage usage) {
        // Create and initialize src texture.
        wgpu::TextureDescriptor descriptor;
        descriptor.size = spec.textureSize;
        descriptor.format = spec.format;
        descriptor.mipLevelCount = spec.level + 1;
        descriptor.usage = usage;
        wgpu::Texture texture = this->device.CreateTexture(&descriptor);
        return texture;
    }

    wgpu::Texture CreateAndInitTexture(const TextureSpec& spec,
                                       wgpu::TextureUsage usage,
                                       utils::TextureDataCopyLayout copyLayout,
                                       void const* init,
                                       uint32_t initBytes) {
        wgpu::Texture texture = CreateTexture(spec, usage);

        wgpu::ImageCopyTexture imageTextureInit =
            utils::CreateImageCopyTexture(texture, spec.level, {0, 0});

        wgpu::TextureDataLayout textureDataLayout;
        textureDataLayout.offset = 0;
        textureDataLayout.bytesPerRow = copyLayout.bytesPerRow;
        textureDataLayout.rowsPerImage = copyLayout.rowsPerImage;

        this->device.GetQueue().WriteTexture(&imageTextureInit, init, initBytes, &textureDataLayout,
                                             &copyLayout.mipSize);
        return texture;
    }

    void RunCopyExternalImageToTexture(const TextureSpec& srcSpec,
                                       wgpu::Texture srcTexture,
                                       const TextureSpec& dstSpec,
                                       wgpu::Texture dstTexture,
                                       const wgpu::Extent3D& copySize,
                                       const wgpu::CopyTextureForBrowserOptions options) {
        wgpu::ImageCopyTexture srcImageCopyTexture =
            utils::CreateImageCopyTexture(srcTexture, srcSpec.level, srcSpec.copyOrigin);
        wgpu::ImageCopyTexture dstImageCopyTexture =
            utils::CreateImageCopyTexture(dstTexture, dstSpec.level, dstSpec.copyOrigin);
        this->device.GetQueue().CopyTextureForBrowser(&srcImageCopyTexture, &dstImageCopyTexture,
                                                      &copySize, &options);
    }

    void CheckResultInBuiltInComputePipeline(const TextureSpec& srcSpec,
                                             wgpu::Texture srcTexture,
                                             const TextureSpec& dstSpec,
                                             wgpu::Texture dstTexture,
                                             const wgpu::Extent3D& copySize,
                                             const wgpu::CopyTextureForBrowserOptions options) {
        // Update uniform buffer based on test config
        uint32_t uniformBufferData[] = {
            options.flipY,                                   // copy have flipY option
            GetTextureFormatComponentCount(dstSpec.format),  // channelCount
            srcSpec.copyOrigin.x,
            srcSpec.copyOrigin.y,  // src texture copy origin
            dstSpec.copyOrigin.x,
            dstSpec.copyOrigin.y,  // dst texture copy origin
            copySize.width,
            copySize.height,                        // copy size
            static_cast<uint32_t>(options.alphaOp)  // alphaOp
        };

        this->device.GetQueue().WriteBuffer(uniformBuffer, 0, uniformBufferData,
                                            sizeof(uniformBufferData));

        // Create output buffer to store result
        wgpu::BufferDescriptor outputDesc;
        outputDesc.size = dstSpec.textureSize.width * dstSpec.textureSize.height * sizeof(uint32_t);
        outputDesc.usage =
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer outputBuffer = this->device.CreateBuffer(&outputDesc);

        // Create texture views for test.
        wgpu::TextureViewDescriptor srcTextureViewDesc = {};
        srcTextureViewDesc.baseMipLevel = srcSpec.level;
        wgpu::TextureView srcTextureView = srcTexture.CreateView(&srcTextureViewDesc);

        wgpu::TextureViewDescriptor dstTextureViewDesc = {};
        dstTextureViewDesc.baseMipLevel = dstSpec.level;
        wgpu::TextureView dstTextureView = dstTexture.CreateView(&dstTextureViewDesc);

        // Create bind group based on the config.
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            this->device, pipeline.GetBindGroupLayout(0),
            {{0, srcTextureView}, {1, dstTextureView}, {2, outputBuffer}, {3, uniformBuffer}});

        // Start a pipeline to check pixel value in bit form.
        wgpu::CommandEncoder testEncoder = this->device.CreateCommandEncoder();

        wgpu::CommandBuffer testCommands;
        {
            wgpu::CommandEncoder encoder = this->device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Dispatch(dstSpec.textureSize.width,
                          dstSpec.textureSize.height);  // Verify dst texture content
            pass.EndPass();

            testCommands = encoder.Finish();
        }
        this->device.GetQueue().Submit(1, &testCommands);

        std::vector<uint32_t> expectResult(dstSpec.textureSize.width * dstSpec.textureSize.height,
                                           1);
        EXPECT_BUFFER_U32_RANGE_EQ(expectResult.data(), outputBuffer, 0,
                                   dstSpec.textureSize.width * dstSpec.textureSize.height);
    }

    void DoTest(const TextureSpec& srcSpec,
                const TextureSpec& dstSpec,
                const wgpu::Extent3D& copySize = {kDefaultTextureWidth, kDefaultTextureHeight},
                const wgpu::CopyTextureForBrowserOptions options = {}) {
        // Create and initialize src texture.
        const utils::TextureDataCopyLayout srcCopyLayout =
            utils::GetTextureDataCopyLayoutForTextureAtLevel(
                kTextureFormat,
                {srcSpec.textureSize.width, srcSpec.textureSize.height,
                 copySize.depthOrArrayLayers},
                srcSpec.level);

        std::vector<RGBA8> srcTextureArrayCopyData =
            GetTextureData(srcCopyLayout, TextureCopyRole::SOURCE, options.alphaOp);

        wgpu::TextureUsage srcUsage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                                      wgpu::TextureUsage::TextureBinding;
        wgpu::Texture srcTexture =
            CreateAndInitTexture(srcSpec, srcUsage, srcCopyLayout, srcTextureArrayCopyData.data(),
                                 srcTextureArrayCopyData.size() * sizeof(RGBA8));

        bool testSubRectCopy = srcSpec.copyOrigin.x > 0 || srcSpec.copyOrigin.y > 0 ||
                               dstSpec.copyOrigin.x > 0 || dstSpec.copyOrigin.y > 0 ||
                               srcSpec.textureSize.width > copySize.width ||
                               srcSpec.textureSize.height > copySize.height ||
                               dstSpec.textureSize.width > copySize.width ||
                               dstSpec.textureSize.height > copySize.height;

        // Create and init dst texture.
        wgpu::Texture dstTexture;
        wgpu::TextureUsage dstUsage =
            wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding |
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;

        if (testSubRectCopy) {
            // For subrect copy tests, dst texture use kTextureFormat always.
            const utils::TextureDataCopyLayout dstCopyLayout =
                utils::GetTextureDataCopyLayoutForTextureAtLevel(
                    kTextureFormat,
                    {dstSpec.textureSize.width, dstSpec.textureSize.height,
                     copySize.depthOrArrayLayers},
                    dstSpec.level);

            const std::vector<RGBA8> dstTextureArrayCopyData =
                GetTextureData(dstCopyLayout, TextureCopyRole::DEST);
            dstTexture = CreateAndInitTexture(dstSpec, dstUsage, dstCopyLayout,
                                              dstTextureArrayCopyData.data(),
                                              dstTextureArrayCopyData.size() * sizeof(RGBA8));
        } else {
            dstTexture = CreateTexture(dstSpec, dstUsage);
        }

        // Perform the texture to texture copy
        RunCopyExternalImageToTexture(srcSpec, srcTexture, dstSpec, dstTexture, copySize, options);

        // Check Result
        CheckResultInBuiltInComputePipeline(srcSpec, srcTexture, dstSpec, dstTexture, copySize,
                                            options);
    }

    wgpu::Buffer uniformBuffer;
    wgpu::ComputePipeline pipeline;
};

class CopyTextureForBrowser_Basic : public CopyTextureForBrowserTests<DawnTest> {
  protected:
    void DoBasicCopyTest(const wgpu::Extent3D& copySize,
                         const wgpu::CopyTextureForBrowserOptions options = {}) {
        TextureSpec textureSpec;
        textureSpec.textureSize = copySize;

        DoTest(textureSpec, textureSpec, copySize, options);
    }
};

class CopyTextureForBrowser_Formats
    : public CopyTextureForBrowserTests<DawnTestWithParams<FormatTestParams>> {
  protected:
    bool IsDstFormatSrgbFormats() {
        return GetParam().mDstFormat == wgpu::TextureFormat::RGBA8UnormSrgb ||
               GetParam().mDstFormat == wgpu::TextureFormat::BGRA8UnormSrgb;
    }

    void DoColorConversionTest() {
        TextureSpec srcTextureSpec;
        srcTextureSpec.format = GetParam().mSrcFormat;

        TextureSpec dstTextureSpec;
        dstTextureSpec.format = GetParam().mDstFormat;

        wgpu::Extent3D copySize = {kDefaultTextureWidth, kDefaultTextureHeight};
        wgpu::CopyTextureForBrowserOptions options = {};

        // Create and init source texture.
        // This fixed source texture data is for color conversion tests.
        // The source data can fill a texture in default width and height.
        std::vector<RGBA8> srcTextureArrayCopyData{
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

        const utils::TextureDataCopyLayout srcCopyLayout =
            utils::GetTextureDataCopyLayoutForTextureAtLevel(
                kTextureFormat,
                {srcTextureSpec.textureSize.width, srcTextureSpec.textureSize.height,
                 copySize.depthOrArrayLayers},
                srcTextureSpec.level);

        wgpu::TextureUsage srcUsage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                                      wgpu::TextureUsage::TextureBinding;
        wgpu::Texture srcTexture = CreateAndInitTexture(
            srcTextureSpec, srcUsage, srcCopyLayout, srcTextureArrayCopyData.data(),
            srcTextureArrayCopyData.size() * sizeof(RGBA8));

        // Create dst texture.
        wgpu::Texture dstTexture = CreateTexture(
            dstTextureSpec, wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding |
                                wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);

        // Perform the texture to texture copy
        RunCopyExternalImageToTexture(srcTextureSpec, srcTexture, dstTextureSpec, dstTexture,
                                      copySize, options);

        // Check Result
        CheckResultInBuiltInComputePipeline(srcTextureSpec, srcTexture, dstTextureSpec, dstTexture,
                                            copySize, options);
    }
};

class CopyTextureForBrowser_SubRects
    : public CopyTextureForBrowserTests<DawnTestWithParams<SubRectTestParams>> {
  protected:
    void DoCopySubRectTest() {
        TextureSpec srcTextureSpec;
        srcTextureSpec.copyOrigin = GetParam().mSrcOrigin;
        srcTextureSpec.textureSize = {6, 7};

        TextureSpec dstTextureSpec;
        dstTextureSpec.copyOrigin = GetParam().mDstOrigin;
        dstTextureSpec.textureSize = {8, 5};
        wgpu::CopyTextureForBrowserOptions options = {};
        options.flipY = GetParam().mFlipY;

        wgpu::Extent3D copySize = GetParam().mCopySize;

        DoTest(srcTextureSpec, dstTextureSpec, copySize, options);
    }
};

class CopyTextureForBrowser_AlphaOps
    : public CopyTextureForBrowserTests<DawnTestWithParams<AlphaTestParams>> {
  protected:
    void DoAlphaOpTest() {
        constexpr uint32_t kWidth = 10;
        constexpr uint32_t kHeight = 10;

        TextureSpec textureSpec;
        textureSpec.textureSize = {kWidth, kHeight};

        wgpu::CopyTextureForBrowserOptions options = {};
        options.alphaOp = GetParam().mAlpha;

        DoTest(textureSpec, textureSpec, {kWidth, kHeight}, options);
    }
};

// Verify CopyTextureForBrowserTests works with internal pipeline.
// The case do copy without any transform.
TEST_P(CopyTextureForBrowser_Basic, PassthroughCopy) {
    DoBasicCopyTest({10, 1});
}

TEST_P(CopyTextureForBrowser_Basic, VerifyCopyOnXDirection) {
    DoBasicCopyTest({1000, 1});
}

TEST_P(CopyTextureForBrowser_Basic, VerifyCopyOnYDirection) {
    DoBasicCopyTest({1, 1000});
}

TEST_P(CopyTextureForBrowser_Basic, VerifyCopyFromLargeTexture) {
    // TODO(crbug.com/dawn/1070): Flaky VK_DEVICE_LOST
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsIntel());

    DoBasicCopyTest({899, 999});
}

TEST_P(CopyTextureForBrowser_Basic, VerifyFlipY) {
    wgpu::CopyTextureForBrowserOptions options = {};
    options.flipY = true;

    DoBasicCopyTest({901, 1001}, options);
}

TEST_P(CopyTextureForBrowser_Basic, VerifyFlipYInSlimTexture) {
    wgpu::CopyTextureForBrowserOptions options = {};
    options.flipY = true;

    DoBasicCopyTest({1, 1001}, options);
}

DAWN_INSTANTIATE_TEST(CopyTextureForBrowser_Basic,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

// Verify |CopyTextureForBrowser| doing color conversion correctly when
// the source texture is RGBA8Unorm format.
TEST_P(CopyTextureForBrowser_Formats, ColorConversion) {
    // Skip OpenGLES backend because it fails on using RGBA8Unorm as
    // source texture format.
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES());

    // Skip OpenGL backend on linux because it fails on using *-srgb format as
    // dst texture format
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() && IsLinux() && IsDstFormatSrgbFormats());

    DoColorConversionTest();
}

DAWN_INSTANTIATE_TEST_P(
    CopyTextureForBrowser_Formats,
    {D3D12Backend(), MetalBackend(), OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
    std::vector<wgpu::TextureFormat>({wgpu::TextureFormat::RGBA8Unorm,
                                      wgpu::TextureFormat::BGRA8Unorm}),
    std::vector<wgpu::TextureFormat>(
        {wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R16Float, wgpu::TextureFormat::R32Float,
         wgpu::TextureFormat::RG8Unorm, wgpu::TextureFormat::RG16Float,
         wgpu::TextureFormat::RG32Float, wgpu::TextureFormat::RGBA8Unorm,
         wgpu::TextureFormat::BGRA8Unorm, wgpu::TextureFormat::RGB10A2Unorm,
         wgpu::TextureFormat::RGBA16Float, wgpu::TextureFormat::RGBA32Float}));

// Verify |CopyTextureForBrowser| doing subrect copy.
// Source texture is a full red texture and dst texture is a full
// green texture originally. After the subrect copy, affected part
// in dst texture should be red and other part should remain green.
TEST_P(CopyTextureForBrowser_SubRects, CopySubRect) {
    // Tests skip due to crbug.com/dawn/592.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    // Tests skip due to crbug.com/dawn/1104.
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    DoCopySubRectTest();
}

DAWN_INSTANTIATE_TEST_P(CopyTextureForBrowser_SubRects,
                        {D3D12Backend(), MetalBackend(), OpenGLBackend(), OpenGLESBackend(),
                         VulkanBackend()},
                        std::vector<wgpu::Origin3D>({{1, 1}, {1, 2}, {2, 1}}),
                        std::vector<wgpu::Origin3D>({{1, 1}, {1, 2}, {2, 1}}),
                        std::vector<wgpu::Extent3D>({{1, 1}, {2, 1}, {1, 2}, {2, 2}}),
                        std::vector<bool>({true, false}));

// Verify |CopyTextureForBrowser| doing alphaOp.
// Test alpha ops: DontChange, Premultiply, Unpremultiply.
TEST_P(CopyTextureForBrowser_AlphaOps, alphaOp) {
    // Skip OpenGLES backend because it fails on using RGBA8Unorm as
    // source texture format.
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES());

    // Tests skip due to crbug.com/dawn/1104.
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    DoAlphaOpTest();
}

DAWN_INSTANTIATE_TEST_P(
    CopyTextureForBrowser_AlphaOps,
    {D3D12Backend(), MetalBackend(), OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
    std::vector<wgpu::AlphaOp>({wgpu::AlphaOp::DontChange, wgpu::AlphaOp::Premultiply,
                                wgpu::AlphaOp::Unpremultiply}));
