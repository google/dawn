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
#include "dawn/tests/unittests/validation/DeprecatedAPITests.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

class RenderPipelineValidationTest : public ValidationTest {
  protected:
    WGPUDevice CreateTestDevice(dawn::native::Adapter dawnAdapter) override {
        wgpu::DeviceDescriptor descriptor;
        wgpu::FeatureName requiredFeatures[1] = {wgpu::FeatureName::ShaderF16};
        descriptor.requiredFeatures = requiredFeatures;
        descriptor.requiredFeaturesCount = 1;

        return dawnAdapter.CreateDevice(&descriptor);
    }

    void SetUp() override {
        ValidationTest::SetUp();

        vsModule = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            })");

        fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            })");

        fsModuleUint = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4u {
                return vec4u(0u, 255u, 0u, 255u);
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

// Tests that depth attachment is required when frag_depth is written in fragment stage.
TEST_F(RenderPipelineValidationTest, DepthAttachmentRequiredWhenFragDepthIsWritten) {
    wgpu::ShaderModule fsModuleFragDepthOutput = utils::CreateShaderModule(device, R"(
        struct Output {
            @builtin(frag_depth) depth_out: f32,
            @location(0) color : vec4f,
        }
        @fragment fn main() -> Output {
            var o: Output;
            // We need to make sure this frag_depth isn't optimized out even its value equals "no op".
            o.depth_out = 0.5;
            o.color = vec4f(1.0, 1.0, 1.0, 1.0);
            return o;
        }
    )");

    {
        // Succeeds because there is depth stencil state.
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleFragDepthOutput;
        descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24PlusStencil8);

        device.CreateRenderPipeline(&descriptor);
    }

    {
        // Fails because there is no depth stencil state.
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleFragDepthOutput;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    {
        // Fails because there is depth stencil state but no depth aspect.
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleFragDepthOutput;
        descriptor.EnableDepthStencil(wgpu::TextureFormat::Stencil8);

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
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

// Tests that target blend must not be set if the format is undefined.
TEST_F(RenderPipelineValidationTest, UndefinedColorStateFormatWithBlend) {
    {
        // Control case: Valid undefined format target.
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cFragment.targetCount = 1;
        descriptor.cTargets[0].format = wgpu::TextureFormat::Undefined;

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
}

// Tests that a color target that's present in the pipeline descriptor but not in the shader must
// have its writeMask set to 0.
TEST_F(RenderPipelineValidationTest, WriteMaskMustBeZeroForColorTargetWithNoShaderOutput) {
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cFragment.targetCount = 2;
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.cTargets[1].format = wgpu::TextureFormat::RGBA8Unorm;

    // Control case: Target 1 not output by the shader but has writeMask = 0
    descriptor.cTargets[1].writeMask = wgpu::ColorWriteMask::None;
    device.CreateRenderPipeline(&descriptor);

    // Error case: the writeMask is not 0.
    descriptor.cTargets[1].writeMask = wgpu::ColorWriteMask::Red;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
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
    std::vector<std::vector<std::string>> kScalarTypeLists = {// Float scalar types
                                                              {"f32", "f16"},
                                                              // Sint scalar type
                                                              {"i32"},
                                                              // Uint scalar type
                                                              {"u32"}};

    std::vector<std::vector<wgpu::TextureFormat>> kColorFormatLists = {
        // Float color formats
        {wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA16Float,
         wgpu::TextureFormat::RGBA32Float},
        // Sint color formats
        {wgpu::TextureFormat::RGBA8Sint, wgpu::TextureFormat::RGBA16Sint,
         wgpu::TextureFormat::RGBA32Sint},
        // Uint color formats
        {wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA16Uint,
         wgpu::TextureFormat::RGBA32Uint}};

    for (size_t i = 0; i < kScalarTypeLists.size(); ++i) {
        for (const std::string& scalarType : kScalarTypeLists[i]) {
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = vsModule;
            std::ostringstream stream;

            // Enable f16 extension if needed.
            if (scalarType == "f16") {
                stream << "enable f16;\n\n";
            }
            stream << R"(
            @fragment fn main() -> @location(0) vec4<)"
                   << scalarType << R"(> {
                var result : vec4<)"
                   << scalarType << R"(>;
                return result;
            })";

            descriptor.cFragment.module = utils::CreateShaderModule(device, stream.str().c_str());

            for (size_t j = 0; j < kColorFormatLists.size(); ++j) {
                for (wgpu::TextureFormat textureFormat : kColorFormatLists[j]) {
                    descriptor.cTargets[0].format = textureFormat;
                    if (i == j) {
                        device.CreateRenderPipeline(&descriptor);
                    } else {
                        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
                    }
                }
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
            @fragment fn main() -> @location(0) )";
        switch (componentCount) {
            case 1:
                stream << R"(f32 {
                return 1.0;
                })";
                break;
            case 2:
                stream << R"(vec2f {
                return vec2f(1.0, 1.0);
                })";
                break;
            case 3:
                stream << R"(vec3f {
                return vec3f(1.0, 1.0, 1.0);
                })";
                break;
            case 4:
                stream << R"(vec4f {
                return vec4f(1.0, 1.0, 1.0, 1.0);
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
        utils::ComboRenderPassDescriptor renderPassDescriptor;

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

// Tests if the sample_mask builtin is a pipeline output of fragment shader,
// then alphaToCoverageEnabled must be false.
TEST_F(RenderPipelineValidationTest, AlphaToCoverageAndSampleMaskOutput) {
    wgpu::ShaderModule fsModuleSampleMaskOutput = utils::CreateShaderModule(device, R"(
        struct Output {
            @builtin(sample_mask) mask_out: u32,
            @location(0) color : vec4f,
        }
        @fragment fn main() -> Output {
            var o: Output;
            // We need to make sure this sample_mask isn't optimized out even its value equals "no op".
            o.mask_out = 0xFFFFFFFFu;
            o.color = vec4f(1.0, 1.0, 1.0, 1.0);
            return o;
        }
    )");

    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleSampleMaskOutput;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = false;

        device.CreateRenderPipeline(&descriptor);
    }

    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleSampleMaskOutput;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    {
        // Control cases: when fragment has no sample_mask output, it's good to have
        // alphaToCoverageEnabled enabled
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;

        device.CreateRenderPipeline(&descriptor);
    }
}

// Tests when alphaToCoverageEnabled is true, targets[0] must exist and have alpha channel.
TEST_F(RenderPipelineValidationTest, AlphaToCoverageAndColorTargetAlpha) {
    {
        // Control case
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;

        device.CreateRenderPipeline(&descriptor);
    }

    {
        // Fragment state must exist
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.fragment = nullptr;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    {
        // Fragment targets[0] must exist
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cFragment.targetCount = 0;
        descriptor.cFragment.targets = nullptr;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;
        descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth32Float);

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    {
        // Fragment targets[0].format must have alpha channel (only 1 target)
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::R8Unorm;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    wgpu::ShaderModule fsModule2 = utils::CreateShaderModule(device, R"(
        struct FragmentOut {
            @location(0) target0 : vec4f,
            @location(1) target1 : vec4f,
        }
        @fragment fn main() -> FragmentOut {
            var out: FragmentOut;
            out.target0 = vec4f(0, 0, 0, 1);
            out.target1 = vec4f(1, 0, 0, 0);
            return out;
        })");

    {
        // Fragment targets[0].format must have alpha channel (2 targets)
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule2;
        descriptor.cFragment.targetCount = 2;
        descriptor.cTargets[0].format = wgpu::TextureFormat::R8Unorm;
        descriptor.cTargets[1].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.multisample.count = 4;
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

                @fragment fn main() {
                    _ = textureDimensions(myTexture);
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
                @fragment fn main() {
                    _ = textureDimensions(myTexture);
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
        @vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            dst.data[VertexIndex] = 0x1234u;
            return vec4f();
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

// Test that specifying a unclippedDepth value is an error if the feature is not enabled.
TEST_F(RenderPipelineValidationTest, UnclippedDepthWithoutFeature) {
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::PrimitiveDepthClipControl depthClipControl;
        depthClipControl.unclippedDepth = true;
        descriptor.primitive.nextInChain = &depthClipControl;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor),
                            testing::HasSubstr("not supported"));
    }
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::PrimitiveDepthClipControl depthClipControl;
        depthClipControl.unclippedDepth = false;
        descriptor.primitive.nextInChain = &depthClipControl;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor),
                            testing::HasSubstr("not supported"));
    }
}

// Test that specifying an unclippedDepth value is an error if the feature is not enabled.
TEST_F(RenderPipelineValidationTest, DepthClipControlWithoutFeature) {
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::PrimitiveDepthClipControl depthClipControl;
        depthClipControl.unclippedDepth = true;
        descriptor.primitive.nextInChain = &depthClipControl;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor),
                            testing::HasSubstr("not supported"));
    }
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::PrimitiveDepthClipControl depthClipControl;
        depthClipControl.unclippedDepth = false;
        descriptor.primitive.nextInChain = &depthClipControl;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor),
                            testing::HasSubstr("not supported"));
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
        @vertex fn vertex_main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }

        @fragment fn fragment_main() -> @location(0) vec4f {
            return vec4f(1.0, 0.0, 0.0, 1.0);
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
        @vertex fn vertex0(@location(0) attrib0 : vec4f)
                                    -> @builtin(position) vec4f {
            return attrib0;
        }
        @vertex fn vertex1(@location(1) attrib1 : vec4f)
                                    -> @builtin(position) vec4f {
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
        @fragment fn fragmentFloat() -> @location(0) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        }
        @fragment fn fragmentUint() -> @location(0) vec4u {
            return vec4u(0u, 0u, 0u, 0u);
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
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f();
        }
    )");

    wgpu::ShaderModule fsModuleWriteNone = utils::CreateShaderModule(device, R"(
        @fragment fn main() {}
    )");

    wgpu::ShaderModule fsModuleWrite0 = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f();
        }
    )");

    wgpu::ShaderModule fsModuleWrite1 = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(1) vec4f {
            return vec4f();
        }
    )");

    wgpu::ShaderModule fsModuleWriteBoth = utils::CreateShaderModule(device, R"(
        struct FragmentOut {
            @location(0) target0 : vec4f,
            @location(1) target1 : vec4f,
        }
        @fragment fn main() -> FragmentOut {
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
            data : vec4f
        }
        @group(0) @binding(0) var<uniform> var0 : Uniforms;
        @group(0) @binding(1) var<uniform> var1 : Uniforms;

        @vertex fn vertex0() -> @builtin(position) vec4f {
            return var0.data;
        }
        @vertex fn vertex1() -> @builtin(position) vec4f {
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

// Test that total fragment output resource validations must be less than limit.
TEST_F(RenderPipelineValidationTest, MaxFragmentCombinedOutputResources) {
    static constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::R8Unorm;
    wgpu::ColorTargetState kEmptyColorTargetState = {};
    kEmptyColorTargetState.format = wgpu::TextureFormat::Undefined;
    wgpu::ColorTargetState kColorTargetState = {};
    kColorTargetState.format = kFormat;

    // Creates a shader with the given number of output resources.
    auto CreateShader = [&](uint32_t numBuffers, uint32_t numTextures) -> wgpu::ShaderModule {
        // Header to declare storage buffer struct.
        static constexpr std::string_view kHeader = "struct Buf { data : array<u32> }\n";
        std::ostringstream bufferBindings;
        std::ostringstream bufferOutputs;
        for (uint32_t i = 0; i < numBuffers; i++) {
            bufferBindings << "@group(0) @binding(" << i << ") var<storage, read_write> b" << i
                           << ": Buf;\n";
            bufferOutputs << "    b" << i << ".data[i] = i;\n";
        }

        std::ostringstream textureBindings;
        std::ostringstream textureOutputs;
        for (uint32_t i = 0; i < numTextures; i++) {
            textureBindings << "@group(1) @binding(" << i << ") var t" << i
                            << ": texture_storage_1d<rgba8uint, write>;\n";
            textureOutputs << "    textureStore(t" << i << ", i, vec4u(i));\n";
        }

        std::ostringstream targetBindings;
        std::ostringstream targetOutputs;
        for (size_t i = 0; i < kMaxColorAttachments; i++) {
            targetBindings << "@location(" << i << ") o" << i << " : vec4f, ";
            targetOutputs << "vec4f(1), ";
        }

        std::ostringstream fsShader;
        fsShader << kHeader;
        fsShader << bufferBindings.str();
        fsShader << textureBindings.str();
        fsShader << "struct Outputs { " << targetBindings.str() << "}\n";
        fsShader << "@fragment fn main(@builtin(sample_index) i : u32) -> Outputs {\n";
        fsShader << bufferOutputs.str();
        fsShader << textureOutputs.str();
        fsShader << "    return Outputs(" << targetOutputs.str() << ");\n";
        fsShader << "}";
        return utils::CreateShaderModule(device, fsShader.str().c_str());
    };

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
    descriptor.vertex.entryPoint = "main";
    descriptor.cFragment.targetCount = kMaxColorAttachments;
    descriptor.cFragment.entryPoint = "main";

    // Runs test using the given parameters.
    auto DoTest = [&](uint32_t numBuffers, uint32_t numTextures,
                      const std::vector<uint32_t>& attachmentIndices, bool useDepthStencil,
                      bool shouldError) {
        descriptor.cFragment.module = CreateShader(numBuffers, numTextures);
        descriptor.cTargets.fill(kEmptyColorTargetState);
        for (const uint32_t attachmentIndex : attachmentIndices) {
            descriptor.cTargets[attachmentIndex] = kColorTargetState;
        }

        if (useDepthStencil) {
            descriptor.EnableDepthStencil();
        } else {
            descriptor.DisableDepthStencil();
        }

        if (shouldError) {
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
        } else {
            device.CreateRenderPipeline(&descriptor);
        }
    };

    // All following tests assume we are using the default limits for the following:
    //   - maxStorageBuffersPerShaderStage    = 8
    //   - maxStorageTexturesPerShaderStage   = 4
    //   - maxColorAttachments                = 8
    //   - maxFragmentCombinedOutputResources = 8
    // Note we use the defaults for the validation tests because otherwise it is hard to verify that
    // we are hitting the maxFragmentCombinedOutputResources limit validations versus potentially
    // hitting other validation errors from the other limits.

    // One of each resource with and without depth-stencil should pass. (Control)
    DoTest(1, 1, {0}, false, false);
    DoTest(1, 1, {0}, true, false);

    // Max number of any single resource within limits should pass. Note we turn on depth stencil in
    // some of these cases because empty attachments are not allowed.
    DoTest(8, 0, {}, true, false);
    DoTest(4, 4, {}, true, false);
    DoTest(0, 4, {0, 1, 2, 3}, false, false);
    DoTest(0, 0, {0, 1, 2, 3, 4, 5, 6, 7}, false, false);
    DoTest(0, 0, {0, 1, 2, 3, 4, 5, 6, 7}, true, false);

    // Max number of resources with different combinations should also pass.
    DoTest(3, 2, {0, 3, 5}, false, false);
    DoTest(2, 3, {2, 4, 6}, true, false);
    DoTest(3, 3, {1, 7}, true, false);

    // Max number of resources + 1 should fail.
    DoTest(3, 3, {0, 3, 5}, false, true);
    DoTest(3, 3, {2, 4, 6}, true, true);
    DoTest(3, 3, {1, 5, 7}, true, true);
}

// Tests validation for per-pixel accounting for render targets. The tests currently assume that the
// default maxColorAttachmentBytesPerSample limit of 32 is used.
TEST_P(DeprecationTests, RenderPipelineColorAttachmentBytesPerSample) {
    // Creates a fragment shader with maximum number of color attachments to enable testing.
    auto CreateShader = [&](const std::vector<wgpu::TextureFormat>& formats) -> wgpu::ShaderModule {
        // Default type to use when formats.size() < kMaxColorAttachments.
        static constexpr std::string_view kDefaultWgslType = "vec4f";

        std::ostringstream bindings;
        std::ostringstream outputs;
        for (size_t i = 0; i < kMaxColorAttachments; i++) {
            if (i < formats.size()) {
                std::ostringstream type;
                type << "vec4<" << utils::GetWGSLColorTextureComponentType(formats.at(i)) << ">";
                bindings << "@location(" << i << ") o" << i << " : " << type.str() << ", ";
                outputs << type.str() << "(1), ";
            } else {
                bindings << "@location(" << i << ") o" << i << " : " << kDefaultWgslType << ", ";
                outputs << kDefaultWgslType << "(1), ";
            }
        }

        std::ostringstream fsShader;
        fsShader << "struct Outputs { " << bindings.str() << "}\n";
        fsShader << "@fragment fn main() -> Outputs {\n";
        fsShader << "    return Outputs(" << outputs.str() << ");\n";
        fsShader << "}";
        return utils::CreateShaderModule(device, fsShader.str().c_str());
    };

    struct TestCase {
        std::vector<wgpu::TextureFormat> formats;
        bool success;
    };
    static std::vector<TestCase> kTestCases = {
        // Simple 1 format cases.

        // R8Unorm take 1 byte and are aligned to 1 byte so we can have 8 (max).
        {{wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm},
         true},
        // RGBA8Uint takes 4 bytes and are aligned to 1 byte so we can have 8 (max).
        {{wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint},
         true},
        // RGBA8Unorm takes 8 bytes (special case) and are aligned to 1 byte so only 4 allowed.
        {{wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm},
         true},
        {{wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm},
         false},
        // RGBA32Float takes 16 bytes and are aligned to 4 bytes so only 2 are allowed.
        {{wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::RGBA32Float}, true},
        {{wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::RGBA32Float,
          wgpu::TextureFormat::RGBA32Float},
         false},

        // Different format alignment cases.

        // Alignment causes the first 1 byte R8Unorm to become 4 bytes. So even though 1+4+8+16+1 <
        // 32, the 4 byte alignment requirement of R32Float makes the first R8Unorm become 4 and
        // 4+4+8+16+1 > 32. Re-ordering this so the R8Unorm's are at the end, however is allowed:
        // 4+8+16+1+1 < 32.
        {{wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R32Float,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA32Float,
          wgpu::TextureFormat::R8Unorm},
         false},
        {{wgpu::TextureFormat::R32Float, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm},
         true},
    };

    for (const TestCase& testCase : kTestCases) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            })");
        descriptor.vertex.entryPoint = "main";
        descriptor.cFragment.module = CreateShader(testCase.formats);
        descriptor.cFragment.entryPoint = "main";
        descriptor.cFragment.targetCount = testCase.formats.size();
        for (size_t i = 0; i < testCase.formats.size(); i++) {
            descriptor.cTargets[i].format = testCase.formats.at(i);
        }
        if (testCase.success) {
            device.CreateRenderPipeline(&descriptor);
        } else {
            EXPECT_DEPRECATION_ERROR_OR_WARNING(device.CreateRenderPipeline(&descriptor));
        }
    }
}

class DepthClipControlValidationTest : public RenderPipelineValidationTest {
  protected:
    WGPUDevice CreateTestDevice(dawn::native::Adapter dawnAdapter) override {
        wgpu::DeviceDescriptor descriptor;
        wgpu::FeatureName requiredFeatures[1] = {wgpu::FeatureName::DepthClipControl};
        descriptor.requiredFeatures = requiredFeatures;
        descriptor.requiredFeaturesCount = 1;
        return dawnAdapter.CreateDevice(&descriptor);
    }
};

// Tests that specifying a unclippedDepth value succeeds if the feature is enabled.
TEST_F(DepthClipControlValidationTest, Success) {
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::PrimitiveDepthClipControl depthClipControl;
        depthClipControl.unclippedDepth = true;
        descriptor.primitive.nextInChain = &depthClipControl;
        device.CreateRenderPipeline(&descriptor);
    }
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::PrimitiveDepthClipControl depthClipControl;
        depthClipControl.unclippedDepth = false;
        descriptor.primitive.nextInChain = &depthClipControl;
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

// Tests that creating render pipeline should fail when there is a fragment input that
// doesn't have its corresponding vertex output at the same location.
TEST_F(InterStageVariableMatchingValidationTest, MissingDeclarationAtSameLocation) {
    wgpu::ShaderModule vertexModuleOutputAtLocation0 = utils::CreateShaderModule(device, R"(
            struct A {
                @location(0) vout: f32,
                @builtin(position) pos: vec4f,
            }
            @vertex fn main() -> A {
                var vertexOut: A;
                vertexOut.pos = vec4f(0.0, 0.0, 0.0, 1.0);
                return vertexOut;
            })");
    wgpu::ShaderModule fragmentModuleAtLocation0 = utils::CreateShaderModule(device, R"(
            struct B {
                @location(0) fin: f32
            }
            @fragment fn main(fragmentIn: B) -> @location(0) vec4f  {
                return vec4f(fragmentIn.fin, 0.0, 0.0, 1.0);
            })");
    wgpu::ShaderModule fragmentModuleInputAtLocation1 = utils::CreateShaderModule(device, R"(
            struct A {
                @location(1) vout: f32
            }
            @fragment fn main(vertexOut: A) -> @location(0) vec4f  {
                return vec4f(vertexOut.vout, 0.0, 0.0, 1.0);
            })");
    wgpu::ShaderModule vertexModuleOutputAtLocation1 = utils::CreateShaderModule(device, R"(
            struct B {
                @location(1) fin: f32,
                @builtin(position) pos: vec4f,
            }
            @vertex fn main() -> B {
                var fragmentIn: B;
                fragmentIn.pos = vec4f(0.0, 0.0, 0.0, 1.0);
                return fragmentIn;
            })");

    {
        // It is okay if the fragment output is a subset of the vertex input.
        CheckCreatingRenderPipeline(vertexModuleOutputAtLocation0, fsModule, true);
    }
    {
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
    constexpr std::array<const char*, 16> kTypes = {
        {"f32", "vec2f", "vec3f", "vec4f", "f16", "vec2<f16>", "vec3<f16>", "vec4<f16>", "i32",
         "vec2i", "vec3i", "vec4i", "u32", "vec2u", "vec3u", "vec4u"}};

    std::array<wgpu::ShaderModule, 16> vertexModules;
    std::array<wgpu::ShaderModule, 16> fragmentModules;
    for (uint32_t i = 0; i < kTypes.size(); ++i) {
        std::string interfaceDeclaration;
        {
            std::ostringstream sstream;
            sstream << "struct A { @location(0) @interpolate(flat) a: " << kTypes[i] << ","
                    << std::endl;
            interfaceDeclaration = sstream.str();
        }

        std::string extensionDeclaration = "enable f16;\n\n";

        {
            std::ostringstream vertexStream;
            vertexStream << extensionDeclaration << interfaceDeclaration << R"(
                    @builtin(position) pos: vec4f,
                }
                @vertex fn main() -> A {
                    var vertexOut: A;
                    vertexOut.pos = vec4f(0.0, 0.0, 0.0, 1.0);
                    return vertexOut;
                })";
            vertexModules[i] = utils::CreateShaderModule(device, vertexStream.str().c_str());
        }
        {
            std::ostringstream fragmentStream;
            fragmentStream << extensionDeclaration << interfaceDeclaration << R"(
                }
                @fragment fn main(fragmentIn: A) -> @location(0) vec4f {
                    return vec4f(0.0, 0.0, 0.0, 1.0);
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
            sstream << " a : vec4f," << std::endl;
            interfaceDeclaration = sstream.str();
        }
        {
            std::ostringstream vertexStream;
            vertexStream << interfaceDeclaration << R"(
                    @builtin(position) pos: vec4f,
                }
                @vertex fn main() -> A {
                    var vertexOut: A;
                    vertexOut.pos = vec4f(0.0, 0.0, 0.0, 1.0);
                    return vertexOut;
                })";
            vertexModules[i] = utils::CreateShaderModule(device, vertexStream.str().c_str());
        }
        {
            std::ostringstream fragmentStream;
            fragmentStream << interfaceDeclaration << R"(
                }
                @fragment fn main(fragmentIn: A) -> @location(0) vec4f {
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
