// Copyright 2017 The Dawn Authors
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

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

class RenderPipelineValidationTest : public ValidationTest {
  protected:
    void SetUp() override {
        ValidationTest::SetUp();

        vsModule = utils::CreateShaderModule(device, R"(
            @stage(vertex) fn main() -> @builtin(position) vec4<f32> {
                return vec4<f32>(0.0, 0.0, 0.0, 1.0);
            })");

        fsModule = utils::CreateShaderModule(device, R"(
            @stage(fragment) fn main() -> @location(0) vec4<f32> {
                return vec4<f32>(0.0, 1.0, 0.0, 1.0);
            })");

        fsModuleUint = utils::CreateShaderModule(device, R"(
            @stage(fragment) fn main() -> @location(0) vec4<u32> {
                return vec4<u32>(0u, 255u, 0u, 255u);
            })");
    }

    wgpu::ShaderModule vsModule;
    wgpu::ShaderModule fsModule;
    wgpu::ShaderModule fsModuleUint;
};

namespace {
bool BlendFactorContainsSrcAlpha(const wgpu::BlendFactor& blendFactor) {
    return blendFactor == wgpu::BlendFactor::SrcAlpha ||
           blendFactor == wgpu::BlendFactor::OneMinusSrcAlpha ||
           blendFactor == wgpu::BlendFactor::SrcAlphaSaturated;
}
}  // namespace

// Test cases where creation should succeed
TEST_F(RenderPipelineValidationTest, CreationSuccess) {
    {
        // New format
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;

        device.CreateRenderPipeline(&descriptor);
    }
}

// Tests that depth bias parameters must not be NaN.
TEST_F(RenderPipelineValidationTest, DepthBiasParameterNotBeNaN) {
    // Control case, depth bias parameters in ComboRenderPipeline default to 0 which is finite
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.EnableDepthStencil();
        device.CreateRenderPipeline(&descriptor);
    }

    // Infinite depth bias clamp is valid
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();
        depthStencil->depthBiasClamp = INFINITY;
        device.CreateRenderPipeline(&descriptor);
    }
    // NAN depth bias clamp is invalid
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();
        depthStencil->depthBiasClamp = NAN;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // Infinite depth bias slope is valid
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();
        depthStencil->depthBiasSlopeScale = INFINITY;
        device.CreateRenderPipeline(&descriptor);
    }
    // NAN depth bias slope is invalid
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();
        depthStencil->depthBiasSlopeScale = NAN;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Tests that depth or stencil aspect is required if we enable depth or stencil test.
TEST_F(RenderPipelineValidationTest, DepthStencilAspectRequirement) {
    // Control case, stencil aspect is required if stencil test or stencil write is enabled
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil =
            descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24PlusStencil8);
        depthStencil->stencilFront.compare = wgpu::CompareFunction::LessEqual;
        depthStencil->stencilBack.failOp = wgpu::StencilOperation::Replace;
        device.CreateRenderPipeline(&descriptor);
    }

    // It is invalid if the texture format doesn't have stencil aspect while stencil test is
    // enabled (depthStencilState.stencilFront are not default values).
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil =
            descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24Plus);
        depthStencil->stencilFront.compare = wgpu::CompareFunction::LessEqual;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // It is invalid if the texture format doesn't have stencil aspect while stencil write is
    // enabled (depthStencilState.stencilBack are not default values).
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil =
            descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24Plus);
        depthStencil->stencilBack.failOp = wgpu::StencilOperation::Replace;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // Control case, depth aspect is required if depth test or depth write is enabled
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil =
            descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24PlusStencil8);
        depthStencil->depthCompare = wgpu::CompareFunction::LessEqual;
        depthStencil->depthWriteEnabled = true;
        device.CreateRenderPipeline(&descriptor);
    }

    // TODO(dawn:666): Add tests for stencil-only format (Stencil8) with depth test or depth write
    // enabled when Stencil8 format is implemented
}

// Tests that at least one color target state is required.
TEST_F(RenderPipelineValidationTest, ColorTargetStateRequired) {
    {
        // This one succeeds because attachment 0 is the color attachment
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cFragment.targetCount = 1;

        device.CreateRenderPipeline(&descriptor);
    }

    {  // Fail because lack of color target states (and depth/stencil state)
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cFragment.targetCount = 0;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Tests that target blend and writeMasks must not be set if the format is undefined.
TEST_F(RenderPipelineValidationTest, UndefinedColorStateFormatWithBlendOrWriteMask) {
    {
        // Control case: Valid undefined format target.
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cFragment.targetCount = 1;
        descriptor.cTargets[0].format = wgpu::TextureFormat::Undefined;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

        device.CreateRenderPipeline(&descriptor);
    }
    {
        // Error case: undefined format target with blend state set.
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cFragment.targetCount = 1;
        descriptor.cTargets[0].format = wgpu::TextureFormat::Undefined;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

        ASSERT_DEVICE_ERROR(
            device.CreateRenderPipeline(&descriptor),
            testing::HasSubstr("Color target[0] blend state is set when the format is undefined."));
    }
    {
        // Error case: undefined format target with write masking not being none.
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cFragment.targetCount = 1;
        descriptor.cTargets[0].format = wgpu::TextureFormat::Undefined;
        descriptor.cTargets[0].blend = nullptr;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::All;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor),
                            testing::HasSubstr("Color target[0] write mask is set to"));
    }
}

// Tests that the color formats must be renderable.
TEST_F(RenderPipelineValidationTest, NonRenderableFormat) {
    {
        // Succeeds because RGBA8Unorm is renderable
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        device.CreateRenderPipeline(&descriptor);
    }

    {
        // Fails because RG11B10Ufloat is non-renderable
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RG11B10Ufloat;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Tests that the color formats must be blendable when blending is enabled.
// Those are renderable color formats with "float" capabilities in
// https://gpuweb.github.io/gpuweb/#plain-color-formats
TEST_F(RenderPipelineValidationTest, NonBlendableFormat) {
    {
        // Succeeds because RGBA8Unorm is blendable
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        device.CreateRenderPipeline(&descriptor);
    }

    {
        // Fails because RGBA32Float is not blendable
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA32Float;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    {
        // Succeeds because RGBA32Float is not blendable but blending is disabled
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].blend = nullptr;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA32Float;

        device.CreateRenderPipeline(&descriptor);
    }

    {
        // Fails because RGBA8Uint is not blendable
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleUint;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Uint;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    {
        // Succeeds because RGBA8Uint is not blendable but blending is disabled
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleUint;
        descriptor.cTargets[0].blend = nullptr;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Uint;

        device.CreateRenderPipeline(&descriptor);
    }
}

// Tests that the format of the color state descriptor must match the output of the fragment shader.
TEST_F(RenderPipelineValidationTest, FragmentOutputFormatCompatibility) {
    std::array<const char*, 3> kScalarTypes = {{"f32", "i32", "u32"}};
    std::array<wgpu::TextureFormat, 3> kColorFormats = {{wgpu::TextureFormat::RGBA8Unorm,
                                                         wgpu::TextureFormat::RGBA8Sint,
                                                         wgpu::TextureFormat::RGBA8Uint}};

    for (size_t i = 0; i < kScalarTypes.size(); ++i) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        std::ostringstream stream;
        stream << R"(
            @stage(fragment) fn main() -> @location(0) vec4<)"
               << kScalarTypes[i] << R"(> {
                var result : vec4<)"
               << kScalarTypes[i] << R"(>;
                return result;
            })";
        descriptor.cFragment.module = utils::CreateShaderModule(device, stream.str().c_str());

        for (size_t j = 0; j < kColorFormats.size(); ++j) {
            descriptor.cTargets[0].format = kColorFormats[j];
            if (i == j) {
                device.CreateRenderPipeline(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
            }
        }
    }
}

// Tests that the component count of the color state target format must be fewer than that of the
// fragment shader output.
TEST_F(RenderPipelineValidationTest, FragmentOutputComponentCountCompatibility) {
    std::array<wgpu::TextureFormat, 3> kColorFormats = {wgpu::TextureFormat::R8Unorm,
                                                        wgpu::TextureFormat::RG8Unorm,
                                                        wgpu::TextureFormat::RGBA8Unorm};

    std::array<wgpu::BlendFactor, 8> kBlendFactors = {wgpu::BlendFactor::Zero,
                                                      wgpu::BlendFactor::One,
                                                      wgpu::BlendFactor::SrcAlpha,
                                                      wgpu::BlendFactor::OneMinusSrcAlpha,
                                                      wgpu::BlendFactor::Src,
                                                      wgpu::BlendFactor::DstAlpha,
                                                      wgpu::BlendFactor::OneMinusDstAlpha,
                                                      wgpu::BlendFactor::Dst};

    for (size_t componentCount = 1; componentCount <= 4; ++componentCount) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        std::ostringstream stream;
        stream << R"(
            @stage(fragment) fn main() -> @location(0) )";
        switch (componentCount) {
            case 1:
                stream << R"(f32 {
                return 1.0;
                })";
                break;
            case 2:
                stream << R"(vec2<f32> {
                return vec2<f32>(1.0, 1.0);
                })";
                break;
            case 3:
                stream << R"(vec3<f32> {
                return vec3<f32>(1.0, 1.0, 1.0);
                })";
                break;
            case 4:
                stream << R"(vec4<f32> {
                return vec4<f32>(1.0, 1.0, 1.0, 1.0);
                })";
                break;
            default:
                UNREACHABLE();
        }
        descriptor.cFragment.module = utils::CreateShaderModule(device, stream.str().c_str());

        for (auto colorFormat : kColorFormats) {
            descriptor.cTargets[0].format = colorFormat;

            descriptor.cTargets[0].blend = nullptr;
            if (componentCount >= utils::GetWGSLRenderableColorTextureComponentCount(colorFormat)) {
                device.CreateRenderPipeline(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
            }

            descriptor.cTargets[0].blend = &descriptor.cBlends[0];

            for (auto colorSrcFactor : kBlendFactors) {
                descriptor.cBlends[0].color.srcFactor = colorSrcFactor;
                for (auto colorDstFactor : kBlendFactors) {
                    descriptor.cBlends[0].color.dstFactor = colorDstFactor;
                    for (auto alphaSrcFactor : kBlendFactors) {
                        descriptor.cBlends[0].alpha.srcFactor = alphaSrcFactor;
                        for (auto alphaDstFactor : kBlendFactors) {
                            descriptor.cBlends[0].alpha.dstFactor = alphaDstFactor;

                            bool valid = true;
                            if (componentCount >=
                                utils::GetWGSLRenderableColorTextureComponentCount(colorFormat)) {
                                if (BlendFactorContainsSrcAlpha(
                                        descriptor.cTargets[0].blend->color.srcFactor) ||
                                    BlendFactorContainsSrcAlpha(
                                        descriptor.cTargets[0].blend->color.dstFactor)) {
                                    valid = componentCount == 4;
                                }
                            } else {
                                valid = false;
                            }

                            if (valid) {
                                device.CreateRenderPipeline(&descriptor);
                            } else {
                                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
                            }
                        }
                    }
                }
            }
        }
    }
}

// Tests that when blendOperationMinOrMax is "min" or "max", both srcBlendFactor and dstBlendFactor
// must be "one".
TEST_F(RenderPipelineValidationTest, BlendOperationAndBlendFactors) {
    constexpr std::array<wgpu::BlendFactor, 8> kBlendFactors = {wgpu::BlendFactor::Zero,
                                                                wgpu::BlendFactor::One,
                                                                wgpu::BlendFactor::SrcAlpha,
                                                                wgpu::BlendFactor::OneMinusSrcAlpha,
                                                                wgpu::BlendFactor::Src,
                                                                wgpu::BlendFactor::DstAlpha,
                                                                wgpu::BlendFactor::OneMinusDstAlpha,
                                                                wgpu::BlendFactor::Dst};

    constexpr std::array<wgpu::BlendOperation, 2> kBlendOperationsForTest = {
        wgpu::BlendOperation::Max, wgpu::BlendOperation::Min};

    for (wgpu::BlendOperation blendOperationMinOrMax : kBlendOperationsForTest) {
        for (wgpu::BlendFactor srcFactor : kBlendFactors) {
            for (wgpu::BlendFactor dstFactor : kBlendFactors) {
                utils::ComboRenderPipelineDescriptor descriptor;
                descriptor.vertex.module = vsModule;
                descriptor.cFragment.module = fsModule;
                descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
                descriptor.cTargets[0].blend = &descriptor.cBlends[0];
                descriptor.cBlends[0].color.srcFactor = srcFactor;
                descriptor.cBlends[0].color.dstFactor = dstFactor;
                descriptor.cBlends[0].alpha.srcFactor = srcFactor;
                descriptor.cBlends[0].alpha.dstFactor = dstFactor;

                descriptor.cBlends[0].color.operation = blendOperationMinOrMax;
                descriptor.cBlends[0].alpha.operation = wgpu::BlendOperation::Add;
                if (srcFactor == wgpu::BlendFactor::One && dstFactor == wgpu::BlendFactor::One) {
                    device.CreateRenderPipeline(&descriptor);
                } else {
                    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
                }

                descriptor.cBlends[0].color.operation = wgpu::BlendOperation::Add;
                descriptor.cBlends[0].alpha.operation = blendOperationMinOrMax;
                if (srcFactor == wgpu::BlendFactor::One && dstFactor == wgpu::BlendFactor::One) {
                    device.CreateRenderPipeline(&descriptor);
                } else {
                    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
                }
            }
        }
    }
}

/// Tests that the sample count of the render pipeline must be valid.
TEST_F(RenderPipelineValidationTest, SampleCount) {
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 4;

        device.CreateRenderPipeline(&descriptor);
    }

    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 3;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Tests that the sample count of the render pipeline must be equal to the one of every attachments
// in the render pass.
TEST_F(RenderPipelineValidationTest, SampleCountCompatibilityWithRenderPass) {
    constexpr uint32_t kMultisampledCount = 4;
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr wgpu::TextureFormat kDepthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;

    wgpu::TextureDescriptor baseTextureDescriptor;
    baseTextureDescriptor.size.width = 4;
    baseTextureDescriptor.size.height = 4;
    baseTextureDescriptor.size.depthOrArrayLayers = 1;
    baseTextureDescriptor.mipLevelCount = 1;
    baseTextureDescriptor.dimension = wgpu::TextureDimension::e2D;
    baseTextureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;

    utils::ComboRenderPipelineDescriptor nonMultisampledPipelineDescriptor;
    nonMultisampledPipelineDescriptor.multisample.count = 1;
    nonMultisampledPipelineDescriptor.vertex.module = vsModule;
    nonMultisampledPipelineDescriptor.cFragment.module = fsModule;
    wgpu::RenderPipeline nonMultisampledPipeline =
        device.CreateRenderPipeline(&nonMultisampledPipelineDescriptor);

    nonMultisampledPipelineDescriptor.cFragment.targetCount = 0;
    nonMultisampledPipelineDescriptor.EnableDepthStencil();
    wgpu::RenderPipeline nonMultisampledPipelineWithDepthStencilOnly =
        device.CreateRenderPipeline(&nonMultisampledPipelineDescriptor);

    utils::ComboRenderPipelineDescriptor multisampledPipelineDescriptor;
    multisampledPipelineDescriptor.multisample.count = kMultisampledCount;
    multisampledPipelineDescriptor.vertex.module = vsModule;
    multisampledPipelineDescriptor.cFragment.module = fsModule;
    wgpu::RenderPipeline multisampledPipeline =
        device.CreateRenderPipeline(&multisampledPipelineDescriptor);

    multisampledPipelineDescriptor.cFragment.targetCount = 0;
    multisampledPipelineDescriptor.EnableDepthStencil();
    wgpu::RenderPipeline multisampledPipelineWithDepthStencilOnly =
        device.CreateRenderPipeline(&multisampledPipelineDescriptor);

    // It is not allowed to use multisampled render pass and non-multisampled render pipeline.
    {
        wgpu::TextureDescriptor textureDescriptor = baseTextureDescriptor;
        textureDescriptor.format = kColorFormat;
        textureDescriptor.sampleCount = kMultisampledCount;
        wgpu::Texture multisampledColorTexture = device.CreateTexture(&textureDescriptor);
        utils::ComboRenderPassDescriptor renderPassDescriptor(
            {multisampledColorTexture.CreateView()});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(nonMultisampledPipeline);
        renderPass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::TextureDescriptor textureDescriptor = baseTextureDescriptor;
        textureDescriptor.sampleCount = kMultisampledCount;
        textureDescriptor.format = kDepthStencilFormat;
        wgpu::Texture multisampledDepthStencilTexture = device.CreateTexture(&textureDescriptor);
        utils::ComboRenderPassDescriptor renderPassDescriptor(
            {}, multisampledDepthStencilTexture.CreateView());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(nonMultisampledPipelineWithDepthStencilOnly);
        renderPass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // It is allowed to use multisampled render pass and multisampled render pipeline.
    {
        wgpu::TextureDescriptor textureDescriptor = baseTextureDescriptor;
        textureDescriptor.format = kColorFormat;
        textureDescriptor.sampleCount = kMultisampledCount;
        wgpu::Texture multisampledColorTexture = device.CreateTexture(&textureDescriptor);
        utils::ComboRenderPassDescriptor renderPassDescriptor(
            {multisampledColorTexture.CreateView()});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(multisampledPipeline);
        renderPass.End();

        encoder.Finish();
    }

    {
        wgpu::TextureDescriptor textureDescriptor = baseTextureDescriptor;
        textureDescriptor.sampleCount = kMultisampledCount;
        textureDescriptor.format = kDepthStencilFormat;
        wgpu::Texture multisampledDepthStencilTexture = device.CreateTexture(&textureDescriptor);
        utils::ComboRenderPassDescriptor renderPassDescriptor(
            {}, multisampledDepthStencilTexture.CreateView());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(multisampledPipelineWithDepthStencilOnly);
        renderPass.End();

        encoder.Finish();
    }

    // It is not allowed to use non-multisampled render pass and multisampled render pipeline.
    {
        wgpu::TextureDescriptor textureDescriptor = baseTextureDescriptor;
        textureDescriptor.format = kColorFormat;
        textureDescriptor.sampleCount = 1;
        wgpu::Texture nonMultisampledColorTexture = device.CreateTexture(&textureDescriptor);
        utils::ComboRenderPassDescriptor nonMultisampledRenderPassDescriptor(
            {nonMultisampledColorTexture.CreateView()});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass =
            encoder.BeginRenderPass(&nonMultisampledRenderPassDescriptor);
        renderPass.SetPipeline(multisampledPipeline);
        renderPass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::TextureDescriptor textureDescriptor = baseTextureDescriptor;
        textureDescriptor.sampleCount = 1;
        textureDescriptor.format = kDepthStencilFormat;
        wgpu::Texture nonMultisampledDepthStencilTexture = device.CreateTexture(&textureDescriptor);
        utils::ComboRenderPassDescriptor renderPassDescriptor(
            {}, nonMultisampledDepthStencilTexture.CreateView());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(multisampledPipelineWithDepthStencilOnly);
        renderPass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Tests that the vertex only pipeline must be used with a depth-stencil attachment only render pass
TEST_F(RenderPipelineValidationTest, VertexOnlyPipelineRequireDepthStencilAttachment) {
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr wgpu::TextureFormat kDepthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;

    wgpu::TextureDescriptor baseTextureDescriptor;
    baseTextureDescriptor.size = {4, 4};
    baseTextureDescriptor.mipLevelCount = 1;
    baseTextureDescriptor.dimension = wgpu::TextureDimension::e2D;
    baseTextureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;

    wgpu::TextureDescriptor colorTextureDescriptor = baseTextureDescriptor;
    colorTextureDescriptor.format = kColorFormat;
    colorTextureDescriptor.sampleCount = 1;
    wgpu::Texture colorTexture = device.CreateTexture(&colorTextureDescriptor);

    wgpu::TextureDescriptor depthStencilTextureDescriptor = baseTextureDescriptor;
    depthStencilTextureDescriptor.sampleCount = 1;
    depthStencilTextureDescriptor.format = kDepthStencilFormat;
    wgpu::Texture depthStencilTexture = device.CreateTexture(&depthStencilTextureDescriptor);
    utils::ComboRenderPassDescriptor renderPassDescriptor({}, depthStencilTexture.CreateView());

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.multisample.count = 1;
    renderPipelineDescriptor.vertex.module = vsModule;

    renderPipelineDescriptor.fragment = nullptr;

    renderPipelineDescriptor.EnableDepthStencil(kDepthStencilFormat);

    wgpu::RenderPipeline vertexOnlyPipeline =
        device.CreateRenderPipeline(&renderPipelineDescriptor);

    // Vertex-only render pipeline can work with depth stencil attachment and no color target
    {
        utils::ComboRenderPassDescriptor renderPassDescriptor({}, depthStencilTexture.CreateView());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(vertexOnlyPipeline);
        renderPass.End();

        encoder.Finish();
    }

    // Vertex-only render pipeline must have a depth stencil attachment
    {
        utils::ComboRenderPassDescriptor renderPassDescriptor({});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(vertexOnlyPipeline);
        renderPass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Vertex-only render pipeline can not work with color target
    {
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()},
                                                              depthStencilTexture.CreateView());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(vertexOnlyPipeline);
        renderPass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Vertex-only render pipeline can not work with color target, and must have a depth stencil
    // attachment
    {
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(vertexOnlyPipeline);
        renderPass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Tests that the sample count of the render pipeline must be valid
// when the alphaToCoverage mode is enabled.
TEST_F(RenderPipelineValidationTest, AlphaToCoverageAndSampleCount) {
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;

        device.CreateRenderPipeline(&descriptor);
    }

    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 1;
        descriptor.multisample.alphaToCoverageEnabled = true;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Tests that the texture component type in shader must match the bind group layout.
TEST_F(RenderPipelineValidationTest, TextureComponentTypeCompatibility) {
    constexpr uint32_t kNumTextureComponentType = 3u;
    std::array<const char*, kNumTextureComponentType> kScalarTypes = {{"f32", "i32", "u32"}};
    std::array<wgpu::TextureSampleType, kNumTextureComponentType> kTextureComponentTypes = {{
        wgpu::TextureSampleType::Float,
        wgpu::TextureSampleType::Sint,
        wgpu::TextureSampleType::Uint,
    }};

    for (size_t i = 0; i < kNumTextureComponentType; ++i) {
        for (size_t j = 0; j < kNumTextureComponentType; ++j) {
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = vsModule;

            std::ostringstream stream;
            stream << R"(
                @group(0) @binding(0) var myTexture : texture_2d<)"
                   << kScalarTypes[i] << R"(>;

                @stage(fragment) fn main() {
                    textureDimensions(myTexture);
                })";
            descriptor.cFragment.module = utils::CreateShaderModule(device, stream.str().c_str());
            descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

            wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Fragment, kTextureComponentTypes[j]}});
            descriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);

            if (i == j) {
                device.CreateRenderPipeline(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
            }
        }
    }
}

// Tests that the texture view dimension in shader must match the bind group layout.
TEST_F(RenderPipelineValidationTest, TextureViewDimensionCompatibility) {
    constexpr uint32_t kNumTextureViewDimensions = 6u;
    std::array<const char*, kNumTextureViewDimensions> kTextureKeywords = {{
        "texture_1d",
        "texture_2d",
        "texture_2d_array",
        "texture_cube",
        "texture_cube_array",
        "texture_3d",
    }};

    std::array<wgpu::TextureViewDimension, kNumTextureViewDimensions> kTextureViewDimensions = {{
        wgpu::TextureViewDimension::e1D,
        wgpu::TextureViewDimension::e2D,
        wgpu::TextureViewDimension::e2DArray,
        wgpu::TextureViewDimension::Cube,
        wgpu::TextureViewDimension::CubeArray,
        wgpu::TextureViewDimension::e3D,
    }};

    for (size_t i = 0; i < kNumTextureViewDimensions; ++i) {
        for (size_t j = 0; j < kNumTextureViewDimensions; ++j) {
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = vsModule;

            std::ostringstream stream;
            stream << R"(
                @group(0) @binding(0) var myTexture : )"
                   << kTextureKeywords[i] << R"(<f32>;
                @stage(fragment) fn main() {
                    textureDimensions(myTexture);
                })";
            descriptor.cFragment.module = utils::CreateShaderModule(device, stream.str().c_str());
            descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

            wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float,
                          kTextureViewDimensions[j]}});
            descriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);

            if (i == j) {
                device.CreateRenderPipeline(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
            }
        }
    }
}

// Test that declaring a storage buffer in the vertex shader without setting pipeline layout won't
// cause crash.
TEST_F(RenderPipelineValidationTest, StorageBufferInVertexShaderNoLayout) {
    wgpu::ShaderModule vsModuleWithStorageBuffer = utils::CreateShaderModule(device, R"(
        struct Dst {
            data : array<u32, 100>
        }
        @group(0) @binding(0) var<storage, read_write> dst : Dst;
        @stage(vertex) fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
            dst.data[VertexIndex] = 0x1234u;
            return vec4<f32>();
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModuleWithStorageBuffer;
    descriptor.cFragment.module = fsModule;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Tests that only strip primitive topologies allow an index format
TEST_F(RenderPipelineValidationTest, StripIndexFormatAllowed) {
    constexpr uint32_t kNumStripType = 2u;
    constexpr uint32_t kNumListType = 3u;
    constexpr uint32_t kNumIndexFormat = 3u;

    std::array<wgpu::PrimitiveTopology, kNumStripType> kStripTopologyTypes = {
        {wgpu::PrimitiveTopology::LineStrip, wgpu::PrimitiveTopology::TriangleStrip}};

    std::array<wgpu::PrimitiveTopology, kNumListType> kListTopologyTypes = {
        {wgpu::PrimitiveTopology::PointList, wgpu::PrimitiveTopology::LineList,
         wgpu::PrimitiveTopology::TriangleList}};

    std::array<wgpu::IndexFormat, kNumIndexFormat> kIndexFormatTypes = {
        {wgpu::IndexFormat::Undefined, wgpu::IndexFormat::Uint16, wgpu::IndexFormat::Uint32}};

    for (wgpu::PrimitiveTopology primitiveTopology : kStripTopologyTypes) {
        for (wgpu::IndexFormat indexFormat : kIndexFormatTypes) {
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = vsModule;
            descriptor.cFragment.module = fsModule;
            descriptor.primitive.topology = primitiveTopology;
            descriptor.primitive.stripIndexFormat = indexFormat;

            // Always succeeds, regardless of if an index format is given.
            device.CreateRenderPipeline(&descriptor);
        }
    }

    for (wgpu::PrimitiveTopology primitiveTopology : kListTopologyTypes) {
        for (wgpu::IndexFormat indexFormat : kIndexFormatTypes) {
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = vsModule;
            descriptor.cFragment.module = fsModule;
            descriptor.primitive.topology = primitiveTopology;
            descriptor.primitive.stripIndexFormat = indexFormat;

            if (indexFormat == wgpu::IndexFormat::Undefined) {
                // Succeeds even when the index format is undefined because the
                // primitive topology isn't a strip type.
                device.CreateRenderPipeline(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
            }
        }
    }
}

// Test that specifying a clampDepth value results in an error if the feature is not enabled.
TEST_F(RenderPipelineValidationTest, ClampDepthWithoutFeature) {
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::PrimitiveDepthClampingState clampingState;
        clampingState.clampDepth = true;
        descriptor.primitive.nextInChain = &clampingState;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::PrimitiveDepthClampingState clampingState;
        clampingState.clampDepth = false;
        descriptor.primitive.nextInChain = &clampingState;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Test that depthStencil.depthCompare must not be undefiend.
TEST_F(RenderPipelineValidationTest, DepthCompareUndefinedIsError) {
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth32Float);

    // Control case: Always is valid.
    descriptor.cDepthStencil.depthCompare = wgpu::CompareFunction::Always;
    device.CreateRenderPipeline(&descriptor);

    // Error case: Undefined is invalid.
    descriptor.cDepthStencil.depthCompare = wgpu::CompareFunction::Undefined;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test that the entryPoint names must be present for the correct stage in the shader module.
TEST_F(RenderPipelineValidationTest, EntryPointNameValidation) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @stage(vertex) fn vertex_main() -> @builtin(position) vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 1.0);
        }

        @stage(fragment) fn fragment_main() -> @location(0) vec4<f32> {
            return vec4<f32>(1.0, 0.0, 0.0, 1.0);
        }
    )");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = module;
    descriptor.vertex.entryPoint = "vertex_main";
    descriptor.cFragment.module = module;
    descriptor.cFragment.entryPoint = "fragment_main";

    // Success case.
    device.CreateRenderPipeline(&descriptor);

    // Test for the vertex stage entryPoint name.
    {
        // The entryPoint name doesn't exist in the module.
        descriptor.vertex.entryPoint = "main";
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

        // The entryPoint name exists, but not for the correct stage.
        descriptor.vertex.entryPoint = "fragment_main";
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    descriptor.vertex.entryPoint = "vertex_main";

    // Test for the fragment stage entryPoint name.
    {
        // The entryPoint name doesn't exist in the module.
        descriptor.cFragment.entryPoint = "main";
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

        // The entryPoint name exists, but not for the correct stage.
        descriptor.cFragment.entryPoint = "vertex_main";
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Test that vertex attrib validation is for the correct entryPoint
TEST_F(RenderPipelineValidationTest, VertexAttribCorrectEntryPoint) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @stage(vertex) fn vertex0(@location(0) attrib0 : vec4<f32>)
                                    -> @builtin(position) vec4<f32> {
            return attrib0;
        }
        @stage(vertex) fn vertex1(@location(1) attrib1 : vec4<f32>)
                                    -> @builtin(position) vec4<f32> {
            return attrib1;
        }
    )");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = module;
    descriptor.cFragment.module = fsModule;

    descriptor.vertex.bufferCount = 1;
    descriptor.cBuffers[0].attributeCount = 1;
    descriptor.cBuffers[0].arrayStride = 16;
    descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
    descriptor.cAttributes[0].offset = 0;

    // Success cases, the attribute used by the entryPoint is declared in the pipeline.
    descriptor.vertex.entryPoint = "vertex0";
    descriptor.cAttributes[0].shaderLocation = 0;
    device.CreateRenderPipeline(&descriptor);

    descriptor.vertex.entryPoint = "vertex1";
    descriptor.cAttributes[0].shaderLocation = 1;
    device.CreateRenderPipeline(&descriptor);

    // Error cases, the attribute used by the entryPoint isn't declared in the pipeline.
    descriptor.vertex.entryPoint = "vertex1";
    descriptor.cAttributes[0].shaderLocation = 0;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

    descriptor.vertex.entryPoint = "vertex0";
    descriptor.cAttributes[0].shaderLocation = 1;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test that fragment output validation is for the correct entryPoint
TEST_F(RenderPipelineValidationTest, FragmentOutputCorrectEntryPoint) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @stage(fragment) fn fragmentFloat() -> @location(0) vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 0.0);
        }
        @stage(fragment) fn fragmentUint() -> @location(0) vec4<u32> {
            return vec4<u32>(0u, 0u, 0u, 0u);
        }
    )");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = module;

    // Success case, the component type matches between the pipeline and the entryPoint
    descriptor.cFragment.entryPoint = "fragmentFloat";
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA32Float;
    device.CreateRenderPipeline(&descriptor);

    descriptor.cFragment.entryPoint = "fragmentUint";
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA32Uint;
    device.CreateRenderPipeline(&descriptor);

    // Error case, the component type doesn't match between the pipeline and the entryPoint
    descriptor.cFragment.entryPoint = "fragmentUint";
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA32Float;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

    descriptor.cFragment.entryPoint = "fragmentFloat";
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA32Uint;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test that unwritten fragment outputs must have a write mask of 0.
TEST_F(RenderPipelineValidationTest, UnwrittenFragmentOutputsMask0) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @stage(vertex) fn main() -> @builtin(position) vec4<f32> {
            return vec4<f32>();
        }
    )");

    wgpu::ShaderModule fsModuleWriteNone = utils::CreateShaderModule(device, R"(
        @stage(fragment) fn main() {}
    )");

    wgpu::ShaderModule fsModuleWrite0 = utils::CreateShaderModule(device, R"(
        @stage(fragment) fn main() -> @location(0) vec4<f32> {
            return vec4<f32>();
        }
    )");

    wgpu::ShaderModule fsModuleWrite1 = utils::CreateShaderModule(device, R"(
        @stage(fragment) fn main() -> @location(1) vec4<f32> {
            return vec4<f32>();
        }
    )");

    wgpu::ShaderModule fsModuleWriteBoth = utils::CreateShaderModule(device, R"(
        struct FragmentOut {
            @location(0) target0 : vec4<f32>,
            @location(1) target1 : vec4<f32>,
        }
        @stage(fragment) fn main() -> FragmentOut {
            var out : FragmentOut;
            return out;
        }
    )");

    // Control case: write to target 0
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 1;
        descriptor.cFragment.module = fsModuleWrite0;
        device.CreateRenderPipeline(&descriptor);
    }

    // Control case: write to target 0 and target 1
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 2;
        descriptor.cFragment.module = fsModuleWriteBoth;
        device.CreateRenderPipeline(&descriptor);
    }

    // Write only target 1 (not in pipeline fragment state).
    // Errors because target 0 does not have a write mask of 0.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 1;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::All;
        descriptor.cFragment.module = fsModuleWrite1;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // Write only target 1 (not in pipeline fragment state).
    // OK because target 0 has a write mask of 0.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 1;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        descriptor.cFragment.module = fsModuleWrite1;
        device.CreateRenderPipeline(&descriptor);
    }

    // Write only target 0 with two color targets.
    // Errors because target 1 does not have a write mask of 0.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 2;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::Red;
        descriptor.cTargets[1].writeMask = wgpu::ColorWriteMask::Alpha;
        descriptor.cFragment.module = fsModuleWrite0;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // Write only target 0 with two color targets.
    // OK because target 1 has a write mask of 0.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 2;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::All;
        descriptor.cTargets[1].writeMask = wgpu::ColorWriteMask::None;
        descriptor.cFragment.module = fsModuleWrite0;
        device.CreateRenderPipeline(&descriptor);
    }

    // Write nothing with two color targets.
    // Errors because both target 0 and 1 have nonzero write masks.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 2;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::Red;
        descriptor.cTargets[1].writeMask = wgpu::ColorWriteMask::Green;
        descriptor.cFragment.module = fsModuleWriteNone;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // Write nothing with two color targets.
    // OK because target 0 and 1 have write masks of 0.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 2;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        descriptor.cTargets[1].writeMask = wgpu::ColorWriteMask::None;
        descriptor.cFragment.module = fsModuleWriteNone;
        device.CreateRenderPipeline(&descriptor);
    }
}

// Test that fragment output validation is for the correct entryPoint
TEST_F(RenderPipelineValidationTest, BindingsFromCorrectEntryPoint) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct Uniforms {
            data : vec4<f32>
        }
        @group(0) @binding(0) var<uniform> var0 : Uniforms;
        @group(0) @binding(1) var<uniform> var1 : Uniforms;

        @stage(vertex) fn vertex0() -> @builtin(position) vec4<f32> {
            return var0.data;
        }
        @stage(vertex) fn vertex1() -> @builtin(position) vec4<f32> {
            return var1.data;
        }
    )");

    wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}});
    wgpu::PipelineLayout layout0 = utils::MakeBasicPipelineLayout(device, &bgl0);

    wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}});
    wgpu::PipelineLayout layout1 = utils::MakeBasicPipelineLayout(device, &bgl1);

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = module;
    descriptor.cFragment.module = fsModule;

    // Success case, the BGL matches the bindings used by the entryPoint
    descriptor.vertex.entryPoint = "vertex0";
    descriptor.layout = layout0;
    device.CreateRenderPipeline(&descriptor);

    descriptor.vertex.entryPoint = "vertex1";
    descriptor.layout = layout1;
    device.CreateRenderPipeline(&descriptor);

    // Error case, the BGL doesn't match the bindings used by the entryPoint
    descriptor.vertex.entryPoint = "vertex1";
    descriptor.layout = layout0;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

    descriptor.vertex.entryPoint = "vertex0";
    descriptor.layout = layout1;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

class DepthClampingValidationTest : public RenderPipelineValidationTest {
  protected:
    WGPUDevice CreateTestDevice() override {
        wgpu::DeviceDescriptor descriptor;
        wgpu::FeatureName requiredFeatures[1] = {wgpu::FeatureName::DepthClamping};
        descriptor.requiredFeatures = requiredFeatures;
        descriptor.requiredFeaturesCount = 1;
        return adapter.CreateDevice(&descriptor);
    }
};

// Tests that specifying a clampDepth value succeeds if the feature is enabled.
TEST_F(DepthClampingValidationTest, Success) {
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::PrimitiveDepthClampingState clampingState;
        clampingState.clampDepth = true;
        descriptor.primitive.nextInChain = &clampingState;
        device.CreateRenderPipeline(&descriptor);
    }
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::PrimitiveDepthClampingState clampingState;
        clampingState.clampDepth = false;
        descriptor.primitive.nextInChain = &clampingState;
        device.CreateRenderPipeline(&descriptor);
    }
}

class InterStageVariableMatchingValidationTest : public RenderPipelineValidationTest {
  protected:
    void CheckCreatingRenderPipeline(wgpu::ShaderModule vertexModule,
                                     wgpu::ShaderModule fragmentModule,
                                     bool shouldSucceed) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vertexModule;
        descriptor.cFragment.module = fragmentModule;
        if (shouldSucceed) {
            device.CreateRenderPipeline(&descriptor);
        } else {
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
        }
    }
};

// Tests that creating render pipeline should fail when there is a vertex output that doesn't have
// its corresponding fragment input at the same location, and there is a fragment input that
// doesn't have its corresponding vertex output at the same location.
TEST_F(InterStageVariableMatchingValidationTest, MissingDeclarationAtSameLocation) {
    wgpu::ShaderModule vertexModuleOutputAtLocation0 = utils::CreateShaderModule(device, R"(
            struct A {
                @location(0) vout: f32,
                @builtin(position) pos: vec4<f32>,
            }
            @stage(vertex) fn main() -> A {
                var vertexOut: A;
                vertexOut.pos = vec4<f32>(0.0, 0.0, 0.0, 1.0);
                return vertexOut;
            })");
    wgpu::ShaderModule fragmentModuleAtLocation0 = utils::CreateShaderModule(device, R"(
            struct B {
                @location(0) fin: f32
            }
            @stage(fragment) fn main(fragmentIn: B) -> @location(0) vec4<f32>  {
                return vec4<f32>(fragmentIn.fin, 0.0, 0.0, 1.0);
            })");
    wgpu::ShaderModule fragmentModuleInputAtLocation1 = utils::CreateShaderModule(device, R"(
            struct A {
                @location(1) vout: f32
            }
            @stage(fragment) fn main(vertexOut: A) -> @location(0) vec4<f32>  {
                return vec4<f32>(vertexOut.vout, 0.0, 0.0, 1.0);
            })");
    wgpu::ShaderModule vertexModuleOutputAtLocation1 = utils::CreateShaderModule(device, R"(
            struct B {
                @location(1) fin: f32,
                @builtin(position) pos: vec4<f32>,
            }
            @stage(vertex) fn main() -> B {
                var fragmentIn: B;
                fragmentIn.pos = vec4<f32>(0.0, 0.0, 0.0, 1.0);
                return fragmentIn;
            })");

    {
        CheckCreatingRenderPipeline(vertexModuleOutputAtLocation0, fsModule, false);
        CheckCreatingRenderPipeline(vsModule, fragmentModuleAtLocation0, false);
        CheckCreatingRenderPipeline(vertexModuleOutputAtLocation0, fragmentModuleInputAtLocation1,
                                    false);
        CheckCreatingRenderPipeline(vertexModuleOutputAtLocation1, fragmentModuleAtLocation0,
                                    false);
    }

    {
        CheckCreatingRenderPipeline(vertexModuleOutputAtLocation0, fragmentModuleAtLocation0, true);
        CheckCreatingRenderPipeline(vertexModuleOutputAtLocation1, fragmentModuleInputAtLocation1,
                                    true);
    }
}

// Tests that creating render pipeline should fail when the type of a vertex stage output variable
// doesn't match the type of the fragment stage input variable at the same location.
TEST_F(InterStageVariableMatchingValidationTest, DifferentTypeAtSameLocation) {
    constexpr std::array<const char*, 12> kTypes = {{"f32", "vec2<f32>", "vec3<f32>", "vec4<f32>",
                                                     "i32", "vec2<i32>", "vec3<i32>", "vec4<i32>",
                                                     "u32", "vec2<u32>", "vec3<u32>", "vec4<u32>"}};

    std::array<wgpu::ShaderModule, 12> vertexModules;
    std::array<wgpu::ShaderModule, 12> fragmentModules;
    for (uint32_t i = 0; i < kTypes.size(); ++i) {
        std::string interfaceDeclaration;
        {
            std::ostringstream sstream;
            sstream << "struct A { @location(0) @interpolate(flat) a: " << kTypes[i] << ","
                    << std::endl;
            interfaceDeclaration = sstream.str();
        }
        {
            std::ostringstream vertexStream;
            vertexStream << interfaceDeclaration << R"(
                    @builtin(position) pos: vec4<f32>,
                }
                @stage(vertex) fn main() -> A {
                    var vertexOut: A;
                    vertexOut.pos = vec4<f32>(0.0, 0.0, 0.0, 1.0);
                    return vertexOut;
                })";
            vertexModules[i] = utils::CreateShaderModule(device, vertexStream.str().c_str());
        }
        {
            std::ostringstream fragmentStream;
            fragmentStream << interfaceDeclaration << R"(
                }
                @stage(fragment) fn main(fragmentIn: A) -> @location(0) vec4<f32> {
                    return vec4<f32>(0.0, 0.0, 0.0, 1.0);
                })";
            fragmentModules[i] = utils::CreateShaderModule(device, fragmentStream.str().c_str());
        }
    }

    for (uint32_t vertexModuleIndex = 0; vertexModuleIndex < kTypes.size(); ++vertexModuleIndex) {
        wgpu::ShaderModule vertexModule = vertexModules[vertexModuleIndex];
        for (uint32_t fragmentModuleIndex = 0; fragmentModuleIndex < kTypes.size();
             ++fragmentModuleIndex) {
            wgpu::ShaderModule fragmentModule = fragmentModules[fragmentModuleIndex];
            bool shouldSuccess = vertexModuleIndex == fragmentModuleIndex;
            CheckCreatingRenderPipeline(vertexModule, fragmentModule, shouldSuccess);
        }
    }
}

// Tests that creating render pipeline should fail when the interpolation attribute of a vertex
// stage output variable doesn't match the type of the fragment stage input variable at the same
// location.
TEST_F(InterStageVariableMatchingValidationTest, DifferentInterpolationAttributeAtSameLocation) {
    enum class InterpolationType : uint8_t {
        None = 0,
        Perspective,
        Linear,
        Flat,
        Count,
    };
    enum class InterpolationSampling : uint8_t {
        None = 0,
        Center,
        Centroid,
        Sample,
        Count,
    };
    constexpr std::array<const char*, static_cast<size_t>(InterpolationType::Count)>
        kInterpolationTypeString = {{"", "perspective", "linear", "flat"}};
    constexpr std::array<const char*, static_cast<size_t>(InterpolationSampling::Count)>
        kInterpolationSamplingString = {{"", "center", "centroid", "sample"}};

    struct InterpolationAttribute {
        InterpolationType interpolationType;
        InterpolationSampling interpolationSampling;
    };

    // Interpolation sampling is not used with flat interpolation.
    constexpr std::array<InterpolationAttribute, 10> validInterpolationAttributes = {{
        {InterpolationType::None, InterpolationSampling::None},
        {InterpolationType::Flat, InterpolationSampling::None},
        {InterpolationType::Linear, InterpolationSampling::None},
        {InterpolationType::Linear, InterpolationSampling::Center},
        {InterpolationType::Linear, InterpolationSampling::Centroid},
        {InterpolationType::Linear, InterpolationSampling::Sample},
        {InterpolationType::Perspective, InterpolationSampling::None},
        {InterpolationType::Perspective, InterpolationSampling::Center},
        {InterpolationType::Perspective, InterpolationSampling::Centroid},
        {InterpolationType::Perspective, InterpolationSampling::Sample},
    }};

    std::vector<wgpu::ShaderModule> vertexModules(validInterpolationAttributes.size());
    std::vector<wgpu::ShaderModule> fragmentModules(validInterpolationAttributes.size());
    for (uint32_t i = 0; i < validInterpolationAttributes.size(); ++i) {
        std::string interfaceDeclaration;
        {
            const auto& interpolationAttribute = validInterpolationAttributes[i];
            std::ostringstream sstream;
            sstream << "struct A { @location(0)";
            if (interpolationAttribute.interpolationType != InterpolationType::None) {
                sstream << " @interpolate("
                        << kInterpolationTypeString[static_cast<uint8_t>(
                               interpolationAttribute.interpolationType)];
                if (interpolationAttribute.interpolationSampling != InterpolationSampling::None) {
                    sstream << ", "
                            << kInterpolationSamplingString[static_cast<uint8_t>(
                                   interpolationAttribute.interpolationSampling)];
                }
                sstream << ")";
            }
            sstream << " a : vec4<f32>," << std::endl;
            interfaceDeclaration = sstream.str();
        }
        {
            std::ostringstream vertexStream;
            vertexStream << interfaceDeclaration << R"(
                    @builtin(position) pos: vec4<f32>,
                }
                @stage(vertex) fn main() -> A {
                    var vertexOut: A;
                    vertexOut.pos = vec4<f32>(0.0, 0.0, 0.0, 1.0);
                    return vertexOut;
                })";
            vertexModules[i] = utils::CreateShaderModule(device, vertexStream.str().c_str());
        }
        {
            std::ostringstream fragmentStream;
            fragmentStream << interfaceDeclaration << R"(
                }
                @stage(fragment) fn main(fragmentIn: A) -> @location(0) vec4<f32> {
                    return fragmentIn.a;
                })";
            fragmentModules[i] = utils::CreateShaderModule(device, fragmentStream.str().c_str());
        }
    }

    auto GetAppliedInterpolationAttribute = [](const InterpolationAttribute& attribute) {
        InterpolationAttribute appliedAttribute = {attribute.interpolationType,
                                                   attribute.interpolationSampling};
        switch (attribute.interpolationType) {
            // If the interpolation attribute is not specified, then
            // @interpolate(perspective, center) or @interpolate(perspective) is assumed.
            case InterpolationType::None:
                appliedAttribute.interpolationType = InterpolationType::Perspective;
                appliedAttribute.interpolationSampling = InterpolationSampling::Center;
                break;

            // If the interpolation type is perspective or linear, and the interpolation
            // sampling is not specified, then 'center' is assumed.
            case InterpolationType::Perspective:
            case InterpolationType::Linear:
                if (appliedAttribute.interpolationSampling == InterpolationSampling::None) {
                    appliedAttribute.interpolationSampling = InterpolationSampling::Center;
                }
                break;

            case InterpolationType::Flat:
                break;
            default:
                UNREACHABLE();
        }
        return appliedAttribute;
    };

    auto InterpolationAttributeMatch = [GetAppliedInterpolationAttribute](
                                           const InterpolationAttribute& attribute1,
                                           const InterpolationAttribute& attribute2) {
        InterpolationAttribute appliedAttribute1 = GetAppliedInterpolationAttribute(attribute1);
        InterpolationAttribute appliedAttribute2 = GetAppliedInterpolationAttribute(attribute2);

        return appliedAttribute1.interpolationType == appliedAttribute2.interpolationType &&
               appliedAttribute1.interpolationSampling == appliedAttribute2.interpolationSampling;
    };

    for (uint32_t vertexModuleIndex = 0; vertexModuleIndex < validInterpolationAttributes.size();
         ++vertexModuleIndex) {
        wgpu::ShaderModule vertexModule = vertexModules[vertexModuleIndex];
        for (uint32_t fragmentModuleIndex = 0;
             fragmentModuleIndex < validInterpolationAttributes.size(); ++fragmentModuleIndex) {
            wgpu::ShaderModule fragmentModule = fragmentModules[fragmentModuleIndex];
            bool shouldSuccess =
                InterpolationAttributeMatch(validInterpolationAttributes[vertexModuleIndex],
                                            validInterpolationAttributes[fragmentModuleIndex]);
            CheckCreatingRenderPipeline(vertexModule, fragmentModule, shouldSuccess);
        }
    }
}
