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

#include <utility>

#include "dawn/common/Math.h"
#include "dawn/native/vulkan/AdapterVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/tests/white_box/VulkanImageWrappingTests.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn::native::vulkan {

using ExternalTexture = VulkanImageWrappingTestBackend::ExternalTexture;
using ExternalSemaphore = VulkanImageWrappingTestBackend::ExternalSemaphore;

namespace {

class VulkanImageWrappingTestBase : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::DawnInternalUsages};
    }

  public:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());

        mBackend = VulkanImageWrappingTestBackend::Create(device);

        defaultDescriptor.dimension = wgpu::TextureDimension::e2D;
        defaultDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        defaultDescriptor.size = {1, 1, 1};
        defaultDescriptor.sampleCount = 1;
        defaultDescriptor.mipLevelCount = 1;
        defaultDescriptor.usage = wgpu::TextureUsage::RenderAttachment |
                                  wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;

        defaultTexture =
            mBackend->CreateTexture(1, 1, defaultDescriptor.format, defaultDescriptor.usage);
    }

    void TearDown() override {
        if (UsesWire()) {
            DawnTest::TearDown();
            return;
        }

        defaultTexture = nullptr;
        mBackend = nullptr;
        DawnTest::TearDown();
    }

    wgpu::Texture WrapVulkanImage(wgpu::Device dawnDevice,
                                  const wgpu::TextureDescriptor* textureDescriptor,
                                  const ExternalTexture* externalTexture,
                                  std::vector<std::unique_ptr<ExternalSemaphore>> semaphores,
                                  bool isInitialized = true,
                                  bool expectValid = true) {
        ExternalImageDescriptorVkForTesting descriptor;
        return WrapVulkanImage(dawnDevice, textureDescriptor, externalTexture,
                               std::move(semaphores), descriptor.releasedOldLayout,
                               descriptor.releasedNewLayout, isInitialized, expectValid);
    }

    wgpu::Texture WrapVulkanImage(wgpu::Device dawnDevice,
                                  const wgpu::TextureDescriptor* textureDescriptor,
                                  const ExternalTexture* externalTexture,
                                  std::vector<std::unique_ptr<ExternalSemaphore>> semaphores,
                                  VkImageLayout releasedOldLayout,
                                  VkImageLayout releasedNewLayout,
                                  bool isInitialized = true,
                                  bool expectValid = true) {
        ExternalImageDescriptorVkForTesting descriptor;
        descriptor.cTextureDescriptor =
            reinterpret_cast<const WGPUTextureDescriptor*>(textureDescriptor);
        descriptor.isInitialized = isInitialized;
        descriptor.releasedOldLayout = releasedOldLayout;
        descriptor.releasedNewLayout = releasedNewLayout;

        wgpu::Texture texture =
            mBackend->WrapImage(dawnDevice, externalTexture, descriptor, std::move(semaphores));

        if (expectValid) {
            EXPECT_NE(texture, nullptr) << "Failed to wrap image, are external memory / "
                                           "semaphore extensions supported?";
        } else {
            EXPECT_EQ(texture, nullptr);
        }

        return texture;
    }

    // Exports the signal from a wrapped texture and ignores it
    // We have to export the signal before destroying the wrapped texture else it's an
    // assertion failure
    void IgnoreSignalSemaphore(wgpu::Texture wrappedTexture) {
        ExternalImageExportInfoVkForTesting exportInfo;
        bool result = mBackend->ExportImage(wrappedTexture, VK_IMAGE_LAYOUT_GENERAL, &exportInfo);
        ASSERT(result);
    }

  protected:
    std::unique_ptr<VulkanImageWrappingTestBackend> mBackend;

    wgpu::TextureDescriptor defaultDescriptor;
    std::unique_ptr<ExternalTexture> defaultTexture;
};

}  // namespace

using VulkanImageWrappingValidationTests = VulkanImageWrappingTestBase;

// Test no error occurs if the import is valid
TEST_P(VulkanImageWrappingValidationTests, SuccessfulImport) {
    wgpu::Texture texture =
        WrapVulkanImage(device, &defaultDescriptor, defaultTexture.get(), {}, true, true);
    EXPECT_NE(texture.Get(), nullptr);
    IgnoreSignalSemaphore(texture);
}

// Test no error occurs if the import is valid with DawnTextureInternalUsageDescriptor
TEST_P(VulkanImageWrappingValidationTests, SuccessfulImportWithInternalUsageDescriptor) {
    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    defaultDescriptor.nextInChain = &internalDesc;
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
    internalDesc.sType = wgpu::SType::DawnTextureInternalUsageDescriptor;

    wgpu::Texture texture =
        WrapVulkanImage(device, &defaultDescriptor, defaultTexture.get(), {}, true, true);
    EXPECT_NE(texture.Get(), nullptr);
    IgnoreSignalSemaphore(texture);
}

// Test an error occurs if an invalid sType is the nextInChain
TEST_P(VulkanImageWrappingValidationTests, InvalidTextureDescriptor) {
    wgpu::ChainedStruct chainedDescriptor;
    chainedDescriptor.sType = wgpu::SType::SurfaceDescriptorFromWindowsSwapChainPanel;
    defaultDescriptor.nextInChain = &chainedDescriptor;

    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapVulkanImage(
                            device, &defaultDescriptor, defaultTexture.get(), {}, true, false));
    EXPECT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor dimension isn't 2D
TEST_P(VulkanImageWrappingValidationTests, InvalidTextureDimension) {
    defaultDescriptor.dimension = wgpu::TextureDimension::e1D;

    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapVulkanImage(
                            device, &defaultDescriptor, defaultTexture.get(), {}, true, false));
    EXPECT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor mip level count isn't 1
TEST_P(VulkanImageWrappingValidationTests, InvalidMipLevelCount) {
    defaultDescriptor.mipLevelCount = 2;

    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapVulkanImage(
                            device, &defaultDescriptor, defaultTexture.get(), {}, true, false));
    EXPECT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor depth isn't 1
TEST_P(VulkanImageWrappingValidationTests, InvalidDepth) {
    defaultDescriptor.size.depthOrArrayLayers = 2;

    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapVulkanImage(
                            device, &defaultDescriptor, defaultTexture.get(), {}, true, false));
    EXPECT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor sample count isn't 1
TEST_P(VulkanImageWrappingValidationTests, InvalidSampleCount) {
    defaultDescriptor.sampleCount = 4;

    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapVulkanImage(
                            device, &defaultDescriptor, defaultTexture.get(), {}, true, false));
    EXPECT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if we try to export the signal semaphore twice
TEST_P(VulkanImageWrappingValidationTests, DoubleSignalSemaphoreExport) {
    wgpu::Texture texture =
        WrapVulkanImage(device, &defaultDescriptor, defaultTexture.get(), {}, true, true);
    ASSERT_NE(texture.Get(), nullptr);
    IgnoreSignalSemaphore(texture);

    ExternalImageExportInfoVkForTesting exportInfo;
    ASSERT_DEVICE_ERROR(bool success =
                            mBackend->ExportImage(texture, VK_IMAGE_LAYOUT_GENERAL, &exportInfo));
    ASSERT_FALSE(success);
    ASSERT_EQ(exportInfo.semaphores.size(), 0u);
}

// Test an error occurs if we try to export the signal semaphore from a normal texture
TEST_P(VulkanImageWrappingValidationTests, NormalTextureSignalSemaphoreExport) {
    wgpu::Texture texture = device.CreateTexture(&defaultDescriptor);
    ASSERT_NE(texture.Get(), nullptr);

    ExternalImageExportInfoVkForTesting exportInfo;
    ASSERT_DEVICE_ERROR(bool success =
                            mBackend->ExportImage(texture, VK_IMAGE_LAYOUT_GENERAL, &exportInfo));
    ASSERT_FALSE(success);
    ASSERT_EQ(exportInfo.semaphores.size(), 0u);
}

// Test an error occurs if we try to export the signal semaphore from a destroyed texture
TEST_P(VulkanImageWrappingValidationTests, DestroyedTextureSignalSemaphoreExport) {
    wgpu::Texture texture = device.CreateTexture(&defaultDescriptor);
    ASSERT_NE(texture.Get(), nullptr);
    texture.Destroy();

    ExternalImageExportInfoVkForTesting exportInfo;
    ASSERT_DEVICE_ERROR(bool success =
                            mBackend->ExportImage(texture, VK_IMAGE_LAYOUT_GENERAL, &exportInfo));
    ASSERT_FALSE(success);
    ASSERT_EQ(exportInfo.semaphores.size(), 0u);
}

// Fixture to test using external memory textures through different usages.
// These tests are skipped if the harness is using the wire.
class VulkanImageWrappingUsageTests : public VulkanImageWrappingTestBase {
  public:
    void SetUp() override {
        VulkanImageWrappingTestBase::SetUp();
        if (UsesWire()) {
            return;
        }

        // Create another device based on the original
        backendAdapter =
            dawn::native::vulkan::ToBackend(dawn::native::FromAPI(device.Get())->GetAdapter());
        deviceDescriptor.nextInChain = &togglesDesc;
        togglesDesc.forceEnabledToggles = GetParam().forceEnabledWorkarounds.data();
        togglesDesc.forceEnabledTogglesCount = GetParam().forceEnabledWorkarounds.size();
        togglesDesc.forceDisabledToggles = GetParam().forceDisabledWorkarounds.data();
        togglesDesc.forceDisabledTogglesCount = GetParam().forceDisabledWorkarounds.size();

        secondDeviceVk =
            dawn::native::vulkan::ToBackend(backendAdapter->APICreateDevice(&deviceDescriptor));
        secondDevice = wgpu::Device::Acquire(dawn::native::ToAPI(secondDeviceVk));
    }

  protected:
    dawn::native::vulkan::Adapter* backendAdapter;
    dawn::native::DeviceDescriptor deviceDescriptor;
    dawn::native::DawnTogglesDeviceDescriptor togglesDesc;

    wgpu::Device secondDevice;
    dawn::native::vulkan::Device* secondDeviceVk;

    // Clear a texture on a given device
    void ClearImage(wgpu::Device dawnDevice, wgpu::Texture wrappedTexture, wgpu::Color clearColor) {
        wgpu::TextureView wrappedView = wrappedTexture.CreateView();

        // Submit a clear operation
        utils::ComboRenderPassDescriptor renderPassDescriptor({wrappedView}, {});
        renderPassDescriptor.cColorAttachments[0].clearValue = clearColor;
        renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;

        wgpu::CommandEncoder encoder = dawnDevice.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescriptor);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();

        wgpu::Queue queue = dawnDevice.GetQueue();
        queue.Submit(1, &commands);
    }

    // Submits a 1x1x1 copy from source to destination
    void SimpleCopyTextureToTexture(wgpu::Device dawnDevice,
                                    wgpu::Queue dawnQueue,
                                    wgpu::Texture source,
                                    wgpu::Texture destination) {
        wgpu::ImageCopyTexture copySrc = utils::CreateImageCopyTexture(source, 0, {0, 0, 0});
        wgpu::ImageCopyTexture copyDst = utils::CreateImageCopyTexture(destination, 0, {0, 0, 0});

        wgpu::Extent3D copySize = {1, 1, 1};

        wgpu::CommandEncoder encoder = dawnDevice.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
        wgpu::CommandBuffer commands = encoder.Finish();

        dawnQueue.Submit(1, &commands);
    }
};

// Clear an image in |secondDevice|
// Verify clear color is visible in |device|
TEST_P(VulkanImageWrappingUsageTests, ClearImageAcrossDevices) {
    // Import the image on |secondDevice|
    wgpu::Texture wrappedTexture =
        WrapVulkanImage(secondDevice, &defaultDescriptor, defaultTexture.get(), {},
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Clear |wrappedTexture| on |secondDevice|
    ClearImage(secondDevice, wrappedTexture, {1 / 255.0f, 2 / 255.0f, 3 / 255.0f, 4 / 255.0f});

    ExternalImageExportInfoVkForTesting exportInfo;
    ASSERT_TRUE(
        mBackend->ExportImage(wrappedTexture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, &exportInfo));

    // Import the image to |device|, making sure we wait on signalFd
    wgpu::Texture nextWrappedTexture = WrapVulkanImage(
        device, &defaultDescriptor, defaultTexture.get(), std::move(exportInfo.semaphores),
        exportInfo.releasedOldLayout, exportInfo.releasedNewLayout);

    // Verify |device| sees the changes from |secondDevice|
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 2, 3, 4), nextWrappedTexture, 0, 0);

    IgnoreSignalSemaphore(nextWrappedTexture);
}

// Clear an image in |secondDevice|
// Verify clear color is not visible in |device| if we import the texture as not cleared
TEST_P(VulkanImageWrappingUsageTests, UninitializedTextureIsCleared) {
    // Import the image on |secondDevice|
    wgpu::Texture wrappedTexture =
        WrapVulkanImage(secondDevice, &defaultDescriptor, defaultTexture.get(), {},
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Clear |wrappedTexture| on |secondDevice|
    ClearImage(secondDevice, wrappedTexture, {1 / 255.0f, 2 / 255.0f, 3 / 255.0f, 4 / 255.0f});

    ExternalImageExportInfoVkForTesting exportInfo;
    ASSERT_TRUE(
        mBackend->ExportImage(wrappedTexture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, &exportInfo));

    // Import the image to |device|, making sure we wait on signalFd
    wgpu::Texture nextWrappedTexture = WrapVulkanImage(
        device, &defaultDescriptor, defaultTexture.get(), std::move(exportInfo.semaphores),
        exportInfo.releasedOldLayout, exportInfo.releasedNewLayout, false);

    // Verify |device| doesn't see the changes from |secondDevice|
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), nextWrappedTexture, 0, 0);

    IgnoreSignalSemaphore(nextWrappedTexture);
}

// Import a texture into |secondDevice|
// Clear the texture on |secondDevice|
// Issue a copy of the imported texture inside |device| to |copyDstTexture|
// Verify the clear color from |secondDevice| is visible in |copyDstTexture|
TEST_P(VulkanImageWrappingUsageTests, CopyTextureToTextureSrcSync) {
    // Import the image on |secondDevice|
    wgpu::Texture wrappedTexture =
        WrapVulkanImage(secondDevice, &defaultDescriptor, defaultTexture.get(), {},
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Clear |wrappedTexture| on |secondDevice|
    ClearImage(secondDevice, wrappedTexture, {1 / 255.0f, 2 / 255.0f, 3 / 255.0f, 4 / 255.0f});

    ExternalImageExportInfoVkForTesting exportInfo;
    ASSERT_TRUE(
        mBackend->ExportImage(wrappedTexture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, &exportInfo));

    // Import the image to |device|, making sure we wait on |signalFd|
    wgpu::Texture deviceWrappedTexture = WrapVulkanImage(
        device, &defaultDescriptor, defaultTexture.get(), std::move(exportInfo.semaphores),
        exportInfo.releasedOldLayout, exportInfo.releasedNewLayout);

    // Create a second texture on |device|
    wgpu::Texture copyDstTexture = device.CreateTexture(&defaultDescriptor);

    // Copy |deviceWrappedTexture| into |copyDstTexture|
    SimpleCopyTextureToTexture(device, queue, deviceWrappedTexture, copyDstTexture);

    // Verify |copyDstTexture| sees changes from |secondDevice|
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 2, 3, 4), copyDstTexture, 0, 0);

    IgnoreSignalSemaphore(deviceWrappedTexture);
}

// Import a texture into |device|
// Clear texture with color A on |device|
// Import same texture into |secondDevice|, waiting on the copy signal
// Clear the new texture with color B on |secondDevice|
// Copy color B using Texture to Texture copy on |secondDevice|
// Import texture back into |device|, waiting on color B signal
// Verify texture contains color B
// If texture destination isn't synchronized, |secondDevice| could copy color B
// into the texture first, then |device| writes color A
TEST_P(VulkanImageWrappingUsageTests, CopyTextureToTextureDstSync) {
    // Import the image on |device|
    wgpu::Texture wrappedTexture =
        WrapVulkanImage(device, &defaultDescriptor, defaultTexture.get(), {},
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Clear |wrappedTexture| on |device|
    ClearImage(device, wrappedTexture, {5 / 255.0f, 6 / 255.0f, 7 / 255.0f, 8 / 255.0f});

    ExternalImageExportInfoVkForTesting exportInfo;
    ASSERT_TRUE(
        mBackend->ExportImage(wrappedTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &exportInfo));

    // Import the image to |secondDevice|, making sure we wait on |signalFd|
    wgpu::Texture secondDeviceWrappedTexture = WrapVulkanImage(
        secondDevice, &defaultDescriptor, defaultTexture.get(), std::move(exportInfo.semaphores),
        exportInfo.releasedOldLayout, exportInfo.releasedNewLayout);

    // Create a texture with color B on |secondDevice|
    wgpu::Texture copySrcTexture = secondDevice.CreateTexture(&defaultDescriptor);
    ClearImage(secondDevice, copySrcTexture, {1 / 255.0f, 2 / 255.0f, 3 / 255.0f, 4 / 255.0f});

    // Copy color B on |secondDevice|
    wgpu::Queue secondDeviceQueue = secondDevice.GetQueue();
    SimpleCopyTextureToTexture(secondDevice, secondDeviceQueue, copySrcTexture,
                               secondDeviceWrappedTexture);

    // Re-import back into |device|, waiting on |secondDevice|'s signal
    ExternalImageExportInfoVkForTesting secondExportInfo;
    ASSERT_TRUE(mBackend->ExportImage(secondDeviceWrappedTexture,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, &secondExportInfo));

    wgpu::Texture nextWrappedTexture = WrapVulkanImage(
        device, &defaultDescriptor, defaultTexture.get(), std::move(secondExportInfo.semaphores),
        secondExportInfo.releasedOldLayout, secondExportInfo.releasedNewLayout);

    // Verify |nextWrappedTexture| contains the color from our copy
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 2, 3, 4), nextWrappedTexture, 0, 0);

    IgnoreSignalSemaphore(nextWrappedTexture);
}

// Import a texture from |secondDevice|
// Clear the texture on |secondDevice|
// Issue a copy of the imported texture inside |device| to |copyDstBuffer|
// Verify the clear color from |secondDevice| is visible in |copyDstBuffer|
TEST_P(VulkanImageWrappingUsageTests, CopyTextureToBufferSrcSync) {
    // Import the image on |secondDevice|
    wgpu::Texture wrappedTexture =
        WrapVulkanImage(secondDevice, &defaultDescriptor, defaultTexture.get(), {},
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Clear |wrappedTexture| on |secondDevice|
    ClearImage(secondDevice, wrappedTexture, {1 / 255.0f, 2 / 255.0f, 3 / 255.0f, 4 / 255.0f});

    ExternalImageExportInfoVkForTesting exportInfo;
    ASSERT_TRUE(
        mBackend->ExportImage(wrappedTexture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, &exportInfo));

    // Import the image to |device|, making sure we wait on |signalFd|
    wgpu::Texture deviceWrappedTexture = WrapVulkanImage(
        device, &defaultDescriptor, defaultTexture.get(), std::move(exportInfo.semaphores),
        exportInfo.releasedOldLayout, exportInfo.releasedNewLayout);

    // Create a destination buffer on |device|
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 4;
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer copyDstBuffer = device.CreateBuffer(&bufferDesc);

    // Copy |deviceWrappedTexture| into |copyDstBuffer|
    wgpu::ImageCopyTexture copySrc =
        utils::CreateImageCopyTexture(deviceWrappedTexture, 0, {0, 0, 0});
    wgpu::ImageCopyBuffer copyDst = utils::CreateImageCopyBuffer(copyDstBuffer, 0, 256);

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Verify |copyDstBuffer| sees changes from |secondDevice|
    uint32_t expected = 0x04030201;
    EXPECT_BUFFER_U32_EQ(expected, copyDstBuffer, 0);

    IgnoreSignalSemaphore(deviceWrappedTexture);
}

// Import a texture into |device|
// Clear texture with color A on |device|
// Import same texture into |secondDevice|, waiting on the copy signal
// Copy color B using Buffer to Texture copy on |secondDevice|
// Import texture back into |device|, waiting on color B signal
// Verify texture contains color B
// If texture destination isn't synchronized, |secondDevice| could copy color B
// into the texture first, then |device| writes color A
TEST_P(VulkanImageWrappingUsageTests, CopyBufferToTextureDstSync) {
    // Import the image on |device|
    wgpu::Texture wrappedTexture =
        WrapVulkanImage(device, &defaultDescriptor, defaultTexture.get(), {},
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Clear |wrappedTexture| on |device|
    ClearImage(device, wrappedTexture, {5 / 255.0f, 6 / 255.0f, 7 / 255.0f, 8 / 255.0f});

    ExternalImageExportInfoVkForTesting exportInfo;
    ASSERT_TRUE(
        mBackend->ExportImage(wrappedTexture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, &exportInfo));

    // Import the image to |secondDevice|, making sure we wait on |signalFd|
    wgpu::Texture secondDeviceWrappedTexture = WrapVulkanImage(
        secondDevice, &defaultDescriptor, defaultTexture.get(), std::move(exportInfo.semaphores),
        exportInfo.releasedOldLayout, exportInfo.releasedNewLayout);

    // Copy color B on |secondDevice|
    wgpu::Queue secondDeviceQueue = secondDevice.GetQueue();

    // Create a buffer on |secondDevice|
    wgpu::Buffer copySrcBuffer =
        utils::CreateBufferFromData(secondDevice, wgpu::BufferUsage::CopySrc, {0x04030201});

    // Copy |copySrcBuffer| into |secondDeviceWrappedTexture|
    wgpu::ImageCopyBuffer copySrc = utils::CreateImageCopyBuffer(copySrcBuffer, 0, 256);
    wgpu::ImageCopyTexture copyDst =
        utils::CreateImageCopyTexture(secondDeviceWrappedTexture, 0, {0, 0, 0});

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = secondDevice.CreateCommandEncoder();
    encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
    wgpu::CommandBuffer commands = encoder.Finish();
    secondDeviceQueue.Submit(1, &commands);

    // Re-import back into |device|, waiting on |secondDevice|'s signal
    ExternalImageExportInfoVkForTesting secondExportInfo;
    ASSERT_TRUE(mBackend->ExportImage(secondDeviceWrappedTexture,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, &secondExportInfo));

    wgpu::Texture nextWrappedTexture = WrapVulkanImage(
        device, &defaultDescriptor, defaultTexture.get(), std::move(secondExportInfo.semaphores),
        secondExportInfo.releasedOldLayout, secondExportInfo.releasedNewLayout);

    // Verify |nextWrappedTexture| contains the color from our copy
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 2, 3, 4), nextWrappedTexture, 0, 0);

    IgnoreSignalSemaphore(nextWrappedTexture);
}

// Import a texture from |secondDevice|
// Clear the texture on |secondDevice|
// Issue a copy of the imported texture inside |device| to |copyDstTexture|
// Issue second copy to |secondCopyDstTexture|
// Verify the clear color from |secondDevice| is visible in both copies
TEST_P(VulkanImageWrappingUsageTests, DoubleTextureUsage) {
    // Import the image on |secondDevice|
    wgpu::Texture wrappedTexture =
        WrapVulkanImage(secondDevice, &defaultDescriptor, defaultTexture.get(), {},
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Clear |wrappedTexture| on |secondDevice|
    ClearImage(secondDevice, wrappedTexture, {1 / 255.0f, 2 / 255.0f, 3 / 255.0f, 4 / 255.0f});

    ExternalImageExportInfoVkForTesting exportInfo;
    ASSERT_TRUE(
        mBackend->ExportImage(wrappedTexture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, &exportInfo));

    // Import the image to |device|, making sure we wait on |signalFd|
    wgpu::Texture deviceWrappedTexture = WrapVulkanImage(
        device, &defaultDescriptor, defaultTexture.get(), std::move(exportInfo.semaphores),
        exportInfo.releasedOldLayout, exportInfo.releasedNewLayout);

    // Create a second texture on |device|
    wgpu::Texture copyDstTexture = device.CreateTexture(&defaultDescriptor);

    // Create a third texture on |device|
    wgpu::Texture secondCopyDstTexture = device.CreateTexture(&defaultDescriptor);

    // Copy |deviceWrappedTexture| into |copyDstTexture|
    SimpleCopyTextureToTexture(device, queue, deviceWrappedTexture, copyDstTexture);

    // Copy |deviceWrappedTexture| into |secondCopyDstTexture|
    SimpleCopyTextureToTexture(device, queue, deviceWrappedTexture, secondCopyDstTexture);

    // Verify |copyDstTexture| sees changes from |secondDevice|
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 2, 3, 4), copyDstTexture, 0, 0);

    // Verify |secondCopyDstTexture| sees changes from |secondDevice|
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 2, 3, 4), secondCopyDstTexture, 0, 0);

    IgnoreSignalSemaphore(deviceWrappedTexture);
}

// Tex A on device 3 (external export)
// Tex B on device 2 (external export)
// Tex C on device 1 (external export)
// Clear color for A on device 3
// Copy A->B on device 3
// Copy B->C on device 2 (wait on B from previous op)
// Copy C->D on device 1 (wait on C from previous op)
// Verify D has same color as A
TEST_P(VulkanImageWrappingUsageTests, ChainTextureCopy) {
    // device 1 = |device|
    // device 2 = |secondDevice|
    // Create device 3
    dawn::native::vulkan::Device* thirdDeviceVk =
        dawn::native::vulkan::ToBackend(backendAdapter->APICreateDevice(&deviceDescriptor));
    wgpu::Device thirdDevice = wgpu::Device::Acquire(dawn::native::ToAPI(thirdDeviceVk));

    // Make queue for device 2 and 3
    wgpu::Queue secondDeviceQueue = secondDevice.GetQueue();
    wgpu::Queue thirdDeviceQueue = thirdDevice.GetQueue();

    // Create textures A, B, C
    std::unique_ptr<ExternalTexture> textureA =
        mBackend->CreateTexture(1, 1, wgpu::TextureFormat::RGBA8Unorm, defaultDescriptor.usage);
    std::unique_ptr<ExternalTexture> textureB =
        mBackend->CreateTexture(1, 1, wgpu::TextureFormat::RGBA8Unorm, defaultDescriptor.usage);
    std::unique_ptr<ExternalTexture> textureC =
        mBackend->CreateTexture(1, 1, wgpu::TextureFormat::RGBA8Unorm, defaultDescriptor.usage);

    // Import TexA, TexB on device 3
    wgpu::Texture wrappedTexADevice3 =
        WrapVulkanImage(thirdDevice, &defaultDescriptor, textureA.get(), {},
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    wgpu::Texture wrappedTexBDevice3 =
        WrapVulkanImage(thirdDevice, &defaultDescriptor, textureB.get(), {},
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Clear TexA
    ClearImage(thirdDevice, wrappedTexADevice3, {1 / 255.0f, 2 / 255.0f, 3 / 255.0f, 4 / 255.0f});

    // Copy A->B
    SimpleCopyTextureToTexture(thirdDevice, thirdDeviceQueue, wrappedTexADevice3,
                               wrappedTexBDevice3);

    ExternalImageExportInfoVkForTesting exportInfoTexBDevice3;
    ASSERT_TRUE(mBackend->ExportImage(wrappedTexBDevice3, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                      &exportInfoTexBDevice3));
    IgnoreSignalSemaphore(wrappedTexADevice3);

    // Import TexB, TexC on device 2
    wgpu::Texture wrappedTexBDevice2 = WrapVulkanImage(
        secondDevice, &defaultDescriptor, textureB.get(),
        std::move(exportInfoTexBDevice3.semaphores), exportInfoTexBDevice3.releasedOldLayout,
        exportInfoTexBDevice3.releasedNewLayout);

    wgpu::Texture wrappedTexCDevice2 =
        WrapVulkanImage(secondDevice, &defaultDescriptor, textureC.get(), {},
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy B->C on device 2
    SimpleCopyTextureToTexture(secondDevice, secondDeviceQueue, wrappedTexBDevice2,
                               wrappedTexCDevice2);

    ExternalImageExportInfoVkForTesting exportInfoTexCDevice2;
    ASSERT_TRUE(mBackend->ExportImage(wrappedTexCDevice2, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                      &exportInfoTexCDevice2));
    IgnoreSignalSemaphore(wrappedTexBDevice2);

    // Import TexC on device 1
    wgpu::Texture wrappedTexCDevice1 = WrapVulkanImage(
        device, &defaultDescriptor, textureC.get(), std::move(exportInfoTexCDevice2.semaphores),
        exportInfoTexCDevice2.releasedOldLayout, exportInfoTexCDevice2.releasedNewLayout);

    // Create TexD on device 1
    wgpu::Texture texD = device.CreateTexture(&defaultDescriptor);

    // Copy C->D on device 1
    SimpleCopyTextureToTexture(device, queue, wrappedTexCDevice1, texD);

    // Verify D matches clear color
    EXPECT_PIXEL_RGBA8_EQ(RGBA8(1, 2, 3, 4), texD, 0, 0);

    IgnoreSignalSemaphore(wrappedTexCDevice1);
}

// Tests a larger image is preserved when importing
TEST_P(VulkanImageWrappingUsageTests, LargerImage) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 640;
    descriptor.size.height = 480;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;

    // Fill memory with textures
    std::vector<wgpu::Texture> textures;
    for (int i = 0; i < 20; i++) {
        textures.push_back(device.CreateTexture(&descriptor));
    }

    wgpu::Queue secondDeviceQueue = secondDevice.GetQueue();

    // Make an image on |secondDevice|
    std::unique_ptr<ExternalTexture> texture = mBackend->CreateTexture(
        descriptor.size.width, descriptor.size.height, descriptor.format, descriptor.usage);

    // Import the image on |secondDevice|
    wgpu::Texture wrappedTexture =
        WrapVulkanImage(secondDevice, &descriptor, texture.get(), {}, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Draw a non-trivial picture
    uint32_t width = 640, height = 480, pixelSize = 4;
    uint32_t bytesPerRow = Align(width * pixelSize, kTextureBytesPerRowAlignment);
    std::vector<unsigned char> data(bytesPerRow * (height - 1) + width * pixelSize);

    for (uint32_t row = 0; row < height; row++) {
        for (uint32_t col = 0; col < width; col++) {
            float normRow = static_cast<float>(row) / height;
            float normCol = static_cast<float>(col) / width;
            float dist = sqrt(normRow * normRow + normCol * normCol) * 3;
            dist = dist - static_cast<int>(dist);
            data[4 * (row * width + col)] = static_cast<unsigned char>(dist * 255);
            data[4 * (row * width + col) + 1] = static_cast<unsigned char>(dist * 255);
            data[4 * (row * width + col) + 2] = static_cast<unsigned char>(dist * 255);
            data[4 * (row * width + col) + 3] = 255;
        }
    }

    // Write the picture
    {
        wgpu::Buffer copySrcBuffer = utils::CreateBufferFromData(
            secondDevice, data.data(), data.size(), wgpu::BufferUsage::CopySrc);
        wgpu::ImageCopyBuffer copySrc = utils::CreateImageCopyBuffer(copySrcBuffer, 0, bytesPerRow);
        wgpu::ImageCopyTexture copyDst =
            utils::CreateImageCopyTexture(wrappedTexture, 0, {0, 0, 0});
        wgpu::Extent3D copySize = {width, height, 1};

        wgpu::CommandEncoder encoder = secondDevice.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
        wgpu::CommandBuffer commands = encoder.Finish();
        secondDeviceQueue.Submit(1, &commands);
    }
    ExternalImageExportInfoVkForTesting exportInfo;
    ASSERT_TRUE(
        mBackend->ExportImage(wrappedTexture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, &exportInfo));

    // Import the image on |device|
    wgpu::Texture nextWrappedTexture =
        WrapVulkanImage(device, &descriptor, texture.get(), std::move(exportInfo.semaphores),
                        exportInfo.releasedOldLayout, exportInfo.releasedNewLayout);

    // Copy the image into a buffer for comparison
    wgpu::BufferDescriptor copyDesc;
    copyDesc.size = data.size();
    copyDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer copyDstBuffer = device.CreateBuffer(&copyDesc);
    {
        wgpu::ImageCopyTexture copySrc =
            utils::CreateImageCopyTexture(nextWrappedTexture, 0, {0, 0, 0});
        wgpu::ImageCopyBuffer copyDst = utils::CreateImageCopyBuffer(copyDstBuffer, 0, bytesPerRow);

        wgpu::Extent3D copySize = {width, height, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    // Check the image is not corrupted on |device|
    EXPECT_BUFFER_U32_RANGE_EQ(reinterpret_cast<uint32_t*>(data.data()), copyDstBuffer, 0,
                               data.size() / 4);

    IgnoreSignalSemaphore(nextWrappedTexture);
}

// Test that texture descriptor view formats are passed to the backend for wrapped external
// textures, and that contents may be reinterpreted as sRGB.
TEST_P(VulkanImageWrappingUsageTests, SRGBReinterpretation) {
    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::RGBA8UnormSrgb;

    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size = {2, 2, 1};
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    textureDesc.viewFormatCount = 1;
    textureDesc.viewFormats = &viewDesc.format;

    std::unique_ptr<ExternalTexture> backendTexture = mBackend->CreateTexture(
        textureDesc.size.width, textureDesc.size.height, textureDesc.format, textureDesc.usage);

    // Import the image on |device|
    wgpu::Texture texture =
        WrapVulkanImage(device, &textureDesc, backendTexture.get(), {}, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    ASSERT_NE(texture.Get(), nullptr);

    wgpu::ImageCopyTexture dst = {};
    dst.texture = texture;
    std::array<RGBA8, 4> rgbaTextureData = {
        RGBA8(180, 0, 0, 255),
        RGBA8(0, 84, 0, 127),
        RGBA8(0, 0, 62, 100),
        RGBA8(62, 180, 84, 90),
    };

    wgpu::TextureDataLayout dataLayout = {};
    dataLayout.bytesPerRow = textureDesc.size.width * sizeof(RGBA8);

    queue.WriteTexture(&dst, rgbaTextureData.data(), rgbaTextureData.size() * sizeof(RGBA8),
                       &dataLayout, &textureDesc.size);

    wgpu::TextureView textureView = texture.CreateView(&viewDesc);

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
            @stage(vertex)
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
                var pos = array<vec2<f32>, 6>(
                                            vec2<f32>(-1.0, -1.0),
                                            vec2<f32>(-1.0,  1.0),
                                            vec2<f32>( 1.0, -1.0),
                                            vec2<f32>(-1.0,  1.0),
                                            vec2<f32>( 1.0, -1.0),
                                            vec2<f32>( 1.0,  1.0));
                return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
            }
        )");
    pipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var texture : texture_2d<f32>;

            @stage(fragment)
            fn main(@builtin(position) coord: vec4<f32>) -> @location(0) vec4<f32> {
                return textureLoad(texture, vec2<i32>(coord.xy), 0);
            }
        )");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, textureDesc.size.width, textureDesc.size.height, wgpu::TextureFormat::RGBA8Unorm);
    pipelineDesc.cTargets[0].format = renderPass.colorFormat;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, textureView}});

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_BETWEEN(  //
        RGBA8(116, 0, 0, 255),   //
        RGBA8(117, 0, 0, 255), renderPass.color, 0, 0);
    EXPECT_PIXEL_RGBA8_BETWEEN(  //
        RGBA8(0, 23, 0, 127),    //
        RGBA8(0, 24, 0, 127), renderPass.color, 1, 0);
    EXPECT_PIXEL_RGBA8_BETWEEN(  //
        RGBA8(0, 0, 12, 100),    //
        RGBA8(0, 0, 13, 100), renderPass.color, 0, 1);
    EXPECT_PIXEL_RGBA8_BETWEEN(  //
        RGBA8(12, 116, 23, 90),  //
        RGBA8(13, 117, 24, 90), renderPass.color, 1, 1);

    IgnoreSignalSemaphore(texture);
}

DAWN_INSTANTIATE_TEST(VulkanImageWrappingValidationTests, VulkanBackend());
DAWN_INSTANTIATE_TEST(VulkanImageWrappingUsageTests, VulkanBackend());

}  // namespace dawn::native::vulkan
