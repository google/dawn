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

// This file contains test for deprecated parts of Dawn's API while following WebGPU's evolution.
// It contains test for the "old" behavior that will be deleted once users are migrated, tests that
// a deprecation warning is emitted when the "old" behavior is used, and tests that an error is
// emitted when both the old and the new behavior are used (when applicable).

#include "tests/DawnTest.h"

#include "common/Constants.h"
#include "common/VertexFormatUtils.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

#include <cmath>

class DeprecationTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        // Skip when validation is off because warnings might be emitted during validation calls
        DAWN_SKIP_TEST_IF(HasToggleEnabled("skip_validation"));
    }
};

// Test that SetIndexBufferWithFormat is deprecated.
TEST_P(DeprecationTests, SetIndexBufferWithFormat) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 4;
    bufferDesc.usage = wgpu::BufferUsage::Index;
    wgpu::Buffer indexBuffer = device.CreateBuffer(&bufferDesc);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    EXPECT_DEPRECATION_WARNING(
        pass.SetIndexBufferWithFormat(indexBuffer, wgpu::IndexFormat::Uint32));
    pass.EndPass();
}

// Test that BindGroupLayoutEntry cannot have a type if buffer, sampler, texture, or storageTexture
// are defined.
TEST_P(DeprecationTests, BindGroupLayoutEntryTypeConflict) {
    wgpu::BindGroupLayoutEntry binding;
    binding.binding = 0;
    binding.visibility = wgpu::ShaderStage::Vertex;

    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = 1;
    descriptor.entries = &binding;

    // Succeeds with only a type.
    binding.type = wgpu::BindingType::UniformBuffer;
    EXPECT_DEPRECATION_WARNING(device.CreateBindGroupLayout(&descriptor));

    binding.type = wgpu::BindingType::Undefined;

    // Succeeds with only a buffer.type.
    binding.buffer.type = wgpu::BufferBindingType::Uniform;
    device.CreateBindGroupLayout(&descriptor);
    // Fails when both type and a buffer.type are specified.
    binding.type = wgpu::BindingType::UniformBuffer;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor));

    binding.buffer.type = wgpu::BufferBindingType::Undefined;
    binding.type = wgpu::BindingType::Undefined;

    // Succeeds with only a sampler.type.
    binding.sampler.type = wgpu::SamplerBindingType::Filtering;
    device.CreateBindGroupLayout(&descriptor);
    // Fails when both type and a sampler.type are specified.
    binding.type = wgpu::BindingType::Sampler;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor));

    binding.sampler.type = wgpu::SamplerBindingType::Undefined;
    binding.type = wgpu::BindingType::Undefined;

    // Succeeds with only a texture.sampleType.
    binding.texture.sampleType = wgpu::TextureSampleType::Float;
    device.CreateBindGroupLayout(&descriptor);
    // Fails when both type and a texture.sampleType are specified.
    binding.type = wgpu::BindingType::SampledTexture;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor));

    binding.texture.sampleType = wgpu::TextureSampleType::Undefined;
    binding.type = wgpu::BindingType::Undefined;

    // Succeeds with only a storageTexture.access.
    binding.storageTexture.access = wgpu::StorageTextureAccess::ReadOnly;
    binding.storageTexture.format = wgpu::TextureFormat::RGBA8Unorm;
    device.CreateBindGroupLayout(&descriptor);
    // Fails when both type and a storageTexture.access are specified.
    binding.type = wgpu::BindingType::ReadonlyStorageTexture;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&descriptor));
}

// Test that the deprecated BGLEntry path correctly handles the defaulting of viewDimension.
// This is a regression test for crbug.com/dawn/620
TEST_P(DeprecationTests, BindGroupLayoutEntryViewDimensionDefaulting) {
    wgpu::BindGroupLayoutEntry binding;
    binding.binding = 0;
    binding.visibility = wgpu::ShaderStage::Vertex;
    binding.type = wgpu::BindingType::SampledTexture;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = &binding;

    wgpu::BindGroupLayout bgl;

    // Check that the default viewDimension is 2D.
    {
        binding.viewDimension = wgpu::TextureViewDimension::Undefined;
        EXPECT_DEPRECATION_WARNING(bgl = device.CreateBindGroupLayout(&bglDesc));

        wgpu::TextureDescriptor desc;
        desc.usage = wgpu::TextureUsage::Sampled;
        desc.size = {1, 1, 1};
        desc.format = wgpu::TextureFormat::RGBA8Unorm;
        desc.dimension = wgpu::TextureDimension::e2D;
        wgpu::Texture texture = device.CreateTexture(&desc);

        // Success, the default is 2D and we give it a 2D view.
        utils::MakeBindGroup(device, bgl, {{0, texture.CreateView()}});
    }

    // Check that setting a non-default viewDimension works.
    {
        binding.viewDimension = wgpu::TextureViewDimension::e2DArray;
        EXPECT_DEPRECATION_WARNING(bgl = device.CreateBindGroupLayout(&bglDesc));

        wgpu::TextureDescriptor desc;
        desc.usage = wgpu::TextureUsage::Sampled;
        desc.size = {1, 1, 4};
        desc.format = wgpu::TextureFormat::RGBA8Unorm;
        desc.dimension = wgpu::TextureDimension::e2D;
        wgpu::Texture texture = device.CreateTexture(&desc);

        // Success, the view will be 2DArray and the BGL expects a 2DArray.
        utils::MakeBindGroup(device, bgl, {{0, texture.CreateView()}});
    }
}

// Test Device::GetDefaultQueue deprecation.
TEST_P(DeprecationTests, GetDefaultQueueDeprecation) {
    // Using GetDefaultQueue emits a warning.
    wgpu::Queue deprecatedQueue;
    EXPECT_DEPRECATION_WARNING(deprecatedQueue = device.GetDefaultQueue());

    // Using GetQueue doesn't emit a warning.
    wgpu::Queue queue = device.GetQueue();

    // Both objects are the same, even with dawn_wire.
    EXPECT_EQ(deprecatedQueue.Get(), queue.Get());
}

// Test that fences are deprecated.
TEST_P(DeprecationTests, CreateFence) {
    EXPECT_DEPRECATION_WARNING(queue.CreateFence());
}

// Test GPUExtent3D.depth deprecation in TextureDescriptor.size
TEST_P(DeprecationTests, GPUExtent3DDepthDeprecationTextureDescriptor) {
    wgpu::TextureDescriptor kBaseDesc;
    kBaseDesc.usage = wgpu::TextureUsage::Sampled;
    kBaseDesc.size.width = 1;
    kBaseDesc.size.height = 1;
    kBaseDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    kBaseDesc.dimension = wgpu::TextureDimension::e2D;

    {
        // Valid: default
        wgpu::TextureDescriptor desc = kBaseDesc;
        wgpu::Texture texture;
        texture = device.CreateTexture(&desc);
    }
    {
        // Warning: use deprecated depth but still valid
        wgpu::TextureDescriptor desc = kBaseDesc;
        desc.mipLevelCount = 2;
        desc.size.width = 2;
        desc.size.height = 2;
        desc.size.depth = 2;
        wgpu::Texture texture;
        EXPECT_DEPRECATION_WARNING(texture = device.CreateTexture(&desc));
    }
    {
        // Warning: use deprecated depth
        // Error: use deprecated depth and the descriptor is invalid
        // because 2D texture with depth == 0 is not allowed
        // This is to verify the deprecated depth is picked up by the implementation.
        wgpu::TextureDescriptor desc = kBaseDesc;
        desc.size.depth = 0;
        wgpu::Texture texture;
        ASSERT_DEVICE_ERROR(EXPECT_DEPRECATION_WARNING(texture = device.CreateTexture(&desc)));
    }
    {
        // Error: use both deprecated depth and depthOrArrayLayers
        wgpu::TextureDescriptor desc = kBaseDesc;
        desc.size.depth = 2;
        desc.size.depthOrArrayLayers = 2;
        wgpu::Texture texture;
        ASSERT_DEVICE_ERROR(texture = device.CreateTexture(&desc));
    }
    {
        // Valid: use updated depthOrArrayLayers
        wgpu::TextureDescriptor desc = kBaseDesc;
        desc.mipLevelCount = 2;
        desc.size.width = 2;
        desc.size.height = 2;
        desc.size.depthOrArrayLayers = 2;
        wgpu::Texture texture;
        texture = device.CreateTexture(&desc);
    }
    {
        // Error: use updated depthOrArrayLayers and the descriptor is invalid
        // because 2D texture with depthOrArrayLayers == 0 is not allowed
        wgpu::TextureDescriptor desc = kBaseDesc;
        desc.size.depthOrArrayLayers = 0;
        wgpu::Texture texture;
        ASSERT_DEVICE_ERROR(texture = device.CreateTexture(&desc));
    }
}

// Test GPUExtent3D.depth deprecation in CopyBufferToTexture, CopyTextureToBuffer, and
// CopyTextureToTexture
TEST_P(DeprecationTests, GPUExtent3DDepthDeprecationCopy) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 4 * 256;
    bufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer srcBuffer = device.CreateBuffer(&bufferDesc);

    wgpu::TextureDescriptor dstTextureDesc;
    dstTextureDesc.usage = wgpu::TextureUsage::CopyDst;
    dstTextureDesc.size.width = 4;
    dstTextureDesc.size.height = 4;
    dstTextureDesc.size.depthOrArrayLayers = 1;
    dstTextureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    dstTextureDesc.dimension = wgpu::TextureDimension::e2D;
    wgpu::Texture dstTexture = device.CreateTexture(&dstTextureDesc);

    wgpu::TextureDescriptor srcTextureDesc = dstTextureDesc;
    srcTextureDesc.usage = wgpu::TextureUsage::CopySrc;
    wgpu::Texture srcTexture = device.CreateTexture(&srcTextureDesc);

    wgpu::ImageCopyBuffer imageCopyBuffer = utils::CreateImageCopyBuffer(srcBuffer, 0, 256, 4);
    wgpu::ImageCopyTexture imageCopyDstTexture =
        utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
    wgpu::ImageCopyTexture imageCopySrcTexture =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);

    wgpu::Extent3D kBaseExtent3D;
    kBaseExtent3D.width = 4;
    kBaseExtent3D.height = 4;

    {
        // Valid: default
        wgpu::Extent3D extent3D = kBaseExtent3D;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyDstTexture, &extent3D);
        encoder.CopyTextureToBuffer(&imageCopySrcTexture, &imageCopyBuffer, &extent3D);
        encoder.CopyTextureToTexture(&imageCopySrcTexture, &imageCopyDstTexture, &extent3D);
        encoder.Finish();
    }
    {
        // Warning: empty copy use deprecated depth == 0 but still valid
        wgpu::Extent3D extent3D = kBaseExtent3D;
        extent3D.width = 0;
        extent3D.height = 0;
        extent3D.depth = 0;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EXPECT_DEPRECATION_WARNING(
            encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyDstTexture, &extent3D));
        EXPECT_DEPRECATION_WARNING(
            encoder.CopyTextureToBuffer(&imageCopySrcTexture, &imageCopyBuffer, &extent3D));
        EXPECT_DEPRECATION_WARNING(
            encoder.CopyTextureToTexture(&imageCopySrcTexture, &imageCopyDstTexture, &extent3D));
        encoder.Finish();
    }
    {
        // Warning: use deprecated depth
        // Error: depth > 1
        // This is to verify the deprecated depth is picked up by the implementation.
        wgpu::Extent3D extent3D = kBaseExtent3D;
        extent3D.depth = 2;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            EXPECT_DEPRECATION_WARNING(
                encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyDstTexture, &extent3D));
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            EXPECT_DEPRECATION_WARNING(
                encoder.CopyTextureToBuffer(&imageCopySrcTexture, &imageCopyBuffer, &extent3D));
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            EXPECT_DEPRECATION_WARNING(encoder.CopyTextureToTexture(
                &imageCopySrcTexture, &imageCopyDstTexture, &extent3D));
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }
    {
        // Error: use both deprecated depth and depthOrArrayLayers
        wgpu::Extent3D extent3D = kBaseExtent3D;
        extent3D.width = 0;
        extent3D.height = 0;
        extent3D.depth = 0;
        extent3D.depthOrArrayLayers = 0;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyDstTexture, &extent3D);
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyTextureToBuffer(&imageCopySrcTexture, &imageCopyBuffer, &extent3D);
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyTextureToTexture(&imageCopySrcTexture, &imageCopyDstTexture, &extent3D);
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }
    {
        // Valid: use updated depthOrArrayLayers
        wgpu::Extent3D extent3D = kBaseExtent3D;
        extent3D.width = 0;
        extent3D.height = 0;
        extent3D.depthOrArrayLayers = 0;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyDstTexture, &extent3D);
        encoder.CopyTextureToBuffer(&imageCopySrcTexture, &imageCopyBuffer, &extent3D);
        encoder.CopyTextureToTexture(&imageCopySrcTexture, &imageCopyDstTexture, &extent3D);
        encoder.Finish();
    }
    {
        // Error: use updated depthOrArrayLayers and is invalid
        wgpu::Extent3D extent3D = kBaseExtent3D;
        extent3D.depthOrArrayLayers = 2;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyDstTexture, &extent3D);
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyTextureToBuffer(&imageCopySrcTexture, &imageCopyBuffer, &extent3D);
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyTextureToTexture(&imageCopySrcTexture, &imageCopyDstTexture, &extent3D);
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }
}

// Test GPUExtent3D.depth deprecation in WriteTexture
TEST_P(DeprecationTests, GPUExtent3DDepthDeprecationWriteTexture) {
    wgpu::TextureDescriptor dstTextureDesc;
    dstTextureDesc.usage = wgpu::TextureUsage::CopyDst;
    dstTextureDesc.size.width = 4;
    dstTextureDesc.size.height = 4;
    dstTextureDesc.size.depthOrArrayLayers = 1;
    dstTextureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    dstTextureDesc.dimension = wgpu::TextureDimension::e2D;
    wgpu::Texture dstTexture = device.CreateTexture(&dstTextureDesc);

    size_t dataSize = 4 * 256;
    std::vector<uint8_t> data(dataSize);

    wgpu::TextureDataLayout textureDataLayout;
    textureDataLayout.offset = 0;
    textureDataLayout.bytesPerRow = 256;
    textureDataLayout.rowsPerImage = 4;

    wgpu::ImageCopyTexture imageCopyDstTexture =
        utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);

    wgpu::Extent3D kBaseExtent3D;
    kBaseExtent3D.width = 4;
    kBaseExtent3D.height = 4;

    {
        // Valid: default
        wgpu::Extent3D extent3D = kBaseExtent3D;
        wgpu::Queue queue = device.GetQueue();
        queue.WriteTexture(&imageCopyDstTexture, data.data(), dataSize, &textureDataLayout,
                           &extent3D);
    }
    {
        // Warning: use deprecated depth == 0 but still valid
        wgpu::Extent3D extent3D = kBaseExtent3D;
        extent3D.width = 0;
        extent3D.height = 0;
        extent3D.depth = 0;
        wgpu::Queue queue = device.GetQueue();
        EXPECT_DEPRECATION_WARNING(queue.WriteTexture(&imageCopyDstTexture, data.data(), dataSize,
                                                      &textureDataLayout, &extent3D));
    }
    {
        // Warning: use deprecated depth
        // Error: depth > 1 for 2D textures
        // This is to verify the deprecated depth is picked up by the implementation.
        wgpu::Extent3D extent3D = kBaseExtent3D;
        extent3D.depth = 2;
        wgpu::Queue queue = device.GetQueue();
        ASSERT_DEVICE_ERROR(EXPECT_DEPRECATION_WARNING(queue.WriteTexture(
            &imageCopyDstTexture, data.data(), dataSize, &textureDataLayout, &extent3D)));
    }
    {
        // Error: use both deprecated depth and depthOrArrayLayers
        wgpu::Extent3D extent3D = kBaseExtent3D;
        extent3D.width = 0;
        extent3D.height = 0;
        extent3D.depth = 0;
        extent3D.depthOrArrayLayers = 0;
        wgpu::Queue queue = device.GetQueue();
        ASSERT_DEVICE_ERROR(queue.WriteTexture(&imageCopyDstTexture, data.data(), dataSize,
                                               &textureDataLayout, &extent3D));
    }
    {
        // Valid: use updated depthOrArrayLayers
        wgpu::Extent3D extent3D = kBaseExtent3D;
        extent3D.width = 0;
        extent3D.height = 0;
        extent3D.depthOrArrayLayers = 0;
        wgpu::Queue queue = device.GetQueue();
        queue.WriteTexture(&imageCopyDstTexture, data.data(), dataSize, &textureDataLayout,
                           &extent3D);
    }
    {
        // Error: use updated depthOrArrayLayers and depthOrArrayLayers > 1 for 2D textures
        wgpu::Extent3D extent3D = kBaseExtent3D;
        extent3D.depthOrArrayLayers = 2;
        wgpu::Queue queue = device.GetQueue();
        ASSERT_DEVICE_ERROR(queue.WriteTexture(&imageCopyDstTexture, data.data(), dataSize,
                                               &textureDataLayout, &extent3D));
    }
}

DAWN_INSTANTIATE_TEST(DeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class ImageCopyBufferDeprecationTests : public DeprecationTests {
  protected:
    wgpu::ImageCopyTexture MakeImageCopyTexture() {
        wgpu::TextureDescriptor desc = {};
        desc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.size = {1, 1, 2};
        desc.format = wgpu::TextureFormat::RGBA8Unorm;

        wgpu::ImageCopyTexture copy;
        copy.texture = device.CreateTexture(&desc);
        copy.origin = {0, 0, 1};
        return copy;
    }

    wgpu::Extent3D copySize = {1, 1, 1};
};

// Tests that deprecated vertex formats properly raise a deprecation warning when used
class VertexFormatDeprecationTests : public DeprecationTests {
  protected:
    // Runs the test
    void DoTest(const wgpu::VertexFormat vertexFormat, bool deprecated) {
        std::string attribute = "[[location(0)]] var<in> a : ";
        attribute += dawn::GetWGSLVertexFormatType(vertexFormat);
        attribute += ";";

        std::string attribAccess = dawn::VertexFormatNumComponents(vertexFormat) > 1
                                       ? "vec4<f32>(f32(a.x), 0.0, 0.0, 1.0)"
                                       : "vec4<f32>(f32(a), 0.0, 0.0, 1.0)";

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, (attribute + R"(
                [[builtin(position)]] var<out> Position : vec4<f32>;

                [[stage(vertex)]] fn main() -> void {
                    Position = )" + attribAccess + R"(;
                    return;
                }
            )")
                                                                            .c_str());
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
                [[location(0)]] var<out> outColor : vec4<f32>;

                [[stage(fragment)]] fn main() -> void {
                    outColor = vec4<f32>(1.0, 1.0, 1.0, 1.0);
                    return;
                }
            )");

        utils::ComboRenderPipelineDescriptor2 descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
        descriptor.vertex.bufferCount = 1;
        descriptor.cBuffers[0].arrayStride = 32;
        descriptor.cBuffers[0].attributeCount = 1;
        descriptor.cAttributes[0].format = vertexFormat;
        descriptor.cAttributes[0].offset = 0;
        descriptor.cAttributes[0].shaderLocation = 0;
        descriptor.cTargets[0].format = utils::BasicRenderPass::kDefaultColorFormat;

        if (deprecated) {
            EXPECT_DEPRECATION_WARNING(device.CreateRenderPipeline2(&descriptor));
        } else {
            device.CreateRenderPipeline2(&descriptor);
        }
    }
};

TEST_P(VertexFormatDeprecationTests, NewVertexFormats) {
    // Using the new vertex formats does not emit a warning.
    for (auto& format : dawn::kAllVertexFormats) {
        DoTest(format, false);
    }
}

TEST_P(VertexFormatDeprecationTests, DeprecatedVertexFormats) {
    // Using deprecated vertex formats does emit a warning.
    for (auto& format : dawn::kAllDeprecatedVertexFormats) {
        DoTest(format, true);
    }
}

DAWN_INSTANTIATE_TEST(VertexFormatDeprecationTests,
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
