// Copyright 2026 The Dawn & Tint Authors
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

#include <string>

#include "src/dawn/tests/DawnTest.h"

namespace dawn {
namespace {

class TextureAllocationTests : public DawnTest {
  protected:
    wgpu::TextureDescriptor MakeDescriptor() {
        wgpu::TextureDescriptor descriptor;
        descriptor.size = {512, 512, 1};
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.usage = wgpu::TextureUsage::TextureBinding;
        return descriptor;
    }

    void CheckTextureCreation(
        const wgpu::TextureDescriptor& descriptor,
        wgpu::ErrorType expectedError = wgpu::ErrorType::NoError,
        const char* expectedMessage = "Texture allocation exceeds the 1 MiB limit for testing.") {
        device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        bool done = false;
        device.PopErrorScope(
            wgpu::CallbackMode::AllowProcessEvents,
            [&](wgpu::PopErrorScopeStatus status, wgpu::ErrorType error, wgpu::StringView message) {
                EXPECT_EQ(status, wgpu::PopErrorScopeStatus::Success);
                EXPECT_EQ(error, expectedError);
                if (expectedError == wgpu::ErrorType::OutOfMemory) {
                    EXPECT_THAT(std::string(message), testing::HasSubstr(expectedMessage));
                }
                done = true;
            });
        while (!done) {
            WaitABit();
        }
    }
};

// Only the base mip counts: a full mip chain with a 1 MiB base level succeeds.
TEST_P(TextureAllocationTests, BaseLevelSizeLimit) {
    auto descriptor = MakeDescriptor();
    descriptor.mipLevelCount = 10;
    CheckTextureCreation(descriptor);
    descriptor.size.width++;
    CheckTextureCreation(descriptor, wgpu::ErrorType::OutOfMemory);
}

// Use the maximum texel block memory cost, including depth/stencil padding.
TEST_P(TextureAllocationTests, DepthStencilMemoryCost) {
    auto descriptor = MakeDescriptor();
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;

    // Interleaved depth/stencil can cost 8 bytes per texel, not 4 + 1.
    descriptor.format = wgpu::TextureFormat::Depth24PlusStencil8;
    descriptor.size = {512, 256, 1};
    CheckTextureCreation(descriptor);
    descriptor.size.width++;
    CheckTextureCreation(descriptor, wgpu::ErrorType::OutOfMemory);

    // Stencil-only storage can cost 4 bytes per texel, not 1.
    descriptor.format = wgpu::TextureFormat::Stencil8;
    descriptor.size = {512, 512, 1};
    CheckTextureCreation(descriptor);
    descriptor.size.width++;
    CheckTextureCreation(descriptor, wgpu::ErrorType::OutOfMemory);
}

DAWN_INSTANTIATE_TEST(
    TextureAllocationTests,
    D3D11Backend({"limit_max_texture_bytes_to_1mb"}),
    D3D12Backend({"limit_max_texture_bytes_to_1mb"}),
    MetalBackend({"limit_max_texture_bytes_to_1mb"}),
    OpenGLBackend({"limit_max_texture_bytes_to_1mb"}, {"limit_3d_texture_pot_footprint_to_4gb"}),
    OpenGLESBackend({"limit_max_texture_bytes_to_1mb"}, {"limit_3d_texture_pot_footprint_to_4gb"}),
    OpenGLESBackend({"limit_max_texture_bytes_to_1mb", "gl_defer"},
                    {"limit_3d_texture_pot_footprint_to_4gb"}),
    VulkanBackend({"limit_max_texture_bytes_to_1mb"}, {"limit_3d_texture_pot_footprint_to_4gb"}));

class TextureAllocationWithoutLimitTests : public TextureAllocationTests {};

TEST_P(TextureAllocationWithoutLimitTests, LimitIsDisabledByDefault) {
    auto descriptor = MakeDescriptor();
    descriptor.size.width++;
    CheckTextureCreation(descriptor);
}

DAWN_INSTANTIATE_TEST(TextureAllocationWithoutLimitTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class Texture3DPotFootprintTests : public TextureAllocationTests {};

// Regression test for crbug.com/497202432.
TEST_P(Texture3DPotFootprintTests, PowerOfTwoRounding) {
    auto descriptor = MakeDescriptor();
    descriptor.dimension = wgpu::TextureDimension::e3D;
    descriptor.size = {64, 64, 64};
    descriptor.mipLevelCount = 7;
    CheckTextureCreation(descriptor);
    descriptor.mipLevelCount = 1;

    // Below 1 MiB logically, but 2 MiB after PoT rounding: only the workaround should reject it.
    descriptor.size = {33, 65, 33};
    CheckTextureCreation(descriptor,
                         HasToggleEnabled("limit_3d_texture_pot_footprint_to_4gb")
                             ? wgpu::ErrorType::OutOfMemory
                             : wgpu::ErrorType::NoError,
                         "3D texture level power-of-two footprint exceeds limit");

    // A 2D array with the same dimensions is exempt from the PoT workaround.
    descriptor.dimension = wgpu::TextureDimension::e2D;
    CheckTextureCreation(descriptor);
}

DAWN_INSTANTIATE_TEST(
    Texture3DPotFootprintTests,
    OpenGLBackend({"limit_max_texture_bytes_to_1mb", "limit_3d_texture_pot_footprint_to_4gb"}),
    OpenGLESBackend({"limit_max_texture_bytes_to_1mb", "limit_3d_texture_pot_footprint_to_4gb"}),
    OpenGLESBackend({"limit_max_texture_bytes_to_1mb", "limit_3d_texture_pot_footprint_to_4gb",
                     "gl_defer"}),
    VulkanBackend({"limit_max_texture_bytes_to_1mb", "limit_3d_texture_pot_footprint_to_4gb"}),
    OpenGLBackend({"limit_max_texture_bytes_to_1mb"}, {"limit_3d_texture_pot_footprint_to_4gb"}),
    OpenGLESBackend({"limit_max_texture_bytes_to_1mb"}, {"limit_3d_texture_pot_footprint_to_4gb"}),
    VulkanBackend({"limit_max_texture_bytes_to_1mb"}, {"limit_3d_texture_pot_footprint_to_4gb"}));

}  // anonymous namespace
}  // namespace dawn
