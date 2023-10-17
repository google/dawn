// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/tests/end2end/SharedTextureMemoryTests.h"

#include "dawn/tests/MockCallback.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {

void SharedTextureMemoryNoFeatureTests::SetUp() {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    DawnTestWithParams<SharedTextureMemoryTestParams>::SetUp();
}

std::vector<wgpu::FeatureName> SharedTextureMemoryTests::GetRequiredFeatures() {
    auto features = GetParam().mBackend->RequiredFeatures();
    if (!SupportsFeatures(features)) {
        return {};
    }
    if (SupportsFeatures({wgpu::FeatureName::TransientAttachments})) {
        features.push_back(wgpu::FeatureName::TransientAttachments);
    }
    return features;
}

void SharedTextureMemoryTests::SetUp() {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    DawnTestWithParams<SharedTextureMemoryTestParams>::SetUp();
    DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures(GetParam().mBackend->RequiredFeatures()));
}

std::vector<wgpu::SharedTextureMemory> SharedTextureMemoryTestBackend::CreateSharedTextureMemories(
    wgpu::Device& device) {
    std::vector<wgpu::SharedTextureMemory> memories;
    for (auto& memory : CreatePerDeviceSharedTextureMemories({device})) {
        DAWN_ASSERT(memory.size() == 1u);
        memories.push_back(std::move(memory[0]));
    }
    return memories;
}

std::vector<std::vector<wgpu::SharedTextureMemory>>
SharedTextureMemoryTestBackend::CreatePerDeviceSharedTextureMemoriesFilterByUsage(
    const std::vector<wgpu::Device>& devices,
    wgpu::TextureUsage requiredUsage) {
    std::vector<std::vector<wgpu::SharedTextureMemory>> out;
    for (auto& memories : CreatePerDeviceSharedTextureMemories(devices)) {
        wgpu::SharedTextureMemoryProperties properties;
        memories[0].GetProperties(&properties);

        if ((properties.usage & requiredUsage) == requiredUsage) {
            out.push_back(std::move(memories));
        }
    }
    return out;
}

wgpu::Device SharedTextureMemoryTests::CreateDevice() {
    if (GetParam().mBackend->UseSameDevice()) {
        return device;
    }
    return DawnTestBase::CreateDevice();
}

void SharedTextureMemoryTests::UseInRenderPass(wgpu::Device& deviceObj, wgpu::Texture& texture) {
    wgpu::CommandEncoder encoder = deviceObj.CreateCommandEncoder();
    utils::ComboRenderPassDescriptor passDescriptor({texture.CreateView()});
    passDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Load;
    passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
    pass.End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    deviceObj.GetQueue().Submit(1, &commandBuffer);
}

void SharedTextureMemoryTests::UseInCopy(wgpu::Device& deviceObj, wgpu::Texture& texture) {
    wgpu::CommandEncoder encoder = deviceObj.CreateCommandEncoder();
    wgpu::ImageCopyTexture source;
    source.texture = texture;

    // Create a destination buffer, large enough for 1 texel of any format.
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 128;
    bufferDesc.usage = wgpu::BufferUsage::CopyDst;

    wgpu::ImageCopyBuffer destination;
    destination.buffer = deviceObj.CreateBuffer(&bufferDesc);

    wgpu::Extent3D size = {1, 1, 1};
    encoder.CopyTextureToBuffer(&source, &destination, &size);

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    deviceObj.GetQueue().Submit(1, &commandBuffer);
}

// Make a command buffer that clears the texture to four different colors in each quadrant.
wgpu::CommandBuffer SharedTextureMemoryTests::MakeFourColorsClearCommandBuffer(
    wgpu::Device& deviceObj,
    wgpu::Texture& texture) {
    wgpu::ShaderModule module = utils::CreateShaderModule(deviceObj, R"(
      struct VertexOut {
          @builtin(position) position : vec4f,
          @location(0) uv : vec2f,
      }

      struct FragmentIn {
          @location(0) uv : vec2f,
      }

      @vertex fn vert_main(@builtin(vertex_index) VertexIndex : u32) -> VertexOut {
          let pos = array(
            vec2( 1.0,  1.0),
            vec2( 1.0, -1.0),
            vec2(-1.0, -1.0),
            vec2( 1.0,  1.0),
            vec2(-1.0, -1.0),
            vec2(-1.0,  1.0),
          );

          let uv = array(
            vec2(1.0, 0.0),
            vec2(1.0, 1.0),
            vec2(0.0, 1.0),
            vec2(1.0, 0.0),
            vec2(0.0, 1.0),
            vec2(0.0, 0.0),
          );
          return VertexOut(vec4f(pos[VertexIndex], 0.0, 1.0), uv[VertexIndex]);
      }

      @fragment fn frag_main(in: FragmentIn) -> @location(0) vec4f {
          if (in.uv.x < 0.5) {
            if (in.uv.y < 0.5) {
              return vec4f(0.0, 1.0, 0.0, 1.0);
            } else {
              return vec4f(1.0, 0.0, 0.0, 1.0);
            }
          } else {
            if (in.uv.y < 0.5) {
              return vec4f(0.0, 0.0, 1.0, 1.0);
            } else {
              return vec4f(1.0, 1.0, 0.0, 1.0);
            }
          }
      }
    )");

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = module;
    pipelineDesc.vertex.entryPoint = "vert_main";
    pipelineDesc.cFragment.module = module;
    pipelineDesc.cFragment.entryPoint = "frag_main";
    pipelineDesc.cTargets[0].format = texture.GetFormat();

    wgpu::RenderPipeline pipeline = deviceObj.CreateRenderPipeline(&pipelineDesc);

    wgpu::CommandEncoder encoder = deviceObj.CreateCommandEncoder();
    utils::ComboRenderPassDescriptor passDescriptor({texture.CreateView()});
    passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
    pass.SetPipeline(pipeline);
    pass.Draw(6);
    pass.End();
    return encoder.Finish();
}

// Make a command buffer that samples the contents of the input texture into an RGBA8Unorm texture.
std::pair<wgpu::CommandBuffer, wgpu::Texture>
SharedTextureMemoryTests::MakeCheckBySamplingCommandBuffer(wgpu::Device& deviceObj,
                                                           wgpu::Texture& texture) {
    wgpu::ShaderModule module = utils::CreateShaderModule(deviceObj, R"(
      @vertex fn vert_main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
          let pos = array(
            vec2( 1.0,  1.0),
            vec2( 1.0, -1.0),
            vec2(-1.0, -1.0),
            vec2( 1.0,  1.0),
            vec2(-1.0, -1.0),
            vec2(-1.0,  1.0),
          );
          return vec4f(pos[VertexIndex], 0.0, 1.0);
      }

      @group(0) @binding(0) var t: texture_2d<f32>;

      @fragment fn frag_main(@builtin(position) coord_in: vec4<f32>) -> @location(0) vec4f {
        return textureLoad(t, vec2u(coord_in.xy), 0);
      }
    )");

    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    textureDesc.size = {texture.GetWidth(), texture.GetHeight(), texture.GetDepthOrArrayLayers()};
    textureDesc.label = "intermediate check texture";

    wgpu::Texture colorTarget = deviceObj.CreateTexture(&textureDesc);

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = module;
    pipelineDesc.vertex.entryPoint = "vert_main";
    pipelineDesc.cFragment.module = module;
    pipelineDesc.cFragment.entryPoint = "frag_main";
    pipelineDesc.cTargets[0].format = colorTarget.GetFormat();

    wgpu::RenderPipeline pipeline = deviceObj.CreateRenderPipeline(&pipelineDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(deviceObj, pipeline.GetBindGroupLayout(0),
                                                     {{0, texture.CreateView()}});

    wgpu::CommandEncoder encoder = deviceObj.CreateCommandEncoder();
    utils::ComboRenderPassDescriptor passDescriptor({colorTarget.CreateView()});
    passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(6);
    pass.End();
    return {encoder.Finish(), colorTarget};
}

// Check that the contents of colorTarget are RGBA8Unorm texels that match those written by
// MakeFourColorsClearCommandBuffer.
void SharedTextureMemoryTests::CheckFourColors(wgpu::Device& deviceObj,
                                               wgpu::TextureFormat format,
                                               wgpu::Texture& colorTarget) {
    wgpu::Origin3D tl = {colorTarget.GetWidth() / 4, colorTarget.GetHeight() / 4};
    wgpu::Origin3D bl = {colorTarget.GetWidth() / 4, 3 * colorTarget.GetHeight() / 4};
    wgpu::Origin3D tr = {3 * colorTarget.GetWidth() / 4, colorTarget.GetHeight() / 4};
    wgpu::Origin3D br = {3 * colorTarget.GetWidth() / 4, 3 * colorTarget.GetHeight() / 4};

    switch (format) {
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::RGB10A2Unorm:
        case wgpu::TextureFormat::RGBA16Float:
            EXPECT_TEXTURE_EQ(deviceObj, &utils::RGBA8::kGreen, colorTarget, tl, {1, 1});
            EXPECT_TEXTURE_EQ(deviceObj, &utils::RGBA8::kRed, colorTarget, bl, {1, 1});
            EXPECT_TEXTURE_EQ(deviceObj, &utils::RGBA8::kBlue, colorTarget, tr, {1, 1});
            EXPECT_TEXTURE_EQ(deviceObj, &utils::RGBA8::kYellow, colorTarget, br, {1, 1});
            break;
        case wgpu::TextureFormat::RG16Float:
        case wgpu::TextureFormat::RG8Unorm:
            EXPECT_TEXTURE_EQ(deviceObj, &utils::RGBA8::kGreen, colorTarget, tl, {1, 1});
            EXPECT_TEXTURE_EQ(deviceObj, &utils::RGBA8::kRed, colorTarget, bl, {1, 1});
            EXPECT_TEXTURE_EQ(deviceObj, &utils::RGBA8::kBlack, colorTarget, tr, {1, 1});
            EXPECT_TEXTURE_EQ(deviceObj, &utils::RGBA8::kYellow, colorTarget, br, {1, 1});
            break;
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::R8Unorm:
            EXPECT_TEXTURE_EQ(deviceObj, &utils::RGBA8::kBlack, colorTarget, tl, {1, 1});
            EXPECT_TEXTURE_EQ(deviceObj, &utils::RGBA8::kRed, colorTarget, bl, {1, 1});
            EXPECT_TEXTURE_EQ(deviceObj, &utils::RGBA8::kBlack, colorTarget, tr, {1, 1});
            EXPECT_TEXTURE_EQ(deviceObj, &utils::RGBA8::kRed, colorTarget, br, {1, 1});
            break;
        default:
            DAWN_UNREACHABLE();
    }
}

// Allow tests to be uninstantiated since it's possible no backends are available.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SharedTextureMemoryNoFeatureTests);
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SharedTextureMemoryTests);

namespace {

using testing::HasSubstr;
using testing::MockCallback;

template <typename T>
T& AsNonConst(const T& rhs) {
    return const_cast<T&>(rhs);
}

// Test that creating shared texture memory without the required features is an error.
// Using the memory thereafter produces errors.
TEST_P(SharedTextureMemoryNoFeatureTests, CreationWithoutFeature) {
    // Create external texture memories with an error filter.
    // We should see a message that the feature is not enabled.
    device.PushErrorScope(wgpu::ErrorFilter::Validation);
    const auto& memories = GetParam().mBackend->CreateSharedTextureMemories(device);

    MockCallback<WGPUErrorCallback> popErrorScopeCallback;
    EXPECT_CALL(popErrorScopeCallback,
                Call(WGPUErrorType_Validation, HasSubstr("is not enabled"), this));

    device.PopErrorScope(popErrorScopeCallback.Callback(),
                         popErrorScopeCallback.MakeUserdata(this));

    for (wgpu::SharedTextureMemory memory : memories) {
        ASSERT_DEVICE_ERROR_MSG(wgpu::Texture texture = memory.CreateTexture(),
                                HasSubstr("is invalid"));

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.initialized = true;

        ASSERT_DEVICE_ERROR_MSG(EXPECT_TRUE(memory.BeginAccess(texture, &beginDesc)),
                                HasSubstr("is invalid"));

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        ASSERT_DEVICE_ERROR_MSG(EXPECT_TRUE(memory.EndAccess(texture, &endState)),
                                HasSubstr("is invalid"));
    }
}

// Test that it is an error to import a shared texture memory with no chained struct.
TEST_P(SharedTextureMemoryTests, ImportSharedTextureMemoryNoChain) {
    wgpu::SharedTextureMemoryDescriptor desc;
    ASSERT_DEVICE_ERROR_MSG(
        wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc),
        HasSubstr("chain"));
}

// Test that it is an error to import a shared fence with no chained struct.
// Also test that ExportInfo reports an Undefined type for the error fence.
TEST_P(SharedTextureMemoryTests, ImportSharedFenceNoChain) {
    wgpu::SharedFenceDescriptor desc;
    ASSERT_DEVICE_ERROR_MSG(wgpu::SharedFence fence = device.ImportSharedFence(&desc),
                            HasSubstr("chain"));

    wgpu::SharedFenceExportInfo exportInfo;
    exportInfo.type = static_cast<wgpu::SharedFenceType>(1234);  // should be overrwritten

    // Expect that exporting the fence info writes Undefined, and generates an error.
    ASSERT_DEVICE_ERROR(fence.ExportInfo(&exportInfo));
    EXPECT_EQ(exportInfo.type, wgpu::SharedFenceType::Undefined);
}

// Test that it is an error to import a shared texture memory when the device is destroyed
TEST_P(SharedTextureMemoryTests, ImportSharedTextureMemoryDeviceDestroyed) {
    device.Destroy();

    wgpu::SharedTextureMemoryDescriptor desc;
    ASSERT_DEVICE_ERROR_MSG(
        wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc),
        HasSubstr("lost"));
}

// Test that it is an error to import a shared fence when the device is destroyed
TEST_P(SharedTextureMemoryTests, ImportSharedFenceDeviceDestroyed) {
    device.Destroy();

    wgpu::SharedFenceDescriptor desc;
    ASSERT_DEVICE_ERROR_MSG(wgpu::SharedFence fence = device.ImportSharedFence(&desc),
                            HasSubstr("lost"));
}

// Test calling GetProperties with an error memory. The properties are filled with 0/None/Undefined.
TEST_P(SharedTextureMemoryTests, GetPropertiesErrorMemory) {
    wgpu::SharedTextureMemoryDescriptor desc;
    ASSERT_DEVICE_ERROR(wgpu::SharedTextureMemory memory = device.ImportSharedTextureMemory(&desc));

    wgpu::SharedTextureMemoryProperties properties;
    memory.GetProperties(&properties);

    EXPECT_EQ(properties.usage, wgpu::TextureUsage::None);
    EXPECT_EQ(properties.size.width, 0u);
    EXPECT_EQ(properties.size.height, 0u);
    EXPECT_EQ(properties.size.depthOrArrayLayers, 0u);
    EXPECT_EQ(properties.format, wgpu::TextureFormat::Undefined);
}

// Test calling GetProperties with an invalid chained struct. An error is
// generated, but the properties are still populated.
TEST_P(SharedTextureMemoryTests, GetPropertiesInvalidChain) {
    wgpu::SharedTextureMemory memory = GetParam().mBackend->CreateSharedTextureMemory(device);

    wgpu::ChainedStructOut otherStruct;
    wgpu::SharedTextureMemoryProperties properties1;
    properties1.nextInChain = &otherStruct;
    ASSERT_DEVICE_ERROR(memory.GetProperties(&properties1));

    wgpu::SharedTextureMemoryProperties properties2;
    memory.GetProperties(&properties2);

    EXPECT_EQ(properties1.usage, properties2.usage);
    EXPECT_EQ(properties1.size.width, properties2.size.width);
    EXPECT_EQ(properties1.size.height, properties2.size.height);
    EXPECT_EQ(properties1.size.depthOrArrayLayers, properties2.size.depthOrArrayLayers);
    EXPECT_EQ(properties1.format, properties2.format);
}

// Test that texture usages must be a subset of the shared texture memory's usage.
TEST_P(SharedTextureMemoryTests, UsageValidation) {
    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        // SharedTextureMemory should never support TransientAttachment.
        ASSERT_EQ(properties.usage & wgpu::TextureUsage::TransientAttachment, 0);

        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.format = properties.format;
        textureDesc.size = properties.size;

        for (wgpu::TextureUsage usage : {
                 wgpu::TextureUsage::CopySrc,
                 wgpu::TextureUsage::CopyDst,
                 wgpu::TextureUsage::TextureBinding,
                 wgpu::TextureUsage::StorageBinding,
                 wgpu::TextureUsage::RenderAttachment,
             }) {
            textureDesc.usage = usage;

            // `usage` is valid if it is in the shared texture memory properties.
            if (usage & properties.usage) {
                wgpu::Texture t = memory.CreateTexture(&textureDesc);
                EXPECT_EQ(t.GetUsage(), usage);
            } else {
                ASSERT_DEVICE_ERROR(memory.CreateTexture(&textureDesc));
            }
        }
    }
}

// Test that it is an error if the texture format doesn't match the shared texture memory.
TEST_P(SharedTextureMemoryTests, FormatValidation) {
    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.format = properties.format != wgpu::TextureFormat::RGBA8Unorm
                                 ? wgpu::TextureFormat::RGBA8Unorm
                                 : wgpu::TextureFormat::RGBA16Float;
        textureDesc.size = properties.size;
        textureDesc.usage = properties.usage;

        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc),
                                HasSubstr("doesn't match descriptor format"));
    }
}

// Test that it is an error if the texture size doesn't match the shared texture memory.
TEST_P(SharedTextureMemoryTests, SizeValidation) {
    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.format = properties.format;
        textureDesc.usage = properties.usage;

        textureDesc.size = {properties.size.width + 1, properties.size.height,
                            properties.size.depthOrArrayLayers};
        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc),
                                HasSubstr("doesn't match descriptor size"));

        textureDesc.size = {properties.size.width, properties.size.height + 1,
                            properties.size.depthOrArrayLayers};
        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc),
                                HasSubstr("doesn't match descriptor size"));

        textureDesc.size = {properties.size.width, properties.size.height,
                            properties.size.depthOrArrayLayers + 1};
        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc), HasSubstr("is not 1"));
    }
}

// Test that it is an error if the texture mip level count is not 1.
TEST_P(SharedTextureMemoryTests, MipLevelValidation) {
    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.format = properties.format;
        textureDesc.usage = properties.usage;
        textureDesc.size = properties.size;
        textureDesc.mipLevelCount = 1u;

        memory.CreateTexture(&textureDesc);

        textureDesc.mipLevelCount = 2u;
        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc), HasSubstr("(2) is not 1"));
    }
}

// Test that it is an error if the texture sample count is not 1.
TEST_P(SharedTextureMemoryTests, SampleCountValidation) {
    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.format = properties.format;
        textureDesc.usage = properties.usage;
        textureDesc.size = properties.size;
        textureDesc.sampleCount = 1u;

        memory.CreateTexture(&textureDesc);

        textureDesc.sampleCount = 4u;
        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc), HasSubstr("(4) is not 1"));
    }
}

// Test that it is an error if the texture dimension is not 2D.
TEST_P(SharedTextureMemoryTests, DimensionValidation) {
    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.format = properties.format;
        textureDesc.usage = properties.usage;
        textureDesc.size = properties.size;

        textureDesc.dimension = wgpu::TextureDimension::e1D;
        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc),
                                HasSubstr("is not TextureDimension::e2D"));

        textureDesc.dimension = wgpu::TextureDimension::e3D;
        ASSERT_DEVICE_ERROR_MSG(memory.CreateTexture(&textureDesc),
                                HasSubstr("is not TextureDimension::e2D"));
    }
}

// Test that it is an error to call BeginAccess twice in a row on the same texture and memory.
TEST_P(SharedTextureMemoryTests, DoubleBeginAccess) {
    wgpu::SharedTextureMemory memory = GetParam().mBackend->CreateSharedTextureMemory(device);
    wgpu::Texture texture = memory.CreateTexture();

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.initialized = true;

    // It should be an error to BeginAccess twice in a row.
    EXPECT_TRUE(memory.BeginAccess(texture, &beginDesc));
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.BeginAccess(texture, &beginDesc)),
                            HasSubstr("Cannot begin access with"));
}

// Test that it is an error to call BeginAccess twice in a row on two textures from the same memory.
TEST_P(SharedTextureMemoryTests, DoubleBeginAccessSeparateTextures) {
    wgpu::SharedTextureMemory memory = GetParam().mBackend->CreateSharedTextureMemory(device);
    wgpu::Texture texture1 = memory.CreateTexture();
    wgpu::Texture texture2 = memory.CreateTexture();

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.initialized = true;

    // It should be an error to BeginAccess twice in a row.
    EXPECT_TRUE(memory.BeginAccess(texture1, &beginDesc));
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.BeginAccess(texture2, &beginDesc)),
                            HasSubstr("Cannot begin access with"));
}

// Test that it is an error to call EndAccess twice in a row on the same memory.
TEST_P(SharedTextureMemoryTests, DoubleEndAccess) {
    wgpu::SharedTextureMemory memory = GetParam().mBackend->CreateSharedTextureMemory(device);
    wgpu::Texture texture = memory.CreateTexture();

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.initialized = true;

    EXPECT_TRUE(memory.BeginAccess(texture, &beginDesc));

    wgpu::SharedTextureMemoryEndAccessState endState = {};
    EXPECT_TRUE(memory.EndAccess(texture, &endState));

    // Invalid to end access a second time.
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.EndAccess(texture, &endState)),
                            HasSubstr("Cannot end access"));
}

// Test that it is an error to call EndAccess on a texture that was not the one BeginAccess was
// called on.
TEST_P(SharedTextureMemoryTests, BeginThenEndOnDifferentTexture) {
    wgpu::SharedTextureMemory memory = GetParam().mBackend->CreateSharedTextureMemory(device);
    wgpu::Texture texture1 = memory.CreateTexture();
    wgpu::Texture texture2 = memory.CreateTexture();

    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.initialized = true;

    EXPECT_TRUE(memory.BeginAccess(texture1, &beginDesc));

    wgpu::SharedTextureMemoryEndAccessState endState = {};
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.EndAccess(texture2, &endState)),
                            HasSubstr("Cannot end access"));
}

// Test that it is an error to call EndAccess without a preceding BeginAccess.
TEST_P(SharedTextureMemoryTests, EndAccessWithoutBegin) {
    wgpu::SharedTextureMemory memory = GetParam().mBackend->CreateSharedTextureMemory(device);
    wgpu::Texture texture = memory.CreateTexture();

    wgpu::SharedTextureMemoryEndAccessState endState = {};
    ASSERT_DEVICE_ERROR_MSG(EXPECT_FALSE(memory.EndAccess(texture, &endState)),
                            HasSubstr("Cannot end access"));
}

// Test that it is an error to use the texture on the queue without a preceding BeginAccess.
TEST_P(SharedTextureMemoryTests, UseWithoutBegin) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::SharedTextureMemory memory = GetParam().mBackend->CreateSharedTextureMemory(device);

    wgpu::SharedTextureMemoryProperties properties;
    memory.GetProperties(&properties);

    wgpu::Texture texture = memory.CreateTexture();

    if (properties.usage & wgpu::TextureUsage::RenderAttachment) {
        ASSERT_DEVICE_ERROR_MSG(UseInRenderPass(device, texture),
                                HasSubstr("without current access"));
    } else if (properties.format != wgpu::TextureFormat::R8BG8Biplanar420Unorm) {
        if (properties.usage & wgpu::TextureUsage::CopySrc) {
            ASSERT_DEVICE_ERROR_MSG(UseInCopy(device, texture),
                                    HasSubstr("without current access"));
        }
        if (properties.usage & wgpu::TextureUsage::CopyDst) {
            wgpu::Extent3D writeSize = {1, 1, 1};
            wgpu::ImageCopyTexture dest = {};
            dest.texture = texture;
            wgpu::TextureDataLayout dataLayout = {};
            uint64_t data[2];
            ASSERT_DEVICE_ERROR_MSG(
                device.GetQueue().WriteTexture(&dest, &data, sizeof(data), &dataLayout, &writeSize),
                HasSubstr("without current access"));
        }
    }
}

// Test that it is valid (does not crash) if the memory is dropped while a texture access has begun.
TEST_P(SharedTextureMemoryTests, TextureAccessOutlivesMemory) {
    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.initialized = true;

        // Begin access on a texture, and drop the memory.
        wgpu::Texture texture = memory.CreateTexture();
        memory.BeginAccess(texture, &beginDesc);
        memory = nullptr;

        // Use the texture on the GPU; it should not crash.
        if (properties.usage & wgpu::TextureUsage::RenderAttachment) {
            UseInRenderPass(device, texture);
        } else if (properties.format != wgpu::TextureFormat::R8BG8Biplanar420Unorm) {
            DAWN_ASSERT(properties.usage & wgpu::TextureUsage::CopySrc);
            UseInCopy(device, texture);
        }
    }
}

// Test that if the texture is uninitialized, it is cleared on first use.
TEST_P(SharedTextureMemoryTests, UninitializedTextureIsCleared) {
    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        // Skipped for multiplanar formats because those must be initialized on import.
        // We also need render attachment usage to initially populate the texture.
        if (utils::IsMultiPlanarFormat(properties.format) ||
            (properties.usage & wgpu::TextureUsage::RenderAttachment) == 0) {
            continue;
        }

        wgpu::Texture texture = memory.CreateTexture();

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        wgpu::SharedTextureMemoryEndAccessState endState = {};

        // First fill the texture with data, so we can check that using it uninitialized
        // makes it black.
        {
            wgpu::CommandBuffer commandBuffer = MakeFourColorsClearCommandBuffer(device, texture);

            beginDesc.initialized = true;
            memory.BeginAccess(texture, &beginDesc);
            device.GetQueue().Submit(1, &commandBuffer);
            memory.EndAccess(texture, &endState);
        }

        // Now, BeginAccess on the texture as uninitialized.
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = endState.fences;
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.initialized = false;
        memory.BeginAccess(texture, &beginDesc);

        // Use the texture on the GPU which should lazy clear it.
        if (properties.usage & wgpu::TextureUsage::CopySrc) {
            UseInCopy(device, texture);
        } else {
            DAWN_ASSERT(properties.usage & wgpu::TextureUsage::RenderAttachment);
            UseInRenderPass(device, texture);
        }

        AsNonConst(endState.initialized) = false;  // should be overrwritten
        memory.EndAccess(texture, &endState);
        // The texture should be initialized now.
        EXPECT_TRUE(endState.initialized);

        // Begin access again - and check that the texture contents are zero.
        {
            auto [commandBuffer, colorTarget] = MakeCheckBySamplingCommandBuffer(device, texture);

            beginDesc.fenceCount = endState.fenceCount;
            beginDesc.fences = endState.fences;
            beginDesc.signaledValues = endState.signaledValues;
            beginDesc.initialized = endState.initialized;

            memory.BeginAccess(texture, &beginDesc);
            device.GetQueue().Submit(1, &commandBuffer);
            memory.EndAccess(texture, &endState);

            uint8_t alphaVal;
            switch (properties.format) {
                case wgpu::TextureFormat::RGBA8Unorm:
                case wgpu::TextureFormat::BGRA8Unorm:
                case wgpu::TextureFormat::RGB10A2Unorm:
                case wgpu::TextureFormat::RGBA16Float:
                    alphaVal = 0;
                    break;
                default:
                    // The test checks by sampling. Formats that don't
                    // have alpha return 1 for alpha when sampled in a shader.
                    alphaVal = 255;
                    break;
            }
            std::vector<utils::RGBA8> expected(texture.GetWidth() * texture.GetHeight(),
                                               utils::RGBA8{0, 0, 0, alphaVal});
            EXPECT_TEXTURE_EQ(device, expected.data(), colorTarget, {0, 0},
                              {colorTarget.GetWidth(), colorTarget.GetHeight()})
                << "format: " << static_cast<uint32_t>(properties.format);
        }
    }
}

// Test that if the texture is uninitialized, EndAccess writes the state out as uninitialized.
TEST_P(SharedTextureMemoryTests, UninitializedOnEndAccess) {
    for (wgpu::SharedTextureMemory memory :
         GetParam().mBackend->CreateSharedTextureMemories(device)) {
        wgpu::SharedTextureMemoryProperties properties;
        memory.GetProperties(&properties);

        // Test basic begin+end access exports the state as uninitialized
        // if it starts as uninitialized. Skipped for multiplanar formats
        // because those must be initialized on import.
        if (!utils::IsMultiPlanarFormat(properties.format)) {
            wgpu::Texture texture = memory.CreateTexture();

            wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
            beginDesc.initialized = false;
            memory.BeginAccess(texture, &beginDesc);

            wgpu::SharedTextureMemoryEndAccessState endState = {};
            AsNonConst(endState.initialized) = true;  // should be overrwritten
            memory.EndAccess(texture, &endState);
            EXPECT_FALSE(endState.initialized);
        }

        // Test begin access as initialized, then uninitializing the texture
        // exports the state as uninitialized on end access. Requires render
        // attachment usage to uninitialize.
        if (properties.usage & wgpu::TextureUsage::RenderAttachment) {
            wgpu::Texture texture = memory.CreateTexture();

            wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
            beginDesc.initialized = true;
            memory.BeginAccess(texture, &beginDesc);

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            utils::ComboRenderPassDescriptor passDescriptor({texture.CreateView()});
            passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;

            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
            pass.End();
            wgpu::CommandBuffer commandBuffer = encoder.Finish();
            device.GetQueue().Submit(1, &commandBuffer);

            wgpu::SharedTextureMemoryEndAccessState endState = {};
            AsNonConst(endState.initialized) = true;  // should be overrwritten
            memory.EndAccess(texture, &endState);
            EXPECT_FALSE(endState.initialized);
        }
    }
}

// Test rendering to a texture memory on one device, then sampling it using another device.
// Encode the commands after performing BeginAccess.
TEST_P(SharedTextureMemoryTests, RenderThenSampleEncodeAfterBeginAccess) {
    std::vector<wgpu::Device> devices = {device, CreateDevice()};

    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding)) {
        wgpu::Texture texture = memories[0].CreateTexture();

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.initialized = false;
        memories[0].BeginAccess(texture, &beginDesc);

        // Clear the texture
        wgpu::CommandBuffer commandBuffer = MakeFourColorsClearCommandBuffer(devices[0], texture);
        devices[0].GetQueue().Submit(1, &commandBuffer);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        memories[0].EndAccess(texture, &endState);

        // Sample from the texture

        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.initialized = endState.initialized;

        texture = memories[1].CreateTexture();

        memories[1].BeginAccess(texture, &beginDesc);

        wgpu::Texture colorTarget;
        std::tie(commandBuffer, colorTarget) =
            MakeCheckBySamplingCommandBuffer(devices[1], texture);
        devices[1].GetQueue().Submit(1, &commandBuffer);
        memories[1].EndAccess(texture, &endState);

        CheckFourColors(devices[1], texture.GetFormat(), colorTarget);
    }
}

// Test rendering to a texture memory on one device, then sampling it using another device.
// Encode the commands before performing BeginAccess (the access is only held during) QueueSubmit.
TEST_P(SharedTextureMemoryTests, RenderThenSampleEncodeBeforeBeginAccess) {
    std::vector<wgpu::Device> devices = {device, CreateDevice()};
    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding)) {
        // Create two textures from each memory.
        wgpu::Texture textures[] = {memories[0].CreateTexture(), memories[1].CreateTexture()};

        // Make two command buffers, one that clears the texture, another that samples.
        wgpu::CommandBuffer commandBuffer0 =
            MakeFourColorsClearCommandBuffer(devices[0], textures[0]);
        auto [commandBuffer1, colorTarget] =
            MakeCheckBySamplingCommandBuffer(devices[1], textures[1]);

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.initialized = false;
        memories[0].BeginAccess(textures[0], &beginDesc);

        devices[0].GetQueue().Submit(1, &commandBuffer0);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        memories[0].EndAccess(textures[0], &endState);

        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.initialized = endState.initialized;

        memories[1].BeginAccess(textures[1], &beginDesc);
        devices[1].GetQueue().Submit(1, &commandBuffer1);
        memories[1].EndAccess(textures[1], &endState);

        CheckFourColors(devices[1], textures[1].GetFormat(), colorTarget);
    }
}

// Test rendering to a texture memory on one device, then sampling it using another device.
// Destroy the texture from the first device after submitting the commands, but before performing
// EndAccess. The second device should still be able to wait on the first device and see the
// results.
TEST_P(SharedTextureMemoryTests, RenderThenTextureDestroyBeforeEndAccessThenSample) {
    std::vector<wgpu::Device> devices = {device, CreateDevice()};
    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding)) {
        // Create two textures from each memory.
        wgpu::Texture textures[] = {memories[0].CreateTexture(), memories[1].CreateTexture()};

        // Make two command buffers, one that clears the texture, another that samples.
        wgpu::CommandBuffer commandBuffer0 =
            MakeFourColorsClearCommandBuffer(devices[0], textures[0]);
        auto [commandBuffer1, colorTarget] =
            MakeCheckBySamplingCommandBuffer(devices[1], textures[1]);

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.initialized = false;
        memories[0].BeginAccess(textures[0], &beginDesc);

        devices[0].GetQueue().Submit(1, &commandBuffer0);

        // Destroy the texture before performing EndAccess.
        textures[0].Destroy();

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        memories[0].EndAccess(textures[0], &endState);

        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.initialized = endState.initialized;

        memories[1].BeginAccess(textures[1], &beginDesc);
        devices[1].GetQueue().Submit(1, &commandBuffer1);
        memories[1].EndAccess(textures[1], &endState);

        CheckFourColors(devices[1], textures[1].GetFormat(), colorTarget);
    }
}

// Test accessing the memory on one device, dropping all memories, then
// accessing on the second device. Operations on the second device must
// still wait for the preceding operations to complete.
TEST_P(SharedTextureMemoryTests, RenderThenDropAllMemoriesThenSample) {
    std::vector<wgpu::Device> devices = {device, CreateDevice()};
    for (auto memories : GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding)) {
        // Create two textures from each memory.
        wgpu::Texture textures[] = {memories[0].CreateTexture(), memories[1].CreateTexture()};

        // Make two command buffers, one that clears the texture, another that samples.
        wgpu::CommandBuffer commandBuffer0 =
            MakeFourColorsClearCommandBuffer(devices[0], textures[0]);
        auto [commandBuffer1, colorTarget] =
            MakeCheckBySamplingCommandBuffer(devices[1], textures[1]);

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.initialized = false;
        wgpu::SharedTextureMemoryEndAccessState endState = {};
        // Render to the texture.
        {
            memories[0].BeginAccess(textures[0], &beginDesc);
            devices[0].GetQueue().Submit(1, &commandBuffer0);
            memories[0].EndAccess(textures[0], &endState);
        }

        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.initialized = endState.initialized;

        // Begin access, then drop all memories.
        memories[1].BeginAccess(textures[1], &beginDesc);
        memories.clear();

        // Sample from the texture and check the contents.
        devices[1].GetQueue().Submit(1, &commandBuffer1);
        CheckFourColors(devices[1], textures[1].GetFormat(), colorTarget);
    }
}

// Test rendering to a texture memory on one device, then sampling it using another device.
// Destroy or destroy the first device after submitting the commands, but before performing
// EndAccess. The second device should still be able to wait on the first device and see the
// results.
// This tests both cases where the device is destroyed, and where the device is lost.
TEST_P(SharedTextureMemoryTests, RenderThenLoseOrDestroyDeviceBeforeEndAccessThenSample) {
    // Not supported if using the same device. Not possible to lose one without losing the other.
    DAWN_TEST_UNSUPPORTED_IF(GetParam().mBackend->UseSameDevice());

    auto DoTest = [&](auto DestroyOrLoseDevice) {
        std::vector<wgpu::Device> devices = {CreateDevice(), CreateDevice()};
        auto perDeviceMemories =
            GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
                devices, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding);
        DAWN_TEST_UNSUPPORTED_IF(perDeviceMemories.empty());

        const auto& memories = perDeviceMemories[0];

        // Create two textures from each memory.
        wgpu::Texture textures[] = {memories[0].CreateTexture(), memories[1].CreateTexture()};

        // Make two command buffers, one that clears the texture, another that samples.
        wgpu::CommandBuffer commandBuffer0 =
            MakeFourColorsClearCommandBuffer(devices[0], textures[0]);
        auto [commandBuffer1, colorTarget] =
            MakeCheckBySamplingCommandBuffer(devices[1], textures[1]);

        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.initialized = false;
        memories[0].BeginAccess(textures[0], &beginDesc);

        devices[0].GetQueue().Submit(1, &commandBuffer0);

        // Destroy or lose the device before performing EndAccess.
        DestroyOrLoseDevice(devices[0]);

        wgpu::SharedTextureMemoryEndAccessState endState = {};
        memories[0].EndAccess(textures[0], &endState);
        EXPECT_GT(endState.fenceCount, 0u);

        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = endState.fenceCount;
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.initialized = endState.initialized;

        memories[1].BeginAccess(textures[1], &beginDesc);
        devices[1].GetQueue().Submit(1, &commandBuffer1);
        memories[1].EndAccess(textures[1], &endState);

        CheckFourColors(devices[1], textures[1].GetFormat(), colorTarget);
    };

    DoTest([](wgpu::Device d) { d.Destroy(); });

    DoTest([this](wgpu::Device d) { LoseDeviceForTesting(d); });
}

// Test a shared texture memory created on separate devices but wrapping the same underyling data.
// Write to the texture, then read from two separate devices concurrently, then write again.
// Reads should happen strictly after the writes. The final write should wait for the reads.
TEST_P(SharedTextureMemoryTests, SeparateDevicesWriteThenConcurrentReadThenWrite) {
    DAWN_TEST_UNSUPPORTED_IF(!GetParam().mBackend->SupportsConcurrentRead());

    std::vector<wgpu::Device> devices = {device, CreateDevice(), CreateDevice()};
    for (const auto& memories :
         GetParam().mBackend->CreatePerDeviceSharedTextureMemoriesFilterByUsage(
             devices, wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding)) {
        wgpu::SharedTextureMemoryProperties properties;
        memories[0].GetProperties(&properties);

        wgpu::TextureDescriptor writeTextureDesc = {};
        writeTextureDesc.format = properties.format;
        writeTextureDesc.size = properties.size;
        writeTextureDesc.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
        writeTextureDesc.label = "write texture";

        wgpu::TextureDescriptor readTextureDesc = {};
        readTextureDesc.format = properties.format;
        readTextureDesc.size = properties.size;
        readTextureDesc.usage = wgpu::TextureUsage::TextureBinding;
        readTextureDesc.label = "read texture";

        // Create three textures from each memory.
        // The first one will be written to.
        // The second two will be concurrently read after the write.
        // Then the first one will be written to again.
        wgpu::Texture textures[] = {memories[0].CreateTexture(&writeTextureDesc),
                                    memories[1].CreateTexture(&readTextureDesc),
                                    memories[2].CreateTexture(&readTextureDesc)};

        // Build command buffers for the test.
        wgpu::CommandBuffer writeCommandBuffer0 =
            MakeFourColorsClearCommandBuffer(devices[0], textures[0]);

        auto [checkCommandBuffer1, colorTarget1] =
            MakeCheckBySamplingCommandBuffer(devices[1], textures[1]);

        auto [checkCommandBuffer2, colorTarget2] =
            MakeCheckBySamplingCommandBuffer(devices[2], textures[2]);

        wgpu::CommandBuffer clearToGrayCommandBuffer0;
        {
            wgpu::CommandEncoder encoder = devices[0].CreateCommandEncoder();
            utils::ComboRenderPassDescriptor passDescriptor({textures[0].CreateView()});
            passDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
            passDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
            passDescriptor.cColorAttachments[0].clearValue = {0.5, 0.5, 0.5, 1.0};

            encoder.BeginRenderPass(&passDescriptor).End();
            clearToGrayCommandBuffer0 = encoder.Finish();
        }

        // Begin access on texture 0
        wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
        beginDesc.initialized = false;
        memories[0].BeginAccess(textures[0], &beginDesc);

        // Write
        devices[0].GetQueue().Submit(1, &writeCommandBuffer0);

        // End access on texture 0
        wgpu::SharedTextureMemoryEndAccessState endState = {};
        memories[0].EndAccess(textures[0], &endState);
        EXPECT_TRUE(endState.initialized);

        // Import fences to devices[1] and begin access.
        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[1], endState.fences[i]);
        }
        beginDesc.fenceCount = sharedFences.size();
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = endState.signaledValues;
        beginDesc.initialized = true;
        memories[1].BeginAccess(textures[1], &beginDesc);

        // Import fences to devices[2] and begin access.
        for (size_t i = 0; i < endState.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[2], endState.fences[i]);
        }
        memories[2].BeginAccess(textures[2], &beginDesc);

        // Check contents
        devices[1].GetQueue().Submit(1, &checkCommandBuffer1);
        devices[2].GetQueue().Submit(1, &checkCommandBuffer2);
        CheckFourColors(devices[1], textures[1].GetFormat(), colorTarget1);
        CheckFourColors(devices[2], textures[2].GetFormat(), colorTarget2);

        // End access on texture 1
        wgpu::SharedTextureMemoryEndAccessState endState1;
        memories[1].EndAccess(textures[1], &endState1);
        EXPECT_TRUE(endState1.initialized);

        // End access on texture 2
        wgpu::SharedTextureMemoryEndAccessState endState2;
        memories[2].EndAccess(textures[2], &endState2);
        EXPECT_TRUE(endState2.initialized);

        // Import fences back to devices[0]
        sharedFences.resize(endState1.fenceCount + endState2.fenceCount);
        std::vector<uint64_t> signaledValues(sharedFences.size());

        for (size_t i = 0; i < endState1.fenceCount; ++i) {
            sharedFences[i] = GetParam().mBackend->ImportFenceTo(devices[0], endState1.fences[i]);
            signaledValues[i] = endState1.signaledValues[i];
        }
        for (size_t i = 0; i < endState2.fenceCount; ++i) {
            sharedFences[i + endState1.fenceCount] =
                GetParam().mBackend->ImportFenceTo(devices[0], endState2.fences[i]);
            signaledValues[i + endState1.fenceCount] = endState2.signaledValues[i];
        }

        beginDesc.fenceCount = sharedFences.size();
        beginDesc.fences = sharedFences.data();
        beginDesc.signaledValues = signaledValues.data();
        beginDesc.initialized = true;

        // Begin access on texture 0
        memories[0].BeginAccess(textures[0], &beginDesc);

        // Submit a clear to gray.
        devices[0].GetQueue().Submit(1, &clearToGrayCommandBuffer0);
    }
}

}  // anonymous namespace
}  // namespace dawn
