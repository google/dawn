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

#include "tests/unittests/validation/ValidationTest.h"

#include "utils/WGPUHelpers.h"

class TextureInternalUsageValidationDisabledTest : public ValidationTest {};

// Test that using the feature is an error if it is not enabled
TEST_F(TextureInternalUsageValidationDisabledTest, RequiresFeature) {
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size = {1, 1};
    textureDesc.usage = wgpu::TextureUsage::CopySrc;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;

    // Control case: Normal texture creation works
    device.CreateTexture(&textureDesc);

    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    textureDesc.nextInChain = &internalDesc;

    // Error with chained feature struct.
    ASSERT_DEVICE_ERROR(device.CreateTexture(&textureDesc));

    // Also does not work with various internal usages.
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&textureDesc));

    internalDesc.internalUsage = wgpu::TextureUsage::CopyDst;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&textureDesc));
}

class TextureInternalUsageValidationTest : public ValidationTest {
    WGPUDevice CreateTestDevice() override {
        dawn_native::DawnDeviceDescriptor descriptor;
        descriptor.requiredFeatures.push_back("dawn-internal-usages");

        return adapter.CreateDevice(&descriptor);
    }
};

// Test that internal usages can be passed in a chained descriptor.
TEST_F(TextureInternalUsageValidationTest, Basic) {
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size = {1, 1};
    textureDesc.usage = wgpu::TextureUsage::CopySrc;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    textureDesc.nextInChain = &internalDesc;

    // Internal usage: none
    device.CreateTexture(&textureDesc);

    // Internal usage is the same as the base usage.
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
    device.CreateTexture(&textureDesc);

    // Internal usage adds to the base usage.
    internalDesc.internalUsage = wgpu::TextureUsage::CopyDst;
    device.CreateTexture(&textureDesc);
}

// Test that internal usages takes part in other validation that
// depends on the usage.
TEST_F(TextureInternalUsageValidationTest, UsageValidation) {
    {
        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.size = {1, 1};
        textureDesc.usage = wgpu::TextureUsage::CopySrc;
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;

        wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
        textureDesc.nextInChain = &internalDesc;

        // Internal usage adds an invalid usage.
        internalDesc.internalUsage = static_cast<wgpu::TextureUsage>(-1);
        ASSERT_DEVICE_ERROR(device.CreateTexture(&textureDesc));
    }

    {
        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.size = {1, 1};
        textureDesc.usage = wgpu::TextureUsage::CopySrc;
        textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        textureDesc.sampleCount = 4;

        // Control case: multisampled texture
        device.CreateTexture(&textureDesc);

        wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
        textureDesc.nextInChain = &internalDesc;

        // OK: internal usage adds nothing.
        device.CreateTexture(&textureDesc);

        // Internal usage adds storage usage which is invalid
        // with multisampling.
        internalDesc.internalUsage = wgpu::TextureUsage::StorageBinding;
        ASSERT_DEVICE_ERROR(device.CreateTexture(&textureDesc));
    }
}

// Test that internal usage does not add to the validated usage
// for command encoding
// This test also test the internal copy
TEST_F(TextureInternalUsageValidationTest, CommandValidation) {
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size = {1, 1};
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;

    textureDesc.usage = wgpu::TextureUsage::CopyDst;
    wgpu::Texture dst = device.CreateTexture(&textureDesc);

    textureDesc.usage = wgpu::TextureUsage::CopySrc;
    wgpu::Texture src = device.CreateTexture(&textureDesc);

    textureDesc.usage = wgpu::TextureUsage::None;

    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    textureDesc.nextInChain = &internalDesc;
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;

    wgpu::Texture srcInternal = device.CreateTexture(&textureDesc);

    // Control: src -> dst
    {
        wgpu::ImageCopyTexture srcImageCopyTexture = utils::CreateImageCopyTexture(src, 0, {0, 0});
        wgpu::ImageCopyTexture dstImageCopyTexture = utils::CreateImageCopyTexture(dst, 0, {0, 0});
        wgpu::Extent3D extent3D = {1, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&srcImageCopyTexture, &dstImageCopyTexture, &extent3D);
        encoder.Finish();
    }

    // Invalid: src internal -> dst
    {
        wgpu::ImageCopyTexture srcImageCopyTexture =
            utils::CreateImageCopyTexture(srcInternal, 0, {0, 0});
        wgpu::ImageCopyTexture dstImageCopyTexture = utils::CreateImageCopyTexture(dst, 0, {0, 0});
        wgpu::Extent3D extent3D = {1, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&srcImageCopyTexture, &dstImageCopyTexture, &extent3D);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Control with internal copy: src -> dst
    {
        wgpu::ImageCopyTexture srcImageCopyTexture = utils::CreateImageCopyTexture(src, 0, {0, 0});
        wgpu::ImageCopyTexture dstImageCopyTexture = utils::CreateImageCopyTexture(dst, 0, {0, 0});
        wgpu::Extent3D extent3D = {1, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTextureInternal(&srcImageCopyTexture, &dstImageCopyTexture, &extent3D);
        encoder.Finish();
    }

    // Valid with internal copy: src internal -> dst
    {
        wgpu::ImageCopyTexture srcImageCopyTexture =
            utils::CreateImageCopyTexture(srcInternal, 0, {0, 0});
        wgpu::ImageCopyTexture dstImageCopyTexture = utils::CreateImageCopyTexture(dst, 0, {0, 0});
        wgpu::Extent3D extent3D = {1, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTextureInternal(&srcImageCopyTexture, &dstImageCopyTexture, &extent3D);
        encoder.Finish();
    }
}
