// Copyright 2018 The Dawn Authors
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

namespace {

class RenderPassDescriptorValidationTest : public ValidationTest {
};

dawn::Texture CreateTexture(dawn::Device& device,
                            dawn::TextureDimension dimension,
                            dawn::TextureFormat format,
                            uint32_t width,
                            uint32_t height,
                            uint32_t arraySize,
                            uint32_t levelCount) {
    dawn::TextureDescriptor descriptor;
    descriptor.dimension = dimension;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depth = 1;
    descriptor.arraySize = arraySize;
    descriptor.sampleCount = 1;
    descriptor.format = format;
    descriptor.levelCount = levelCount;
    descriptor.usage = dawn::TextureUsageBit::OutputAttachment;

    return device.CreateTexture(&descriptor);
}

dawn::TextureView Create2DAttachment(dawn::Device& device,
                                     uint32_t width,
                                     uint32_t height,
                                     dawn::TextureFormat format) {
    dawn::Texture texture = CreateTexture(
        device, dawn::TextureDimension::e2D, format, width, height, 1, 1);
    return texture.CreateDefaultTextureView();
}

// A render pass with no attachments isn't valid
TEST_F(RenderPassDescriptorValidationTest, Empty) {
    AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
        .GetResult();
}

// A render pass with only one color or one depth attachment is ok
TEST_F(RenderPassDescriptorValidationTest, OneAttachment) {
    // One color attachment
    {
        dawn::TextureView color = Create2DAttachment(device, 1, 1, dawn::TextureFormat::R8G8B8A8Unorm);
        dawn::RenderPassColorAttachmentDescriptor colorAttachment;
        colorAttachment.attachment = color;
        colorAttachment.resolveTarget = nullptr;
        colorAttachment.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        colorAttachment.loadOp = dawn::LoadOp::Clear;
        colorAttachment.storeOp = dawn::StoreOp::Store;
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachments(1, &colorAttachment)
            .GetResult();
    }
    // One depth-stencil attachment
    {
        dawn::TextureView depthStencil = Create2DAttachment(device, 1, 1, dawn::TextureFormat::D32FloatS8Uint);
        dawn::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment;
        depthStencilAttachment.attachment = depthStencil;
        depthStencilAttachment.depthLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.stencilLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.clearDepth = 1.0f;
        depthStencilAttachment.clearStencil = 0;
        depthStencilAttachment.depthStoreOp = dawn::StoreOp::Store;
        depthStencilAttachment.stencilStoreOp = dawn::StoreOp::Store;
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetDepthStencilAttachment(&depthStencilAttachment)
            .GetResult();
    }
}

// Test OOB color attachment indices are handled
TEST_F(RenderPassDescriptorValidationTest, ColorAttachmentOutOfBounds) {
    // For setting the color attachment, control case
    {
        dawn::TextureView color = Create2DAttachment(device, 1, 1, dawn::TextureFormat::R8G8B8A8Unorm);
        dawn::RenderPassColorAttachmentDescriptor colorAttachments[kMaxColorAttachments];
        colorAttachments[kMaxColorAttachments - 1].attachment = color;
        colorAttachments[kMaxColorAttachments - 1].resolveTarget = nullptr;
        colorAttachments[kMaxColorAttachments - 1].clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        colorAttachments[kMaxColorAttachments - 1].loadOp = dawn::LoadOp::Clear;
        colorAttachments[kMaxColorAttachments - 1].storeOp = dawn::StoreOp::Store;
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachments(kMaxColorAttachments, colorAttachments)
            .GetResult();
    }
    // For setting the color attachment, OOB
    {
        dawn::TextureView color = Create2DAttachment(device, 1, 1, dawn::TextureFormat::R8G8B8A8Unorm);
        dawn::RenderPassColorAttachmentDescriptor colorAttachments[kMaxColorAttachments + 1];
        colorAttachments[kMaxColorAttachments].attachment = color;
        colorAttachments[kMaxColorAttachments].resolveTarget = nullptr;
        colorAttachments[kMaxColorAttachments].clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        colorAttachments[kMaxColorAttachments].loadOp = dawn::LoadOp::Clear;
        colorAttachments[kMaxColorAttachments].storeOp = dawn::StoreOp::Store;
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachments(kMaxColorAttachments + 1, colorAttachments)
            .GetResult();
    }
}

// Attachments must have the same size
TEST_F(RenderPassDescriptorValidationTest, SizeMustMatch) {
    dawn::TextureView color1x1A = Create2DAttachment(device, 1, 1, dawn::TextureFormat::R8G8B8A8Unorm);
    dawn::TextureView color1x1B = Create2DAttachment(device, 1, 1, dawn::TextureFormat::R8G8B8A8Unorm);
    dawn::TextureView color2x2 = Create2DAttachment(device, 2, 2, dawn::TextureFormat::R8G8B8A8Unorm);

    dawn::RenderPassColorAttachmentDescriptor colorAttachment1x1A;
    colorAttachment1x1A.attachment = color1x1A;
    colorAttachment1x1A.resolveTarget = nullptr;
    colorAttachment1x1A.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    colorAttachment1x1A.loadOp = dawn::LoadOp::Clear;
    colorAttachment1x1A.storeOp = dawn::StoreOp::Store;

    dawn::RenderPassColorAttachmentDescriptor colorAttachment1x1B;
    colorAttachment1x1B.attachment = color1x1B;
    colorAttachment1x1B.resolveTarget = nullptr;
    colorAttachment1x1B.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    colorAttachment1x1B.loadOp = dawn::LoadOp::Clear;
    colorAttachment1x1B.storeOp = dawn::StoreOp::Store;

    dawn::RenderPassColorAttachmentDescriptor colorAttachment2x2;
    colorAttachment2x2.attachment = color2x2;
    colorAttachment2x2.resolveTarget = nullptr;
    colorAttachment2x2.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    colorAttachment2x2.loadOp = dawn::LoadOp::Clear;
    colorAttachment2x2.storeOp = dawn::StoreOp::Store;

    dawn::TextureView depthStencil1x1 = Create2DAttachment(device, 1, 1, dawn::TextureFormat::D32FloatS8Uint);
    dawn::TextureView depthStencil2x2 = Create2DAttachment(device, 2, 2, dawn::TextureFormat::D32FloatS8Uint);

    dawn::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment1x1;
    depthStencilAttachment1x1.attachment = depthStencil1x1;
    depthStencilAttachment1x1.depthLoadOp = dawn::LoadOp::Clear;
    depthStencilAttachment1x1.stencilLoadOp = dawn::LoadOp::Clear;
    depthStencilAttachment1x1.clearDepth = 1.0f;
    depthStencilAttachment1x1.clearStencil = 0;
    depthStencilAttachment1x1.depthStoreOp = dawn::StoreOp::Store;
    depthStencilAttachment1x1.stencilStoreOp = dawn::StoreOp::Store;

    dawn::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment2x2;
    depthStencilAttachment2x2.attachment = depthStencil2x2;
    depthStencilAttachment2x2.depthLoadOp = dawn::LoadOp::Clear;
    depthStencilAttachment2x2.stencilLoadOp = dawn::LoadOp::Clear;
    depthStencilAttachment2x2.clearDepth = 1.0f;
    depthStencilAttachment2x2.clearStencil = 0;
    depthStencilAttachment2x2.depthStoreOp = dawn::StoreOp::Store;
    depthStencilAttachment2x2.stencilStoreOp = dawn::StoreOp::Store;

    // Control case: all the same size (1x1)
    {
        dawn::RenderPassColorAttachmentDescriptor colorAttachments[2];
        colorAttachments[0] = colorAttachment1x1A;
        colorAttachments[1] = colorAttachment1x1B;

        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachments(2, colorAttachments)
            .SetDepthStencilAttachment(&depthStencilAttachment1x1)
            .GetResult();
    }

    // One of the color attachments has a different size
    {
        dawn::RenderPassColorAttachmentDescriptor colorAttachments[2];
        colorAttachments[0] = colorAttachment1x1A;
        colorAttachments[1] = colorAttachment2x2;

        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachments(2, colorAttachments)
            .SetDepthStencilAttachment(&depthStencilAttachment1x1)
            .GetResult();
    }

    // The depth stencil attachment has a different size
    {
        dawn::RenderPassColorAttachmentDescriptor colorAttachments[2];
        colorAttachments[0] = colorAttachment1x1A;
        colorAttachments[1] = colorAttachment1x1B;

        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachments(2, colorAttachments)
            .SetDepthStencilAttachment(&depthStencilAttachment2x2)
            .GetResult();
    }
}

// Attachments formats must match whether they are used for color or depth-stencil
TEST_F(RenderPassDescriptorValidationTest, FormatMismatch) {
    dawn::TextureView color = Create2DAttachment(device, 1, 1, dawn::TextureFormat::R8G8B8A8Unorm);
    dawn::TextureView depthStencil = Create2DAttachment(device, 1, 1, dawn::TextureFormat::D32FloatS8Uint);

    // Using depth-stencil for color
    {
        dawn::RenderPassColorAttachmentDescriptor colorAttachment;
        colorAttachment.attachment = depthStencil;
        colorAttachment.resolveTarget = nullptr;
        colorAttachment.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        colorAttachment.loadOp = dawn::LoadOp::Clear;
        colorAttachment.storeOp = dawn::StoreOp::Store;

        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachments(1, &colorAttachment)
            .GetResult();
    }

    // Using color for depth-stencil
    {
        dawn::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment;
        depthStencilAttachment.attachment = color;
        depthStencilAttachment.depthLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.stencilLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.clearDepth = 1.0f;
        depthStencilAttachment.clearStencil = 0;
        depthStencilAttachment.depthStoreOp = dawn::StoreOp::Store;
        depthStencilAttachment.stencilStoreOp = dawn::StoreOp::Store;
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetDepthStencilAttachment(&depthStencilAttachment)
            .GetResult();
    }
}

// Currently only texture views with layerCount == 1 are allowed to be color and depth stencil
// attachments
TEST_F(RenderPassDescriptorValidationTest, TextureViewLayerCountForColorAndDepthStencil) {
    constexpr uint32_t kLevelCount = 1;
    constexpr uint32_t kSize = 32;
    constexpr dawn::TextureFormat kColorFormat = dawn::TextureFormat::R8G8B8A8Unorm;
    constexpr dawn::TextureFormat kDepthStencilFormat = dawn::TextureFormat::D32FloatS8Uint;

    constexpr uint32_t kArrayLayers = 10;

    dawn::Texture colorTexture = CreateTexture(
        device, dawn::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers, kLevelCount);
    dawn::Texture depthStencilTexture = CreateTexture(
        device, dawn::TextureDimension::e2D, kDepthStencilFormat, kSize, kSize, kArrayLayers,
        kLevelCount);

    dawn::TextureViewDescriptor baseDescriptor;
    baseDescriptor.dimension = dawn::TextureViewDimension::e2DArray;
    baseDescriptor.baseArrayLayer = 0;
    baseDescriptor.layerCount = kArrayLayers;
    baseDescriptor.baseMipLevel = 0;
    baseDescriptor.levelCount = kLevelCount;

    // Using 2D array texture view with layerCount > 1 is not allowed for color
    {
        dawn::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.layerCount = 5;

        dawn::TextureView colorTextureView = colorTexture.CreateTextureView(&descriptor);
        dawn::RenderPassColorAttachmentDescriptor colorAttachment;
        colorAttachment.attachment = colorTextureView;
        colorAttachment.resolveTarget = nullptr;
        colorAttachment.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        colorAttachment.loadOp = dawn::LoadOp::Clear;
        colorAttachment.storeOp = dawn::StoreOp::Store;

        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachments(1, &colorAttachment)
            .GetResult();
    }

    // Using 2D array texture view with layerCount > 1 is not allowed for depth stencil
    {
        dawn::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.layerCount = 5;

        dawn::TextureView depthStencilView = depthStencilTexture.CreateTextureView(&descriptor);
        dawn::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment;
        depthStencilAttachment.attachment = depthStencilView;
        depthStencilAttachment.depthLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.stencilLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.clearDepth = 1.0f;
        depthStencilAttachment.clearStencil = 0;
        depthStencilAttachment.depthStoreOp = dawn::StoreOp::Store;
        depthStencilAttachment.stencilStoreOp = dawn::StoreOp::Store;
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetDepthStencilAttachment(&depthStencilAttachment)
            .GetResult();
    }

    // Using 2D array texture view that covers the first layer of the texture is OK for color
    {
        dawn::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.baseArrayLayer = 0;
        descriptor.layerCount = 1;

        dawn::TextureView colorTextureView = colorTexture.CreateTextureView(&descriptor);
        dawn::RenderPassColorAttachmentDescriptor colorAttachment;
        colorAttachment.attachment = colorTextureView;
        colorAttachment.resolveTarget = nullptr;
        colorAttachment.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        colorAttachment.loadOp = dawn::LoadOp::Clear;
        colorAttachment.storeOp = dawn::StoreOp::Store;
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachments(1, &colorAttachment)
            .GetResult();
    }

    // Using 2D array texture view that covers the first layer is OK for depth stencil
    {
        dawn::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.baseArrayLayer = 0;
        descriptor.layerCount = 1;

        dawn::TextureView depthStencilTextureView =
            depthStencilTexture.CreateTextureView(&descriptor);
        dawn::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment;
        depthStencilAttachment.attachment = depthStencilTextureView;
        depthStencilAttachment.depthLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.stencilLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.clearDepth = 1.0f;
        depthStencilAttachment.clearStencil = 0;
        depthStencilAttachment.depthStoreOp = dawn::StoreOp::Store;
        depthStencilAttachment.stencilStoreOp = dawn::StoreOp::Store;
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetDepthStencilAttachment(&depthStencilAttachment)
            .GetResult();
    }

    // Using 2D array texture view that covers the last layer is OK for color
    {
        dawn::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.baseArrayLayer = kArrayLayers - 1;
        descriptor.layerCount = 1;

        dawn::TextureView colorTextureView = colorTexture.CreateTextureView(&descriptor);
        dawn::RenderPassColorAttachmentDescriptor colorAttachment;
        colorAttachment.attachment = colorTextureView;
        colorAttachment.resolveTarget = nullptr;
        colorAttachment.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        colorAttachment.loadOp = dawn::LoadOp::Clear;
        colorAttachment.storeOp = dawn::StoreOp::Store;
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachments(1, &colorAttachment)
            .GetResult();
    }

    // Using 2D array texture view that covers the last layer is OK for depth stencil
    {
        dawn::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.baseArrayLayer = kArrayLayers - 1;
        descriptor.layerCount = 1;

        dawn::TextureView depthStencilTextureView =
            depthStencilTexture.CreateTextureView(&descriptor);
        dawn::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment;
        depthStencilAttachment.attachment = depthStencilTextureView;
        depthStencilAttachment.depthLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.stencilLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.clearDepth = 1.0f;
        depthStencilAttachment.clearStencil = 0;
        depthStencilAttachment.depthStoreOp = dawn::StoreOp::Store;
        depthStencilAttachment.stencilStoreOp = dawn::StoreOp::Store;
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetDepthStencilAttachment(&depthStencilAttachment)
            .GetResult();
    }
}

// Only 2D texture views with levelCount == 1 are allowed to be color attachments
TEST_F(RenderPassDescriptorValidationTest, TextureViewLevelCountForColorAndDepthStencil) {
    constexpr uint32_t kArrayLayers = 1;
    constexpr uint32_t kSize = 32;
    constexpr dawn::TextureFormat kColorFormat = dawn::TextureFormat::R8G8B8A8Unorm;
    constexpr dawn::TextureFormat kDepthStencilFormat = dawn::TextureFormat::D32FloatS8Uint;

    constexpr uint32_t kLevelCount = 4;

    dawn::Texture colorTexture = CreateTexture(
        device, dawn::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers, kLevelCount);
    dawn::Texture depthStencilTexture = CreateTexture(
        device, dawn::TextureDimension::e2D, kDepthStencilFormat, kSize, kSize, kArrayLayers,
        kLevelCount);

    dawn::TextureViewDescriptor baseDescriptor;
    baseDescriptor.dimension = dawn::TextureViewDimension::e2D;
    baseDescriptor.baseArrayLayer = 0;
    baseDescriptor.layerCount = kArrayLayers;
    baseDescriptor.baseMipLevel = 0;
    baseDescriptor.levelCount = kLevelCount;

    // Using 2D texture view with levelCount > 1 is not allowed for color
    {
        dawn::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.levelCount = 2;

        dawn::TextureView colorTextureView = colorTexture.CreateTextureView(&descriptor);
        dawn::RenderPassColorAttachmentDescriptor colorAttachment;
        colorAttachment.attachment = colorTextureView;
        colorAttachment.resolveTarget = nullptr;
        colorAttachment.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        colorAttachment.loadOp = dawn::LoadOp::Clear;
        colorAttachment.storeOp = dawn::StoreOp::Store;
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachments(1, &colorAttachment)
            .GetResult();
    }

    // Using 2D texture view with levelCount > 1 is not allowed for depth stencil
    {
        dawn::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.levelCount = 2;

        dawn::TextureView depthStencilView = depthStencilTexture.CreateTextureView(&descriptor);
        dawn::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment;
        depthStencilAttachment.attachment = depthStencilView;
        depthStencilAttachment.depthLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.stencilLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.clearDepth = 1.0f;
        depthStencilAttachment.clearStencil = 0;
        depthStencilAttachment.depthStoreOp = dawn::StoreOp::Store;
        depthStencilAttachment.stencilStoreOp = dawn::StoreOp::Store;
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetDepthStencilAttachment(&depthStencilAttachment)
            .GetResult();
    }

    // Using 2D texture view that covers the first level of the texture is OK for color
    {
        dawn::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.baseMipLevel = 0;
        descriptor.levelCount = 1;

        dawn::TextureView colorTextureView = colorTexture.CreateTextureView(&descriptor);
        dawn::RenderPassColorAttachmentDescriptor colorAttachment;
        colorAttachment.attachment = colorTextureView;
        colorAttachment.resolveTarget = nullptr;
        colorAttachment.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        colorAttachment.loadOp = dawn::LoadOp::Clear;
        colorAttachment.storeOp = dawn::StoreOp::Store;
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachments(1, &colorAttachment)
            .GetResult();
    }

    // Using 2D texture view that covers the first level is OK for depth stencil
    {
        dawn::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.baseMipLevel = 0;
        descriptor.levelCount = 1;

        dawn::TextureView depthStencilTextureView =
            depthStencilTexture.CreateTextureView(&descriptor);
        dawn::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment;
        depthStencilAttachment.attachment = depthStencilTextureView;
        depthStencilAttachment.depthLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.stencilLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.clearDepth = 1.0f;
        depthStencilAttachment.clearStencil = 0;
        depthStencilAttachment.depthStoreOp = dawn::StoreOp::Store;
        depthStencilAttachment.stencilStoreOp = dawn::StoreOp::Store;
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetDepthStencilAttachment(&depthStencilAttachment)
            .GetResult();
    }

    // Using 2D texture view that covers the last level is OK for color
    {
        dawn::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kColorFormat;
        descriptor.baseMipLevel = kLevelCount - 1;
        descriptor.levelCount = 1;

        dawn::TextureView colorTextureView = colorTexture.CreateTextureView(&descriptor);
        dawn::RenderPassColorAttachmentDescriptor colorAttachment;
        colorAttachment.attachment = colorTextureView;
        colorAttachment.resolveTarget = nullptr;
        colorAttachment.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        colorAttachment.loadOp = dawn::LoadOp::Clear;
        colorAttachment.storeOp = dawn::StoreOp::Store;
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachments(1, &colorAttachment)
            .GetResult();
    }

    // Using 2D texture view that covers the last level is OK for depth stencil
    {
        dawn::TextureViewDescriptor descriptor = baseDescriptor;
        descriptor.format = kDepthStencilFormat;
        descriptor.baseMipLevel = kLevelCount - 1;
        descriptor.levelCount = 1;

        dawn::TextureView depthStencilTextureView =
            depthStencilTexture.CreateTextureView(&descriptor);
        dawn::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment;
        depthStencilAttachment.attachment = depthStencilTextureView;
        depthStencilAttachment.depthLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.stencilLoadOp = dawn::LoadOp::Clear;
        depthStencilAttachment.clearDepth = 1.0f;
        depthStencilAttachment.clearStencil = 0;
        depthStencilAttachment.depthStoreOp = dawn::StoreOp::Store;
        depthStencilAttachment.stencilStoreOp = dawn::StoreOp::Store;
        AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
            .SetDepthStencilAttachment(&depthStencilAttachment)
            .GetResult();
    }
}

// Tests on the resolve target of RenderPassColorAttachmentDescriptor.
// TODO(jiawei.shao@intel.com): add more tests when we support multisample color attachments.
TEST_F(RenderPassDescriptorValidationTest, ResolveTarget) {
    constexpr uint32_t kArrayLayers = 1;
    constexpr uint32_t kSize = 32;
    constexpr dawn::TextureFormat kColorFormat = dawn::TextureFormat::R8G8B8A8Unorm;

    constexpr uint32_t kLevelCount = 1;

    dawn::Texture colorTexture = CreateTexture(
        device, dawn::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers, kLevelCount);

    dawn::Texture resolveTexture = CreateTexture(
        device, dawn::TextureDimension::e2D, kColorFormat, kSize, kSize, kArrayLayers, kLevelCount);

    // It is not allowed to set resolve target when the sample count of the color attachment is 1.
    {
        dawn::TextureView colorTextureView = colorTexture.CreateDefaultTextureView();
        dawn::TextureView resolveTargetTextureView = resolveTexture.CreateDefaultTextureView();

        dawn::RenderPassColorAttachmentDescriptor colorAttachment;
        colorAttachment.attachment = colorTextureView;
        colorAttachment.resolveTarget = resolveTargetTextureView;
        colorAttachment.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        colorAttachment.loadOp = dawn::LoadOp::Clear;
        colorAttachment.storeOp = dawn::StoreOp::Store;
        AssertWillBeError(device.CreateRenderPassDescriptorBuilder())
            .SetColorAttachments(1, &colorAttachment)
            .GetResult();
    }
}

// TODO(cwallez@chromium.org): Constraints on attachment aliasing?

} // anonymous namespace
