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

#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {

class VideoViewsValidation : public ValidationTest {
  protected:
    WGPUDevice CreateTestDevice() override {
        wgpu::DeviceDescriptor descriptor;
        wgpu::FeatureName requiredFeatures[1] = {wgpu::FeatureName::DawnMultiPlanarFormats};
        descriptor.requiredFeatures = requiredFeatures;
        descriptor.requiredFeaturesCount = 1;
        return adapter.CreateDevice(&descriptor);
    }

    wgpu::Texture CreateVideoTextureForTest(wgpu::TextureFormat format, wgpu::TextureUsage usage) {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = 1;
        descriptor.size.height = 1;
        descriptor.format = format;
        descriptor.usage = usage;
        return device.CreateTexture(&descriptor);
    }
};

// Test texture views compatibility rules.
TEST_F(VideoViewsValidation, CreateViewFails) {
    wgpu::Texture videoTexture = CreateVideoTextureForTest(
        wgpu::TextureFormat::R8BG8Biplanar420Unorm, wgpu::TextureUsage::TextureBinding);

    wgpu::TextureViewDescriptor viewDesc = {};

    // Correct plane index but incompatible view format.
    viewDesc.format = wgpu::TextureFormat::R8Uint;
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    ASSERT_DEVICE_ERROR(videoTexture.CreateView(&viewDesc));

    // Compatible view format but wrong plane index.
    viewDesc.format = wgpu::TextureFormat::R8Unorm;
    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    ASSERT_DEVICE_ERROR(videoTexture.CreateView(&viewDesc));

    // Compatible view format but wrong aspect.
    viewDesc.format = wgpu::TextureFormat::R8Unorm;
    viewDesc.aspect = wgpu::TextureAspect::All;
    ASSERT_DEVICE_ERROR(videoTexture.CreateView(&viewDesc));

    // Create a single plane texture.
    wgpu::TextureDescriptor desc;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.dimension = wgpu::TextureDimension::e2D;
    desc.usage = wgpu::TextureUsage::TextureBinding;
    desc.size = {1, 1, 1};

    wgpu::Texture texture = device.CreateTexture(&desc);

    // Plane aspect specified with non-planar texture.
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    // Planar views with non-planar texture.
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    viewDesc.format = wgpu::TextureFormat::R8Unorm;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    viewDesc.format = wgpu::TextureFormat::RG8Unorm;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));
}

// Test texture views compatibility rules.
TEST_F(VideoViewsValidation, CreateViewSucceeds) {
    wgpu::Texture yuvTexture = CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                                         wgpu::TextureUsage::TextureBinding);

    // Per plane view formats unspecified.
    wgpu::TextureViewDescriptor planeViewDesc = {};
    planeViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView plane0View = yuvTexture.CreateView(&planeViewDesc);

    planeViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView plane1View = yuvTexture.CreateView(&planeViewDesc);

    ASSERT_NE(plane0View.Get(), nullptr);
    ASSERT_NE(plane1View.Get(), nullptr);

    // Per plane view formats specified.
    planeViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    planeViewDesc.format = wgpu::TextureFormat::R8Unorm;
    plane0View = yuvTexture.CreateView(&planeViewDesc);

    planeViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    planeViewDesc.format = wgpu::TextureFormat::RG8Unorm;
    plane1View = yuvTexture.CreateView(&planeViewDesc);

    ASSERT_NE(plane0View.Get(), nullptr);
    ASSERT_NE(plane1View.Get(), nullptr);
}

// Test copying from one multi-planar format into another fails.
TEST_F(VideoViewsValidation, T2TCopyAllAspectsFails) {
    wgpu::Texture srcTexture = CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                                         wgpu::TextureUsage::TextureBinding);

    wgpu::Texture dstTexture = CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                                         wgpu::TextureUsage::TextureBinding);

    wgpu::ImageCopyTexture copySrc = utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0});

    wgpu::ImageCopyTexture copyDst = utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0});

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test copying from one multi-planar format into another per plane fails.
TEST_F(VideoViewsValidation, T2TCopyPlaneAspectFails) {
    wgpu::Texture srcTexture = CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                                         wgpu::TextureUsage::TextureBinding);

    wgpu::Texture dstTexture = CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                                         wgpu::TextureUsage::TextureBinding);

    wgpu::ImageCopyTexture copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::ImageCopyTexture copyDst =
        utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);

    wgpu::Extent3D copySize = {1, 1, 1};

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test copying from a multi-planar format to a buffer fails.
TEST_F(VideoViewsValidation, T2BCopyAllAspectsFails) {
    wgpu::Texture srcTexture = CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                                         wgpu::TextureUsage::TextureBinding);

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 1;
    bufferDescriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::ImageCopyTexture copySrc = utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0});

    wgpu::ImageCopyBuffer copyDst = utils::CreateImageCopyBuffer(dstBuffer, 0, 4);

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test copying from multi-planar format per plane to a buffer fails.
TEST_F(VideoViewsValidation, T2BCopyPlaneAspectsFails) {
    wgpu::Texture srcTexture = CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                                         wgpu::TextureUsage::TextureBinding);

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 1;
    bufferDescriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::ImageCopyTexture copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::ImageCopyBuffer copyDst = utils::CreateImageCopyBuffer(dstBuffer, 0, 4);

    wgpu::Extent3D copySize = {1, 1, 1};

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test copying from a buffer to a multi-planar format fails.
TEST_F(VideoViewsValidation, B2TCopyAllAspectsFails) {
    std::vector<uint8_t> placeholderData(4, 0);

    wgpu::Buffer srcBuffer = utils::CreateBufferFromData(
        device, placeholderData.data(), placeholderData.size(), wgpu::BufferUsage::CopySrc);

    wgpu::Texture dstTexture = CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                                         wgpu::TextureUsage::TextureBinding);

    wgpu::ImageCopyBuffer copySrc = utils::CreateImageCopyBuffer(srcBuffer, 0, 12, 4);

    wgpu::ImageCopyTexture copyDst = utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0});

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test copying from a buffer to a multi-planar format per plane fails.
TEST_F(VideoViewsValidation, B2TCopyPlaneAspectsFails) {
    std::vector<uint8_t> placeholderData(4, 0);

    wgpu::Buffer srcBuffer = utils::CreateBufferFromData(
        device, placeholderData.data(), placeholderData.size(), wgpu::BufferUsage::CopySrc);

    wgpu::Texture dstTexture = CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                                         wgpu::TextureUsage::TextureBinding);

    wgpu::ImageCopyBuffer copySrc = utils::CreateImageCopyBuffer(srcBuffer, 0, 12, 4);

    wgpu::ImageCopyTexture copyDst =
        utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::Extent3D copySize = {1, 1, 1};

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    copyDst =
        utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Tests which multi-planar formats are allowed to be sampled.
TEST_F(VideoViewsValidation, SamplingMultiPlanarTexture) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});

    // R8BG8Biplanar420Unorm is allowed to be sampled, if plane 0 or plane 1 is selected.
    wgpu::Texture texture = CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                                      wgpu::TextureUsage::TextureBinding);

    wgpu::TextureViewDescriptor desc = {};

    desc.aspect = wgpu::TextureAspect::Plane0Only;
    utils::MakeBindGroup(device, layout, {{0, texture.CreateView(&desc)}});

    desc.aspect = wgpu::TextureAspect::Plane1Only;
    utils::MakeBindGroup(device, layout, {{0, texture.CreateView(&desc)}});
}

// Tests creating a texture with a multi-plane format.
TEST_F(VideoViewsValidation, CreateTextureFails) {
    // multi-planar formats are NOT allowed to be renderable.
    ASSERT_DEVICE_ERROR(CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                                  wgpu::TextureUsage::RenderAttachment));
}

// Tests writing into a multi-planar format fails.
TEST_F(VideoViewsValidation, WriteTextureAllAspectsFails) {
    wgpu::Texture texture = CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                                      wgpu::TextureUsage::TextureBinding);

    wgpu::TextureDataLayout textureDataLayout = utils::CreateTextureDataLayout(0, 4, 4);

    wgpu::ImageCopyTexture imageCopyTexture = utils::CreateImageCopyTexture(texture, 0, {0, 0, 0});

    std::vector<uint8_t> placeholderData(4, 0);
    wgpu::Extent3D writeSize = {1, 1, 1};

    wgpu::Queue queue = device.GetQueue();

    ASSERT_DEVICE_ERROR(queue.WriteTexture(&imageCopyTexture, placeholderData.data(),
                                           placeholderData.size(), &textureDataLayout, &writeSize));
}

// Tests writing into a multi-planar format per plane fails.
TEST_F(VideoViewsValidation, WriteTexturePlaneAspectsFails) {
    wgpu::Texture texture = CreateVideoTextureForTest(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                                      wgpu::TextureUsage::TextureBinding);

    wgpu::TextureDataLayout textureDataLayout = utils::CreateTextureDataLayout(0, 12, 4);
    wgpu::ImageCopyTexture imageCopyTexture =
        utils::CreateImageCopyTexture(texture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    std::vector<uint8_t> placeholderData(4, 0);
    wgpu::Extent3D writeSize = {1, 1, 1};

    wgpu::Queue queue = device.GetQueue();

    ASSERT_DEVICE_ERROR(queue.WriteTexture(&imageCopyTexture, placeholderData.data(),
                                           placeholderData.size(), &textureDataLayout, &writeSize));
}

}  // anonymous namespace
