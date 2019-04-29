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
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/DawnHelpers.h"

namespace {

class TextureValidationTest : public ValidationTest {
  protected:
    dawn::TextureDescriptor CreateDefaultTextureDescriptor() {
        dawn::TextureDescriptor descriptor;
        descriptor.size.width = kWidth;
        descriptor.size.height = kHeight;
        descriptor.size.depth = 1;
        descriptor.arrayLayerCount = kDefaultArraySize;
        descriptor.mipLevelCount = kDefaultMipLevels;
        descriptor.sampleCount = kDefaultSampleCount;
        descriptor.dimension = dawn::TextureDimension::e2D;
        descriptor.format = kDefaultTextureFormat;
        descriptor.usage = dawn::TextureUsageBit::OutputAttachment | dawn::TextureUsageBit::Sampled;
        return descriptor;
    }

    dawn::Queue queue = device.CreateQueue();

  private:
    static constexpr uint32_t kWidth = 32;
    static constexpr uint32_t kHeight = 32;
    static constexpr uint32_t kDefaultArraySize = 1;
    static constexpr uint32_t kDefaultMipLevels = 1;
    static constexpr uint32_t kDefaultSampleCount = 1;

    static constexpr dawn::TextureFormat kDefaultTextureFormat = dawn::TextureFormat::R8G8B8A8Unorm;
};

// Test the validation of sample count
TEST_F(TextureValidationTest, SampleCount) {
    dawn::TextureDescriptor defaultDescriptor = CreateDefaultTextureDescriptor();

    // sampleCount == 1 is allowed.
    {
        dawn::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 1;

        device.CreateTexture(&descriptor);
    }

    // sampleCount == 4 is allowed.
    {
        dawn::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 4;

        device.CreateTexture(&descriptor);
    }

    // It is an error to create a texture with an invalid sampleCount.
    {
        dawn::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 3;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // It is an error to create a multisampled texture with mipLevelCount > 1.
    {
        dawn::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 4;
        descriptor.mipLevelCount = 2;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // Currently we do not support multisampled 2D array textures.
    {
        dawn::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 4;
        descriptor.arrayLayerCount = 2;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}

// Test the validation of the mip level count
TEST_F(TextureValidationTest, MipLevelCount) {
    dawn::TextureDescriptor defaultDescriptor = CreateDefaultTextureDescriptor();

    // mipLevelCount == 1 is allowed
    {
        dawn::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = 32;
        descriptor.size.height = 32;
        descriptor.mipLevelCount = 1;

        device.CreateTexture(&descriptor);
    }

    // mipLevelCount == 0 is an error
    {
        dawn::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = 32;
        descriptor.size.height = 32;
        descriptor.mipLevelCount = 0;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // Full mip chains are allowed
    {
        dawn::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = 32;
        descriptor.size.height = 32;
        // Mip level sizes: 32, 16, 8, 4, 2, 1
        descriptor.mipLevelCount = 6;

        device.CreateTexture(&descriptor);
    }

    // Too big mip chains on width are disallowed
    {
        dawn::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = 31;
        descriptor.size.height = 32;
        // Mip level width: 31, 15, 7, 3, 1
        descriptor.mipLevelCount = 6;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // Too big mip chains on height are disallowed
    {
        dawn::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = 32;
        descriptor.size.height = 31;
        // Mip level height: 31, 15, 7, 3, 1
        descriptor.mipLevelCount = 6;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }

    // Undefined shift check if miplevel is bigger than the integer bit width.
    {
        dawn::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.size.width = 32;
        descriptor.size.height = 32;
        descriptor.mipLevelCount = 100;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}

// Test that it is valid to destroy a texture
TEST_F(TextureValidationTest, DestroyTexture) {
    dawn::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
    dawn::Texture texture = device.CreateTexture(&descriptor);
    texture.Destroy();
}

// Test that it's valid to destroy a destroyed texture
TEST_F(TextureValidationTest, DestroyDestroyedTexture) {
    dawn::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
    dawn::Texture texture = device.CreateTexture(&descriptor);
    texture.Destroy();
    texture.Destroy();
}

// Test that it's invalid to submit a destroyed texture in a queue
// in the case of destroy, encode, submit
TEST_F(TextureValidationTest, DestroyEncodeSubmit) {
    dawn::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
    dawn::Texture texture = device.CreateTexture(&descriptor);
    dawn::TextureView textureView = texture.CreateDefaultView();

    utils::ComboRenderPassDescriptor renderPass({textureView});

    // Destroy the texture
    texture.Destroy();

    dawn::CommandEncoder encoder_post_destroy = device.CreateCommandEncoder();
    {
        dawn::RenderPassEncoder pass = encoder_post_destroy.BeginRenderPass(&renderPass);
        pass.EndPass();
    }
    dawn::CommandBuffer commands = encoder_post_destroy.Finish();

    // Submit should fail due to destroyed texture
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
}

// Test that it's invalid to submit a destroyed texture in a queue
// in the case of encode, destroy, submit
TEST_F(TextureValidationTest, EncodeDestroySubmit) {
    dawn::TextureDescriptor descriptor = CreateDefaultTextureDescriptor();
    dawn::Texture texture = device.CreateTexture(&descriptor);
    dawn::TextureView textureView = texture.CreateDefaultView();

    utils::ComboRenderPassDescriptor renderPass({textureView});

    dawn::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        dawn::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.EndPass();
    }
    dawn::CommandBuffer commands = encoder.Finish();

    // Destroy the texture
    texture.Destroy();

    // Submit should fail due to destroyed texture
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
}
}  // namespace
