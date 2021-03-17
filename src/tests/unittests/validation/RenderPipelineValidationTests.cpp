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

#include "tests/unittests/validation/ValidationTest.h"

#include "common/Constants.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

#include <cmath>
#include <sstream>

class RenderPipelineValidationTest : public ValidationTest {
  protected:
    void SetUp() override {
        ValidationTest::SetUp();

        vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[builtin(position)]] var<out> Position : vec4<f32>;
            [[stage(vertex)]] fn main() -> void {
                Position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
            })");

        fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
            [[location(0)]] var<out> fragColor : vec4<f32>;
            [[stage(fragment)]] fn main() -> void {
                fragColor = vec4<f32>(0.0, 1.0, 0.0, 1.0);
            })");
    }

    wgpu::ShaderModule vsModule;
    wgpu::ShaderModule fsModule;
};

// Test cases where creation should succeed
TEST_F(RenderPipelineValidationTest, CreationSuccess) {
    {
        // New format
        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;

        device.CreateRenderPipeline2(&descriptor);
    }
    {
        // Deprecated format
        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;

        device.CreateRenderPipeline(&descriptor);
    }
    {
        // Vertex input should be optional
        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;
        descriptor.vertexState = nullptr;

        device.CreateRenderPipeline(&descriptor);
    }
    {
        // Rasterization state should be optional
        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;
        descriptor.rasterizationState = nullptr;
        device.CreateRenderPipeline(&descriptor);
    }
}

// Tests that depth bias parameters must not be NaN.
TEST_F(RenderPipelineValidationTest, DepthBiasParameterNotBeNaN) {
    // Control case, depth bias parameters in ComboRenderPipeline default to 0 which is finite
    {
        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.EnableDepthStencil();
        device.CreateRenderPipeline2(&descriptor);
    }

    // Infinite depth bias clamp is valid
    {
        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();
        depthStencil->depthBiasClamp = INFINITY;
        device.CreateRenderPipeline2(&descriptor);
    }
    // NAN depth bias clamp is invalid
    {
        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();
        depthStencil->depthBiasClamp = NAN;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
    }

    // Infinite depth bias slope is valid
    {
        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();
        depthStencil->depthBiasSlopeScale = INFINITY;
        device.CreateRenderPipeline2(&descriptor);
    }
    // NAN depth bias slope is invalid
    {
        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();
        depthStencil->depthBiasSlopeScale = NAN;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
    }
}

// Tests that at least one color target state is required.
TEST_F(RenderPipelineValidationTest, ColorTargetStateRequired) {
    {
        // This one succeeds because attachment 0 is the color attachment
        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cFragment.targetCount = 1;

        device.CreateRenderPipeline2(&descriptor);
    }

    {  // Fail because lack of color target states (and depth/stencil state)
        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cFragment.targetCount = 0;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
    }
}

// Tests that the color formats must be renderable.
TEST_F(RenderPipelineValidationTest, NonRenderableFormat) {
    {
        // Succeeds because RGBA8Unorm is renderable
        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        device.CreateRenderPipeline2(&descriptor);
    }

    {
        // Fails because RG11B10Ufloat is non-renderable
        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RG11B10Ufloat;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
    }
}

// Tests that the format of the color state descriptor must match the output of the fragment shader.
TEST_F(RenderPipelineValidationTest, FragmentOutputFormatCompatibility) {
    constexpr uint32_t kNumTextureFormatBaseType = 3u;
    std::array<const char*, kNumTextureFormatBaseType> kScalarTypes = {{"f32", "i32", "u32"}};
    std::array<wgpu::TextureFormat, kNumTextureFormatBaseType> kColorFormats = {
        {wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Sint,
         wgpu::TextureFormat::RGBA8Uint}};

    for (size_t i = 0; i < kNumTextureFormatBaseType; ++i) {
        for (size_t j = 0; j < kNumTextureFormatBaseType; ++j) {
            utils::ComboRenderPipelineDescriptor2 descriptor;
            descriptor.vertex.module = vsModule;
            descriptor.cTargets[0].format = kColorFormats[j];

            std::ostringstream stream;
            stream << R"(
                [[location(0)]] var<out> fragColor : vec4<)"
                   << kScalarTypes[i] << R"(>;
                [[stage(fragment)]] fn main() -> void {
                })";
            descriptor.cFragment.module =
                utils::CreateShaderModuleFromWGSL(device, stream.str().c_str());

            if (i == j) {
                device.CreateRenderPipeline2(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
            }
        }
    }
}

/// Tests that the sample count of the render pipeline must be valid.
TEST_F(RenderPipelineValidationTest, SampleCount) {
    {
        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 4;

        device.CreateRenderPipeline2(&descriptor);
    }

    {
        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 3;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
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
    baseTextureDescriptor.size.depth = 1;
    baseTextureDescriptor.mipLevelCount = 1;
    baseTextureDescriptor.dimension = wgpu::TextureDimension::e2D;
    baseTextureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;

    utils::ComboRenderPipelineDescriptor2 nonMultisampledPipelineDescriptor;
    nonMultisampledPipelineDescriptor.multisample.count = 1;
    nonMultisampledPipelineDescriptor.vertex.module = vsModule;
    nonMultisampledPipelineDescriptor.cFragment.module = fsModule;
    wgpu::RenderPipeline nonMultisampledPipeline =
        device.CreateRenderPipeline2(&nonMultisampledPipelineDescriptor);

    nonMultisampledPipelineDescriptor.cFragment.targetCount = 0;
    nonMultisampledPipelineDescriptor.EnableDepthStencil();
    wgpu::RenderPipeline nonMultisampledPipelineWithDepthStencilOnly =
        device.CreateRenderPipeline2(&nonMultisampledPipelineDescriptor);

    utils::ComboRenderPipelineDescriptor2 multisampledPipelineDescriptor;
    multisampledPipelineDescriptor.multisample.count = kMultisampledCount;
    multisampledPipelineDescriptor.vertex.module = vsModule;
    multisampledPipelineDescriptor.cFragment.module = fsModule;
    wgpu::RenderPipeline multisampledPipeline =
        device.CreateRenderPipeline2(&multisampledPipelineDescriptor);

    multisampledPipelineDescriptor.cFragment.targetCount = 0;
    multisampledPipelineDescriptor.EnableDepthStencil();
    wgpu::RenderPipeline multisampledPipelineWithDepthStencilOnly =
        device.CreateRenderPipeline2(&multisampledPipelineDescriptor);

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
        renderPass.EndPass();

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
        renderPass.EndPass();

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
        renderPass.EndPass();

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
        renderPass.EndPass();

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
        renderPass.EndPass();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::TextureDescriptor textureDescriptor = baseTextureDescriptor;
        textureDescriptor.sampleCount = 1;
        textureDescriptor.format = kDepthStencilFormat;
        wgpu::Texture multisampledDepthStencilTexture = device.CreateTexture(&textureDescriptor);
        utils::ComboRenderPassDescriptor renderPassDescriptor(
            {}, multisampledDepthStencilTexture.CreateView());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(multisampledPipelineWithDepthStencilOnly);
        renderPass.EndPass();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Tests that the sample count of the render pipeline must be valid
// when the alphaToCoverage mode is enabled.
TEST_F(RenderPipelineValidationTest, AlphaToCoverageAndSampleCount) {
    {
        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;

        device.CreateRenderPipeline2(&descriptor);
    }

    {
        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 1;
        descriptor.multisample.alphaToCoverageEnabled = true;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
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
            utils::ComboRenderPipelineDescriptor2 descriptor;
            descriptor.vertex.module = vsModule;

            std::ostringstream stream;
            stream << R"(
                [[group(0), binding(0)]] var myTexture : texture_2d<)"
                   << kScalarTypes[i] << R"(>;

                [[stage(fragment)]] fn main() -> void {
                })";
            descriptor.cFragment.module =
                utils::CreateShaderModuleFromWGSL(device, stream.str().c_str());

            wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Fragment, kTextureComponentTypes[j]}});
            descriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);

            if (i == j) {
                device.CreateRenderPipeline2(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
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
            utils::ComboRenderPipelineDescriptor2 descriptor;
            descriptor.vertex.module = vsModule;

            std::ostringstream stream;
            stream << R"(
                [[group(0), binding(0)]] var myTexture : )"
                   << kTextureKeywords[i] << R"(<f32>;
                [[stage(fragment)]] fn main() -> void {
                })";
            descriptor.cFragment.module =
                utils::CreateShaderModuleFromWGSL(device, stream.str().c_str());

            wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float,
                          kTextureViewDimensions[j]}});
            descriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);

            if (i == j) {
                device.CreateRenderPipeline2(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
            }
        }
    }
}

// Test that declaring a storage buffer in the vertex shader without setting pipeline layout won't
// cause crash.
TEST_F(RenderPipelineValidationTest, StorageBufferInVertexShaderNoLayout) {
    wgpu::ShaderModule vsModuleWithStorageBuffer = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct Dst {
            data : array<u32, 100>;
        };
        [[group(0), binding(0)]] var<storage_buffer> dst : [[access(read_write)]] Dst;
        [[builtin(vertex_index)]] var<in> VertexIndex : u32;
        [[stage(vertex)]] fn main() -> void {
            dst.data[VertexIndex] = 0x1234u;
        })");

    utils::ComboRenderPipelineDescriptor2 descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModuleWithStorageBuffer;
    descriptor.cFragment.module = fsModule;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
}

// Tests that strip primitive topologies require an index format
TEST_F(RenderPipelineValidationTest, StripIndexFormatRequired) {
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
            utils::ComboRenderPipelineDescriptor2 descriptor;
            descriptor.vertex.module = vsModule;
            descriptor.cFragment.module = fsModule;
            descriptor.primitive.topology = primitiveTopology;
            descriptor.primitive.stripIndexFormat = indexFormat;

            if (indexFormat == wgpu::IndexFormat::Undefined) {
                // Fail because the index format is undefined and the primitive
                // topology is a strip type.
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
            } else {
                // Succeeds because the index format is given.
                device.CreateRenderPipeline2(&descriptor);
            }
        }
    }

    for (wgpu::PrimitiveTopology primitiveTopology : kListTopologyTypes) {
        for (wgpu::IndexFormat indexFormat : kIndexFormatTypes) {
            utils::ComboRenderPipelineDescriptor2 descriptor;
            descriptor.vertex.module = vsModule;
            descriptor.cFragment.module = fsModule;
            descriptor.primitive.topology = primitiveTopology;
            descriptor.primitive.stripIndexFormat = indexFormat;

            if (indexFormat == wgpu::IndexFormat::Undefined) {
                // Succeeds even when the index format is undefined because the
                // primitive topology isn't a strip type.
                device.CreateRenderPipeline2(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
            }
        }
    }
}

// Test that the entryPoint names must be present for the correct stage in the shader module.
TEST_F(RenderPipelineValidationTest, EntryPointNameValidation) {
    DAWN_SKIP_TEST_IF(!HasWGSL());

    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[builtin(position)]] var<out> position : vec4<f32>;
        [[stage(vertex)]] fn vertex_main() -> void {
            position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
            return;
        }

        [[location(0)]] var<out> color : vec4<f32>;
        [[stage(fragment)]] fn fragment_main() -> void {
            color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
            return;
        }
    )");

    utils::ComboRenderPipelineDescriptor2 descriptor;
    descriptor.vertex.module = module;
    descriptor.vertex.entryPoint = "vertex_main";
    descriptor.cFragment.module = module;
    descriptor.cFragment.entryPoint = "fragment_main";

    // Success case.
    device.CreateRenderPipeline2(&descriptor);

    // Test for the vertex stage entryPoint name.
    {
        // The entryPoint name doesn't exist in the module.
        descriptor.vertex.entryPoint = "main";
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));

        // The entryPoint name exists, but not for the correct stage.
        descriptor.vertex.entryPoint = "fragment_main";
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
    }

    descriptor.vertex.entryPoint = "vertex_main";

    // Test for the fragment stage entryPoint name.
    {
        // The entryPoint name doesn't exist in the module.
        descriptor.cFragment.entryPoint = "main";
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));

        // The entryPoint name exists, but not for the correct stage.
        descriptor.cFragment.entryPoint = "vertex_main";
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
    }
}

// Test that vertex attrib validation is for the correct entryPoint
TEST_F(RenderPipelineValidationTest, VertexAttribCorrectEntryPoint) {
    DAWN_SKIP_TEST_IF(!HasWGSL());

    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[builtin(position)]] var<out> position : vec4<f32>;
        [[location(0)]] var<in> attrib0 : vec4<f32>;
        [[location(1)]] var<in> attrib1 : vec4<f32>;

        [[stage(vertex)]] fn vertex0() -> void {
            position = attrib0;
            return;
        }
        [[stage(vertex)]] fn vertex1() -> void {
            position = attrib1;
            return;
        }
    )");

    utils::ComboRenderPipelineDescriptor2 descriptor;
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
    device.CreateRenderPipeline2(&descriptor);

    descriptor.vertex.entryPoint = "vertex1";
    descriptor.cAttributes[0].shaderLocation = 1;
    device.CreateRenderPipeline2(&descriptor);

    // Error cases, the attribute used by the entryPoint isn't declared in the pipeline.
    descriptor.vertex.entryPoint = "vertex1";
    descriptor.cAttributes[0].shaderLocation = 0;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));

    descriptor.vertex.entryPoint = "vertex0";
    descriptor.cAttributes[0].shaderLocation = 1;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
}

// Test that fragment output validation is for the correct entryPoint
TEST_F(RenderPipelineValidationTest, FragmentOutputCorrectEntryPoint) {
    DAWN_SKIP_TEST_IF(!HasWGSL());

    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[location(0)]] var<out> colorFloat : vec4<f32>;
        [[location(0)]] var<out> colorUint : vec4<u32>;

        [[stage(fragment)]] fn fragmentFloat() -> void {
            colorFloat = vec4<f32>(0.0, 0.0, 0.0, 0.0);
            return;
        }
        [[stage(fragment)]] fn fragmentUint() -> void {
            colorUint = vec4<u32>(0u, 0u, 0u, 0u);
            return;
        }
    )");

    utils::ComboRenderPipelineDescriptor2 descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = module;

    // Success case, the component type matches between the pipeline and the entryPoint
    descriptor.cFragment.entryPoint = "fragmentFloat";
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA32Float;
    device.CreateRenderPipeline2(&descriptor);

    descriptor.cFragment.entryPoint = "fragmentUint";
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA32Uint;
    device.CreateRenderPipeline2(&descriptor);

    // Error case, the component type doesn't match between the pipeline and the entryPoint
    descriptor.cFragment.entryPoint = "fragmentUint";
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA32Float;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));

    descriptor.cFragment.entryPoint = "fragmentFloat";
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA32Uint;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
}

// Test that fragment output validation is for the correct entryPoint
// TODO(dawn:216): Re-enable when we correctly reflect which bindings are used for an entryPoint.
TEST_F(RenderPipelineValidationTest, DISABLED_BindingsFromCorrectEntryPoint) {
    DAWN_SKIP_TEST_IF(!HasWGSL());

    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct Uniforms {
            data : vec4<f32>;
        };
        [[binding 0, set 0]] var<uniform> var0 : Uniforms;
        [[binding 1, set 0]] var<uniform> var1 : Uniforms;
        [[builtin(position)]] var<out> position : vec4<f32>;

        fn vertex0() -> void {
            position = var0.data;
            return;
        }
        fn vertex1() -> void {
            position = var1.data;
            return;
        }

        entry_point vertex = vertex0;
        entry_point vertex = vertex1;
    )");

    wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}});
    wgpu::PipelineLayout layout0 = utils::MakeBasicPipelineLayout(device, &bgl0);

    wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}});
    wgpu::PipelineLayout layout1 = utils::MakeBasicPipelineLayout(device, &bgl1);

    utils::ComboRenderPipelineDescriptor2 descriptor;
    descriptor.vertex.module = module;
    descriptor.cFragment.module = fsModule;

    // Success case, the BGL matches the bindings used by the entryPoint
    descriptor.vertex.entryPoint = "vertex0";
    descriptor.layout = layout0;
    device.CreateRenderPipeline2(&descriptor);

    descriptor.vertex.entryPoint = "vertex1";
    descriptor.layout = layout1;
    device.CreateRenderPipeline2(&descriptor);

    // Error case, the BGL doesn't match the bindings used by the entryPoint
    descriptor.vertex.entryPoint = "vertex1";
    descriptor.layout = layout0;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));

    descriptor.vertex.entryPoint = "vertex0";
    descriptor.layout = layout1;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline2(&descriptor));
}
