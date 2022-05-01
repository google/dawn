// Copyright 2021 The Dawn Authors
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

#include "dawn/tests/unittests/validation/ValidationTest.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {
class ExternalTextureTest : public ValidationTest {
  public:
    wgpu::TextureDescriptor CreateTextureDescriptor(
        wgpu::TextureFormat format = kDefaultTextureFormat) {
        wgpu::TextureDescriptor descriptor;
        descriptor.size.width = kWidth;
        descriptor.size.height = kHeight;
        descriptor.size.depthOrArrayLayers = kDefaultDepth;
        descriptor.mipLevelCount = kDefaultMipLevels;
        descriptor.sampleCount = kDefaultSampleCount;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.format = format;
        descriptor.usage = kDefaultUsage;
        return descriptor;
    }

  protected:
    void SetUp() override {
        ValidationTest::SetUp();

        queue = device.GetQueue();
    }

    static constexpr uint32_t kWidth = 32;
    static constexpr uint32_t kHeight = 32;
    static constexpr uint32_t kDefaultDepth = 1;
    static constexpr uint32_t kDefaultMipLevels = 1;
    static constexpr uint32_t kDefaultSampleCount = 1;
    static constexpr wgpu::TextureUsage kDefaultUsage =
        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;

    static constexpr wgpu::TextureFormat kDefaultTextureFormat = wgpu::TextureFormat::RGBA8Unorm;
    static constexpr wgpu::TextureFormat kBiplanarPlane0Format = wgpu::TextureFormat::R8Unorm;
    static constexpr wgpu::TextureFormat kBiplanarPlane1Format = wgpu::TextureFormat::RG8Unorm;

    wgpu::Queue queue;
};

TEST_F(ExternalTextureTest, CreateExternalTextureValidation) {
    // Creating an external texture from a 2D, single-subresource texture should succeed.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc;
        externalDesc.plane0 = texture.CreateView();
        device.CreateExternalTexture(&externalDesc);
    }

    // Creating an external texture from a non-2D texture should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        textureDescriptor.dimension = wgpu::TextureDimension::e3D;
        textureDescriptor.usage = wgpu::TextureUsage::TextureBinding;
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc;
        externalDesc.plane0 = internalTexture.CreateView();
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Creating an external texture from a texture with mip count > 1 should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        textureDescriptor.mipLevelCount = 2;
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc;
        externalDesc.plane0 = internalTexture.CreateView();
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Creating an external texture from a texture without TextureUsage::TextureBinding should
    // fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        textureDescriptor.mipLevelCount = 2;
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc;
        externalDesc.plane0 = internalTexture.CreateView();
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Creating an external texture with an unsupported format should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        textureDescriptor.format = wgpu::TextureFormat::R8Uint;
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc;
        externalDesc.plane0 = internalTexture.CreateView();
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Creating an external texture with an multisampled texture should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        textureDescriptor.sampleCount = 4;
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc;
        externalDesc.plane0 = internalTexture.CreateView();
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Creating an external texture with an error texture view should fail.
    {
        wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
        wgpu::Texture internalTexture = device.CreateTexture(&textureDescriptor);

        wgpu::TextureViewDescriptor errorViewDescriptor;
        errorViewDescriptor.format = kDefaultTextureFormat;
        errorViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
        errorViewDescriptor.mipLevelCount = 1;
        errorViewDescriptor.arrayLayerCount = 2;
        ASSERT_DEVICE_ERROR(wgpu::TextureView errorTextureView =
                                internalTexture.CreateView(&errorViewDescriptor));

        wgpu::ExternalTextureDescriptor externalDesc;
        externalDesc.plane0 = errorTextureView;
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }
}

// Test that external texture creation works as expected in multiplane scenarios.
TEST_F(ExternalTextureTest, CreateMultiplanarExternalTextureValidation) {
    // Creating an external texture from two 2D, single-subresource textures with a biplanar
    // format should succeed.
    {
        wgpu::TextureDescriptor plane0TextureDescriptor =
            CreateTextureDescriptor(kBiplanarPlane0Format);
        wgpu::TextureDescriptor plane1TextureDescriptor =
            CreateTextureDescriptor(kBiplanarPlane1Format);
        wgpu::Texture texture0 = device.CreateTexture(&plane0TextureDescriptor);
        wgpu::Texture texture1 = device.CreateTexture(&plane1TextureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc;
        externalDesc.plane0 = texture0.CreateView();
        externalDesc.plane1 = texture1.CreateView();

        device.CreateExternalTexture(&externalDesc);
    }

    // Creating a multiplanar external texture with an unsupported format for plane0 should
    // result in an error.
    {
        wgpu::TextureDescriptor plane0TextureDescriptor =
            CreateTextureDescriptor(kDefaultTextureFormat);
        wgpu::TextureDescriptor plane1TextureDescriptor =
            CreateTextureDescriptor(kBiplanarPlane1Format);
        wgpu::Texture texture0 = device.CreateTexture(&plane0TextureDescriptor);
        wgpu::Texture texture1 = device.CreateTexture(&plane1TextureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc;
        externalDesc.plane0 = texture0.CreateView();
        externalDesc.plane1 = texture1.CreateView();

        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Creating a multiplanar external texture with an unsupported format for plane1 should
    // result in an error.
    {
        wgpu::TextureDescriptor plane0TextureDescriptor =
            CreateTextureDescriptor(kBiplanarPlane0Format);
        wgpu::TextureDescriptor plane1TextureDescriptor =
            CreateTextureDescriptor(kDefaultTextureFormat);
        wgpu::Texture texture0 = device.CreateTexture(&plane0TextureDescriptor);
        wgpu::Texture texture1 = device.CreateTexture(&plane1TextureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc;
        externalDesc.plane0 = texture0.CreateView();
        externalDesc.plane1 = texture1.CreateView();

        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }

    // Creating a multiplanar external texture with a non-sRGB color space should fail.
    {
        wgpu::TextureDescriptor plane0TextureDescriptor =
            CreateTextureDescriptor(kBiplanarPlane0Format);
        wgpu::TextureDescriptor plane1TextureDescriptor =
            CreateTextureDescriptor(kBiplanarPlane1Format);
        wgpu::Texture texture0 = device.CreateTexture(&plane0TextureDescriptor);
        wgpu::Texture texture1 = device.CreateTexture(&plane1TextureDescriptor);

        wgpu::ExternalTextureDescriptor externalDesc;
        externalDesc.plane0 = texture0.CreateView();
        externalDesc.plane1 = texture1.CreateView();
        externalDesc.colorSpace = wgpu::PredefinedColorSpace::Undefined;
        ASSERT_DEVICE_ERROR(device.CreateExternalTexture(&externalDesc));
    }
}

// Test that submitting a render pass that contains a destroyed external texture results in
// an error.
TEST_F(ExternalTextureTest, SubmitDestroyedExternalTextureInRenderPass) {
    wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    wgpu::ExternalTextureDescriptor externalDesc;
    externalDesc.plane0 = texture.CreateView();
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Create a bind group that contains the external texture.
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, externalTexture}});

    // Create another texture to use as a color attachment.
    wgpu::TextureDescriptor renderTextureDescriptor = CreateTextureDescriptor();
    wgpu::Texture renderTexture = device.CreateTexture(&renderTextureDescriptor);
    wgpu::TextureView renderView = renderTexture.CreateView();

    utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);

    // Control case should succeed.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();

        queue.Submit(1, &commands);
    }

    // Destroying the external texture should result in an error.
    {
        externalTexture.Destroy();
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

// Test that submitting a render pass that contains a dereferenced external texture results in
// success
TEST_F(ExternalTextureTest, SubmitDereferencedExternalTextureInRenderPass) {
    wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    wgpu::ExternalTextureDescriptor externalDesc;
    externalDesc.plane0 = texture.CreateView();
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Create a bind group that contains the external texture.
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, externalTexture}});

    // Create another texture to use as a color attachment.
    wgpu::TextureDescriptor renderTextureDescriptor = CreateTextureDescriptor();
    wgpu::Texture renderTexture = device.CreateTexture(&renderTextureDescriptor);
    wgpu::TextureView renderView = renderTexture.CreateView();

    utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);

    // Control case should succeed.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();

        queue.Submit(1, &commands);
    }

    // Dereferencing the external texture should not result in a use-after-free error.
    {
        externalTexture = nullptr;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }
}

// Test that submitting a render pass that contains a destroyed external texture plane
// results in an error.
TEST_F(ExternalTextureTest, SubmitDestroyedExternalTexturePlaneInRenderPass) {
    wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    wgpu::ExternalTextureDescriptor externalDesc;
    externalDesc.plane0 = texture.CreateView();
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Create a bind group that contains the external texture.
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, externalTexture}});

    // Create another texture to use as a color attachment.
    wgpu::TextureDescriptor renderTextureDescriptor = CreateTextureDescriptor();
    wgpu::Texture renderTexture = device.CreateTexture(&renderTextureDescriptor);
    wgpu::TextureView renderView = renderTexture.CreateView();

    utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);

    // Control case should succeed.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();

        queue.Submit(1, &commands);
    }

    // Destroying an external texture underlying plane should result in an error.
    {
        texture.Destroy();
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

// Test that submitting a compute pass that contains a destroyed external texture results in
// an error.
TEST_F(ExternalTextureTest, SubmitDestroyedExternalTextureInComputePass) {
    wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    wgpu::ExternalTextureDescriptor externalDesc;
    externalDesc.plane0 = texture.CreateView();
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Create a bind group that contains the external texture.
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, externalTexture}});

    wgpu::ComputePassDescriptor computePass;

    // Control case should succeed.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&computePass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();

        queue.Submit(1, &commands);
    }

    // Destroying the external texture should result in an error.
    {
        externalTexture.Destroy();
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&computePass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

// Test that submitting a compute pass that contains a destroyed external texture plane
// results in an error.
TEST_F(ExternalTextureTest, SubmitDestroyedExternalTexturePlaneInComputePass) {
    wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    wgpu::ExternalTextureDescriptor externalDesc;
    externalDesc.plane0 = texture.CreateView();
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Create a bind group that contains the external texture.
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, externalTexture}});

    wgpu::ComputePassDescriptor computePass;

    // Control case should succeed.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&computePass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    // Destroying an external texture underlying plane should result in an error.
    {
        texture.Destroy();
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&computePass);
        {
            pass.SetBindGroup(0, bindGroup);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
    }
}

// Ensure that bind group validation catches external textures mimatched from the BGL.
TEST_F(ExternalTextureTest, BindGroupDoesNotMatchLayout) {
    wgpu::TextureDescriptor textureDescriptor = CreateTextureDescriptor();
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    wgpu::ExternalTextureDescriptor externalDesc;
    externalDesc.plane0 = texture.CreateView();
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Control case should succeed.
    {
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});
        utils::MakeBindGroup(device, bgl, {{0, externalTexture}});
    }

    // Bind group creation should fail when an external texture is not present in the
    // corresponding slot of the bind group layout.
    {
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});
        ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, externalTexture}}));
    }
}

}  // namespace
